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
           utils::builder builder):
    Gtk::Frame(cobject) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    builder.get_widget("file_resume", m_file_resume);
    builder.get_widget("file_cancel", m_file_cancel);
    builder.get_widget("file_pause", m_file_pause);
    builder.get_widget("file_progress", m_file_progress);
    builder.get_widget("revealer_download", m_revealer_download);
    builder.get_widget("spinner", m_spinner);
    builder.get_widget("file_open_bar", m_file_open_bar);
    builder.get_widget("file_speed", m_file_speed);
    builder.get_widget("file_size", m_file_size);
    builder.get_widget("file_time", m_file_time);
    builder.get_widget("file_name", m_file_name);
    builder.get_widget("widget_list", m_widget_list);
    builder.get_widget("file_info_box_1", m_file_info_box_1);
    builder.get_widget("file_info_box_2", m_file_info_box_2);
    builder.get_widget("file_dir", m_file_dir);
    builder.get_widget("file_open", m_file_open);
    builder.get_widget("file_control", m_file_control);
    builder.get_widget("preview_image_revealer", m_preview_image_revealer);
    builder.get_widget("widget_list", m_box);

    m_preview_image = builder.get_widget_derived<widget::imagescaled>("preview_image");
    auto preview_video_tmp = widget::videoplayer::create();
    m_preview_video = preview_video_tmp.raw();
    m_box->add(*Gtk::manage(m_preview_video));
}

file::file(BaseObjectType* cobject,
           utils::builder builder,
           const std::shared_ptr<toxmm::file>& file):
    file::file(cobject, builder) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_file = file;

    auto binding_flags = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_name(),
                             m_file_name->property_label(),
                             binding_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_size(),
                             m_file_size->property_label(),
                             binding_flags,
                             [](const uint64_t& input, Glib::ustring& output) {
        utils::debug::scope_log log(DBG_LVL_4("gtox"), {});
        //TODO: Replace the following line with
        //      Glib::format_size(input, G_FORMAT_SIZE_DEFAULT)
        //      but will need Glib 2.45.31 or newer
        output = Glib::convert_return_gchar_ptr_to_ustring(
                     g_format_size_full(input, G_FORMAT_SIZE_IEC_UNITS));
        return true;
    }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_progress(),
                             m_file_progress->property_fraction(),
                             binding_flags));

    //Button handling
    auto proprety_state_update = [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto state = m_file->property_state().get_value();
        bool file_resume = state == TOX_FILE_CONTROL_RESUME;
        bool file_pause  = state == TOX_FILE_CONTROL_PAUSE;
        bool file_cancel = state == TOX_FILE_CONTROL_CANCEL;

        if (file_resume != m_file_resume->property_active()) {
            m_file_resume->property_active() = file_resume;
        }
        if (file_pause != m_file_pause->property_active()) {
            m_file_pause->property_active() = file_pause;
        }
        if (file_cancel != m_file_cancel->property_active()) {
            m_file_cancel->property_active() = file_cancel;
        }
    };
    m_file->property_state()
            .signal_changed()
            .connect(sigc::track_obj(proprety_state_update, *this));
    proprety_state_update();

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_resume->property_active(),
                             m_file_resume->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_pause->property_active(),
                             m_file_pause->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_cancel->property_active(),
                             m_file_cancel->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));

    m_file_resume->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_resume->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_RESUME;
        }
    }, *this));
    m_file_pause->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_pause->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_PAUSE;
        }
    }, *this));
    m_file_cancel->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_cancel->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_CANCEL;
        }
    }, *this));

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_active(),
                             m_file_control->property_sensitive(),
                             binding_flags));

    //Buttons when file complete
    m_file_dir->signal_clicked().connect([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto filemanager = Gio::AppInfo::get_default_for_type("inode/directory",
                                                              true);

        try {
            if (!filemanager) {
                throw std::runtime_error("No filemanager found");
            }
            filemanager->launch_uri(
                    Glib::filename_to_uri(m_file->property_path().get_value()));
        } catch (...) {
            // TODO Show user error if no filemanager
        }
    });
    m_file_open->signal_clicked().connect([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        try {
            Gio::AppInfo::launch_default_for_uri(
                        Glib::filename_to_uri(
                            m_file->property_path().get_value()));
        } catch (...) {
            // TODO Show user error if no filemanager
        }
    });
    //Handle button visibility for open/dir
    m_preview_image_revealer->property_reveal_child() = false;
    m_preview_video->property_reveal_child() = false;

    m_file->property_complete()
            .signal_changed()
            .connect(sigc::mem_fun(*this, &file::update_complete));
    update_complete();

    //Display control
    m_dispatcher.emit([this, binding_flags]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_bindings.push_back(Glib::Binding::bind_property(
                                 m_file->property_complete(),
                                 m_revealer_download->property_reveal_child(),
                                 binding_flags| Glib::BINDING_INVERT_BOOLEAN));
    });
}

utils::builder::ref<file> file::create(const std::shared_ptr<toxmm::file>& file) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    return utils::builder::create_ref<widget::file>(
                "/org/gtox/ui/chat_filerecv.ui",
                "chat_filerecv",
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
                "chat_filerecv",
                std::make_shared<dummy_file>(file_path));
}

void file::update_complete() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_file_open_bar->hide();

    if (m_file->is_recv() && !m_file->property_complete().get_value()) {
        return;
    }

    auto file = Gio::File::create_for_path(
                    m_file->property_path().get_value());
    m_file_open_bar->set_visible(
                Glib::file_test(file->get_path(), Glib::FILE_TEST_EXISTS));

    //try loading the file
    if (!m_preview_thread.joinable()) {
        m_preview_image_revealer->property_reveal_child() = false;
        m_preview_video->property_reveal_child() = false;
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
                        dispatcher.emit([this, img]() {
                            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                            m_preview_image->property_pixbuf() = img;
                            m_preview_image_revealer->property_reveal_child() = true;
                            m_spinner->property_visible() = false;
                        });
                    }
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
                        m_preview_video->property_reveal_child() = true;
                        m_spinner->property_visible() = false;
                    });
                }
            }
        });
    }

    //install monitor
    auto update_file = [this](const Glib::RefPtr<Gio::File>&,
                       const Glib::RefPtr<Gio::File>&,
                       Gio::FileMonitorEvent event_type) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        switch (event_type) {
            case Gio::FILE_MONITOR_EVENT_DELETED:
            case Gio::FILE_MONITOR_EVENT_MOVED :
                m_file_open_bar->hide();
                m_monitor.reset();
                break;
            default:
                //Ignore
                break;
        }
    };
    m_monitor = file->monitor_file(Gio::FILE_MONITOR_SEND_MOVED);
    m_monitor->signal_changed().connect(sigc::track_obj(update_file, *this, m_monitor));
}

file::~file() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}
