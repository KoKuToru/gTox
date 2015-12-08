/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
**/
#include "webcam.h"
#include <gstreamermm/bus.h>
#include <gstreamermm/elementfactory.h>
#include <glibmm/i18n.h>

#ifndef SIGC_CPP11_HACK
#define SIGC_CPP11_HACK
namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}
#endif

using namespace utils;

auto webcam::signal_error() -> type_signal_error {
    return m_signal_error;
}

auto webcam::property_device()      -> Glib::PropertyProxy<std::shared_ptr<GstDevice>> {
    return m_property_device.get_proxy();
}
auto webcam::property_state()    -> Glib::PropertyProxy<Gst::State> {
    return m_property_state.get_proxy();
}
auto webcam::property_pixbuf()   -> Glib::PropertyProxy_ReadOnly<Glib::RefPtr<Gdk::Pixbuf>> {
    return Glib::PropertyProxy_ReadOnly<Glib::RefPtr<Gdk::Pixbuf>>(this, "webcam-pixbuf");
}

void webcam::init() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    m_pipeline = Gst::Pipeline::create();
    m_appsink  = Gst::AppSink::create();

    m_appsink->property_caps() = Gst::Caps::create_from_string("video/x-raw,format=RGB,pixel-aspect-ratio=1/1");
    m_appsink->property_max_buffers() = 3;
    m_appsink->property_drop() = true;
    m_appsink->property_emit_signals() = true;

    auto device_source = Glib::wrap(gst_device_create_element(property_device().get_value().get(), nullptr));
    auto convert = Gst::ElementFactory::create_element("videoconvert");
    m_pipeline->add(device_source);
    m_pipeline->add(convert);
    m_pipeline->add(m_appsink);

    device_source->link(convert);
    convert->link(m_appsink);

    utils::dispatcher::ref weak_dispatcher = m_dispatcher;
    auto resolution = std::make_shared<std::pair<int, int>>();

    m_appsink->signal_new_preroll().connect(sigc::track_obj([this, sink = m_appsink, weak_dispatcher, resolution, prev_dispatched = sigc::connection()]() mutable {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
        auto preroll = sink->pull_preroll();
        if (!preroll) {
            return Gst::FLOW_OK;
        }
        //C++ version is buggy, use C version
        auto caps = gst_sample_get_caps(preroll->gobj());
        if (!caps) {
            return Gst::FLOW_OK;
        }
        if (gst_caps_is_empty(caps)) {
            return Gst::FLOW_OK;
        }

        auto struc = gst_caps_get_structure(caps, 0); // < can this fail ?
        int w,h;
        if (!gst_structure_get_int(struc, "width", &w)) {
            return Gst::FLOW_OK;
        }
        if (!gst_structure_get_int(struc, "height", &h)) {
            return Gst::FLOW_OK;
        }
        resolution->first = w;
        resolution->second = h;

        auto frame = extract_frame(preroll, resolution);
        prev_dispatched.disconnect();
        prev_dispatched = weak_dispatcher.emit([this, frame]() {
            utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
            m_property_pixbuf.set_value(frame);
        });
        return Gst::FLOW_OK;

    }, *this));
    m_appsink->signal_new_sample().connect(sigc::track_obj([this, sink = m_appsink, weak_dispatcher, resolution, prev_dispatched = sigc::connection()]() mutable {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
        auto sample = sink->pull_sample();
        auto frame = extract_frame(sample, resolution);
        prev_dispatched.disconnect();
        prev_dispatched = weak_dispatcher.emit([this, frame]() {
            utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
            m_property_pixbuf.set_value(frame);
        });
        return Gst::FLOW_OK;
    }, *this));

    m_pipeline->get_bus()->add_watch(
                sigc::track_obj([this](
                                const Glib::RefPtr<Gst::Bus>&,
                                const Glib::RefPtr<Gst::Message>& message) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        Glib::RefPtr<Gst::MessageError> error;

        switch (message->get_message_type()) {
            case Gst::MESSAGE_EOS:
                property_state() = Gst::STATE_NULL;
                break;
            case Gst::MESSAGE_ERROR:
                error = decltype(error)::cast_static(message);
                if (error) {
                    m_signal_error(error->parse().what());
                } else {
                    m_signal_error(_("unknown gstreamer error"));
                }
                break;
            default:
                break;
        }

        return true;
    }, *this));

    property_state().signal_changed().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_pipeline) {
            m_pipeline->set_state(property_state());
        }
    }, *this));

    m_pipeline->set_state(property_state());
}

