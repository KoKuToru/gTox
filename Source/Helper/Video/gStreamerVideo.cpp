#include "../gStreamerHelper.h"
#include <thread>
#include <gstreamermm/bus.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/buffer.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/appsink.h>
#include <gstreamermm/elementfactory.h>
#include <cstring>
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

void gStreamerVideo::init() {
    //setup pipeline
    m_appsink = Gst::AppSink::create();
    m_appsink->property_caps() = Gst::Caps::create_from_string("video/x-raw,format=RGB,pixel-aspect-ratio=1/1");
    m_appsink->property_max_buffers() = 3;
    m_appsink->property_drop() = true;

    //events
    m_appsink->property_emit_signals() = true;
    std::shared_ptr<bool> alive = m_alive;
    m_appsink->signal_new_sample().connect_notify([this, alive](){
        std::lock_guard<std::recursive_mutex> lg(m_mutex_block_destroy);

        if (!*alive) {
            return;
        }

        auto sample = m_appsink->pull_sample();
        if (!sample) {
            return;
        }

        if (m_lastframe.empty()) {
            auto caps = sample->get_caps();

            if (!caps) {
                return;
            }

            if (caps->empty()) {
                return;
            }

            auto struc = caps->get_structure(0);
            if (!(struc.get_field("width", m_width) &&
                  struc.get_field("height", m_height))) {
                return;
            }
        }

        auto buffer = sample->get_buffer();
        if (!buffer) {
            return;
        }

        auto map = Glib::RefPtr<Gst::MapInfo>(new Gst::MapInfo);
        if (!buffer->map(map, Gst::MAP_READ)) {
            return;
        }

        {
            std::lock_guard<std::recursive_mutex> lg(m_mutex);
            m_lastframe.resize(map->get_size());
            std::copy(map->get_data(), map->get_data() + map->get_size(), m_lastframe.begin());
        }
        buffer->unmap(map);

        Glib::signal_idle().connect_once([this, alive](){
            if (!*alive) {
                return;
            }

            m_update.emit(m_width, m_height, m_lastframe);

            if (m_state != PLAY) {
                auto state = Gst::STATE_PLAYING;
                switch (m_state) {
                    case PAUSE:
                        state = Gst::STATE_PAUSED;
                        break;
                    case STOP:
                        state = Gst::STATE_NULL;
                        break;
                    default:
                        break;
                }

                set_state(state);
            }
        });
    });
}

void gStreamerVideo::init_bus(Glib::RefPtr<Gst::Bus> bus) {
    bus->add_watch([this](const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message) {
        Glib::RefPtr<Gst::MessageError> error;

        switch(message->get_message_type()) {
            case Gst::MESSAGE_EOS:
                m_state = STOP;
                set_state(Gst::STATE_NULL);
                break;
            case Gst::MESSAGE_ERROR:
                error = Glib::RefPtr<Gst::MessageError>::cast_static(message);
                if (error) {
                    m_error.emit(error->parse().what());
                } else {
                    m_error.emit(_("GST_VIDEO_ERROR_UNKNOW"));
                }
                m_state = STOP;
                set_state(Gst::STATE_NULL);
                break;
            default:break;
        }
        return true;
    });
}

gStreamerVideo::gStreamerVideo(std::string uri):
    m_alive(std::make_shared<bool>(true)) {

    //init appsink
    init();

    if (uri.find("dev://") == 0) {
        auto dev = gStreamerHelper::probe_device_by_name(uri.substr(std::strlen("dev://")));

        m_pipeline = Gst::Pipeline::create();
        auto device_source = Glib::wrap(gst_device_create_element(dev.device, nullptr));
        auto convert = Gst::ElementFactory::create_element("videoconvert");
        m_pipeline->add(device_source);
        m_pipeline->add(convert);
        m_pipeline->add(m_appsink);

        device_source->link(convert);
        convert->link(m_appsink);

        init_bus(m_pipeline->get_bus());

        m_pipeline->set_state(Gst::STATE_PLAYING);
        return;
    }

    m_playbin = Gst::PlayBin::create("playbin");
    m_playbin->property_video_sink() = m_appsink; //overwrite sink
    m_playbin->property_uri() = uri;

    init_bus(m_playbin->get_bus());

    stop();
    set_state(Gst::STATE_PLAYING);
}


void gStreamerVideo::play() {
    m_state = PLAY;
    set_state(Gst::STATE_PLAYING);
}

void gStreamerVideo::pause() {
    m_state = PAUSE;
}

void gStreamerVideo::stop() {
    m_state = STOP;
    set_state(Gst::STATE_NULL);
}

gStreamerVideo::~gStreamerVideo() {
    std::lock_guard<std::recursive_mutex> lg(m_mutex_block_destroy);
    *m_alive = false;
    stop();
}
void gStreamerVideo::emit_update_signal() {
    std::lock_guard<std::recursive_mutex> lg(m_mutex);
    if (!m_lastframe.empty()) {
        m_update.emit(m_width, m_height, m_lastframe);
    }
}

void gStreamerVideo::set_state(Gst::State state) {
    if (m_pipeline) {
        m_pipeline->set_state(state);
    }
    if (m_playbin) {
        m_playbin->set_state(state);
    }
}
