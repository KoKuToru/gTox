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
#include "chat_file.h"
#include "tox/contact/file/file.h"
#include <iostream>
#include <mutex>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace widget;

file::file(BaseObjectType* cobject,
           utils::builder builder,
           const std::shared_ptr<toxmm::file>& file):
    Gtk::Frame(cobject),
    m_file_info_popover(file) {

    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    builder.get_widget("file_name", m_file_name);
    builder.get_widget("spinner", m_spinner);
    builder.get_widget("file_info", m_file_info);
    builder.get_widget("file_info_2", m_file_info_2);
    builder.get_widget("preview_revealer", m_preview_revealer);
    builder.get_widget("preview", m_preview);
    builder.get_widget("info_revealer", m_info_revealer);
    builder.get_widget("eventbox", m_eventbox);

    /*auto preview_video_tmp = widget::videoplayer::create();
    m_preview_video = preview_video_tmp.raw();*/

    m_preview->add(m_preview_image);
    //m_preview->add(*Gtk::manage(m_preview_video));

    m_file = file;

    auto binding_flags = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_name(),
                             m_file_name->property_label(),
                             binding_flags));

    m_file_info->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_file_info_popover.set_relative_to(*m_file_info);
        m_file_info_popover.set_visible();
    }, *this));
    m_file_info_2->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_file_info_popover.set_relative_to(*m_file_info_2);
        m_file_info_popover.set_visible();
    }, *this));

    m_file_info_2->get_style_context()->add_class("gtox-opacity-0");
    m_eventbox->add_events(Gdk::ENTER_NOTIFY_MASK);
    m_eventbox->signal_enter_notify_event()
            .connect(sigc::track_obj([this](GdkEventCrossing*) {
        m_file_info_2->get_style_context()->remove_class("gtox-opacity-0");
        //start time to detect leave
        m_leave_timer.disconnect();
        m_leave_timer = Glib::signal_timeout().connect_seconds(sigc::track_obj([this]() {
            //check if mouse cursor is still in area of the eventbox
            int x, y;
            m_eventbox->get_pointer(x, y);

            if (x < 0 ||
                y < 0 ||
                x > m_eventbox->get_allocated_width() ||
                y > m_eventbox->get_allocated_height()) {
                //we are outside !
                m_file_info_2->get_style_context()->add_class("gtox-opacity-0");
                //stop the timer
                return false;
            }
            return true;
        }, *this), 5);
        return false;
    }, *this));

    m_file->property_complete()
            .signal_changed()
            .connect(sigc::mem_fun(*this, &file::update_complete));
    update_complete();
}

utils::builder::ref<file> file::create(const std::shared_ptr<toxmm::file>& file) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    return utils::builder::create_ref<widget::file>(
                "/org/gtox/ui/chat_filerecv.ui",
                "chat_file",
                file);
}

utils::builder::ref<file> file::create(const Glib::ustring& file_path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { file_path.raw() });
    class dummy_file: virtual public Glib::Object, public toxmm::file {
        public:
            dummy_file(const Glib::ustring& file_path):
                Glib::ObjectBase(typeid(dummy_file)) {
                m_property_name =
                        Gio::File::create_for_path(file_path)->get_basename();
                m_property_path = file_path;
                m_property_complete = true;
            }
            virtual ~dummy_file() {}
        protected:
            virtual bool is_recv() override { return true; }
            virtual void resume() override {}
            virtual void send_chunk_request(uint64_t, size_t) override {}
            virtual void recv_chunk(uint64_t, const std::vector<uint8_t>&) override {};
            virtual void finish() override {}
            virtual void abort() override {}
    };

    return utils::builder::create_ref<widget::file>(
                "/org/gtox/ui/chat_filerecv.ui",
                "chat_file",
                std::make_shared<dummy_file>(file_path));
}

void file::update_complete() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    if (m_file->is_recv() && !m_file->property_complete().get_value()) {
        return;
    }

    auto file = Gio::File::create_for_path(
                    m_file->property_path().get_value());

    //try loading the file
    if (!m_preview_thread.joinable()) {
        m_preview_revealer->property_reveal_child() = false;
        m_spinner->property_visible() = true;
        m_preview_thread = std::thread([this,
                                       dispatcher = utils::dispatcher::ref(m_dispatcher),
                                       file]() {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            static std::mutex limit_mutex;
            std::lock_guard<std::mutex> lg(limit_mutex);
            //TODO: check file size before generating preview ?
            double max_size = 1024; //max size if an image will be 1024x1024

            //try to load image
            auto ani = Gdk::PixbufAnimation
                       ::create_from_file(file->get_path());
            if (ani) {
                if (ani->is_static_image()) {
                    ani.reset();
                    auto img = Gdk::Pixbuf
                               ::create_from_file(file->get_path());
                    if (std::max(img->get_width(), img->get_height()) > max_size) {
                        auto scale_w = max_size / img->get_width();
                        auto scale_h = max_size / img->get_height();
                        auto scale = std::min(scale_w, scale_h);
                        auto w = img->get_width() * scale;
                        auto h = img->get_height() * scale;
                        std::clog << w << "x" << h << std::endl;
                        img = img->scale_simple(int(w), int(h),
                                                Gdk::InterpType::INTERP_BILINEAR);
                    }
                    dispatcher.emit([this, img]() {
                        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                        m_preview_image.property_pixbuf() = img;
                        m_preview_revealer->property_reveal_child() = true;
                        m_spinner->property_visible() = false;
                        m_info_revealer->set_reveal_child(false);
                    });
                } else {
                    //TODO: gif..
                    /*
                      widget::imagescaled should be updated to support
                      animated images..
                    */
                    dispatcher.emit([this]() {
                        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                        m_spinner->property_visible() = false;
                    });
                }
            } else {
                bool has_video;
                bool has_audio;
                std::tie(has_video, has_audio) = utils::gstreamer
                                                 ::has_video_audio(file->get_uri());
                if (has_video || has_audio) {
                    dispatcher.emit([this, file]() {
                        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                        m_preview_video->property_uri() = file->get_uri();
                        //TODO: reveal and stop spinner after
                        //peview image got generated !
                        m_preview_revealer->property_reveal_child() = true;
                        m_spinner->property_visible() = false;
                        m_info_revealer->set_reveal_child(false);
                    });
                }
            }
        });
    }
}

file::~file() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (m_preview_thread.joinable()) {
        m_preview_thread.join();
    }
}