void webcam::destroy() {
    if (m_pipeline) {
        m_pipeline->set_state(Gst::STATE_NULL);
    }
}

webcam::webcam():
    Glib::ObjectBase(typeid(webcam)),
    m_property_device(*this, "webcam-device"),
    m_property_state(*this, "webcam-state", Gst::STATE_NULL),
    m_property_pixbuf(*this, "webcam-pixbuf") {

    property_state().signal_changed().connect(sigc::track_obj([this]() {
       if (property_state() == Gst::STATE_PLAYING) {
           if (!m_pipeline && property_device().get_value()) {
               init();
           }
       } else {
           destroy();
       }
    }, *this));

    property_device().signal_changed().connect(sigc::track_obj([this]() {
        // uri changed
        destroy();
    }, *this));
}

webcam::~webcam() {
    destroy();
}

std::vector<std::shared_ptr<GstDevice>> webcam::get_webcam_devices() {
    //INFO: There is no C++ - version of this..
    //TODO: Memory Leak check ?
    GstDeviceMonitor *monitor = gst_device_monitor_new();

    //Filter
    gst_device_monitor_add_filter(monitor, "Video/Source", nullptr);

    //Get all devices
    decltype(get_webcam_devices()) result;
    GList *list = gst_device_monitor_get_devices(monitor);
    GList *it = list;
    while (it != nullptr)
    {
        auto device = (GstDevice*)it->data;

        //maybe need to g_object_ref ?
        result.emplace_back(device, [](GstDevice* ptr) {
            g_object_unref(ptr); // free memory ?
        });

        it = it->next;
    }
    g_list_free(list);       //free memory ?
    g_object_unref(monitor); //free memory ?

    return result;
}

Glib::ustring webcam::get_webcam_device_name(const std::shared_ptr<GstDevice>& device) {
    Glib::ustring result;
    if (device) {
        auto device_name  = gst_device_get_display_name(device.get());
        result = device_name;
        g_free(device_name);
    }
    return result;
}

std::shared_ptr<GstDevice> webcam::get_webcam_device_by_name(const Glib::ustring& name) {
    for (auto device : get_webcam_devices()) {
        if (get_webcam_device_name(device) == name) {
            return device;
        }
    }
    return nullptr;
}

Glib::RefPtr<Gdk::Pixbuf> webcam::extract_frame(Glib::RefPtr<Gst::Sample> sample,
                                                std::shared_ptr<std::pair<int, int>> resolution) {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    static auto null = Glib::RefPtr<Gdk::Pixbuf>();
    if (!sample) {
        return null;
    }

    auto buffer = sample->get_buffer();
    if (!buffer) {
        return null;
    }
    auto map = Glib::RefPtr<Gst::MapInfo>(new Gst::MapInfo);
    if (!buffer->map(map, Gst::MAP_READ)) {
        return null;
    }
    auto mem = new uint8_t[map->get_size()];
    std::copy(map->get_data(), map->get_data() + map->get_size(), mem);
    buffer->unmap(map);

    return Gdk::Pixbuf::create_from_data(mem,
                                         Gdk::COLORSPACE_RGB,
                                         false,
                                         8,
                                         resolution->first,
                                         resolution->second,
                                         GST_ROUND_UP_4( resolution->first*3 ),
                                         [](const guint8* data) {
        delete[] data;
    });
}
