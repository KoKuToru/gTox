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
#include "WidgetChatFileRecv.h"
#include <iomanip>
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

static Glib::ustring s_units[] = {"Bytes", "KiB", "MiB", "GiB"};

WidgetChatFileRecv::WidgetChatFileRecv(BaseObjectType* cobject,
                                       gToxBuilder builder,
                                       gToxObservable* observable,
                                       Toxmm::EventFileRecv file):
    Gtk::Frame(cobject),
    gToxObserver(observable),
    m_builder(builder),
    m_recv(observable, file),
    m_friend_nr(file.nr),
    m_file_number(file.file_number),
    m_file_size(file.file_size) {

    m_builder.get_widget<Gtk::Label>("file_name")->set_text(file.filename);
    double size  = file.file_size;
    int unit = 0;
    while (size > 1024 && unit < 4) {
        size /= 1024;
        unit += 1;
    }
    auto file_size = Glib::ustring::format(std::fixed, std::setprecision(2), size)
                     + " " + s_units[unit];
    m_builder.get_widget<Gtk::Label>("file_size")->set_text(file_size);

    m_builder.get_widget("file_resume", m_file_resume);
    m_builder.get_widget("file_cancel", m_file_cancel);
    m_builder.get_widget("file_pause", m_file_pause);

    m_file_pause->set_active(true);

    m_file_resume->signal_toggled().connect([this](){
        if (m_file_resume->get_active()) {
            if (m_file_cancel->get_active()) {
                m_file_cancel->set_active(false);
            }
            if (m_file_pause->get_active()) {
                m_file_pause->set_active(false);
            }
            m_file_speed->show();
            m_file_time->show();
            m_file_resume->set_sensitive(false);
            try {
                m_recv.resume();
            } catch (...) {
                m_file_cancel->clicked();
            }
        } else {
            m_file_resume->set_sensitive(true);
        }
    });
    m_file_cancel->signal_toggled().connect([this](){
        if (m_file_cancel->get_active()) {
            if (m_file_resume->get_active()) {
                m_file_resume->set_active(false);
            }
            if (m_file_pause->get_active()) {
                m_file_pause->set_active(false);
            }
            try {
                m_recv.cancel();
            } catch (...) {
                //TODO ?
            }
            m_builder.get_widget<Gtk::Widget>("file_info_box_1")->hide();
            m_builder.get_widget<Gtk::Widget>("file_info_box_2")->hide();
            m_file_cancel->set_sensitive(false);
            m_update_interval.disconnect();
            m_file_progress->hide();
        } else {
            m_file_cancel->set_sensitive(true);
        }
    });
    m_file_pause->signal_toggled().connect([this](){
        if (m_file_pause->get_active()) {
            if (m_file_cancel->get_active()) {
                m_file_cancel->set_active(false);
            }
            if (m_file_resume->get_active()) {
                m_file_resume->set_active(false);
            }
            m_file_speed->hide();
            m_file_time->hide();
            m_file_pause->set_sensitive(false);
            try {
                m_recv.pause();
            } catch (...) {
                m_file_cancel->clicked();
            }
        } else {
            m_file_pause->set_sensitive(true);
        }
    });

    m_builder.get_widget("file_progress", m_file_progress);

    m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(true);

    m_set_image_connection = signal_set_image().connect([this](Glib::RefPtr<Gdk::Pixbuf> image) {
        m_builder.get_widget<Gtk::Image>("image")->property_pixbuf() = image;
        m_builder.get_widget<Gtk::Widget>("spinner")->hide();
        m_builder.get_widget<Gtk::Revealer>("revealer_image")->set_reveal_child(true);
    });

    m_player = m_builder.get_widget_derived<VideoPlayer>("video");
    m_try_video = signal_try_video().connect([this]() {
        m_player->set_uri("file://"+m_recv.get_path());
        m_player->get_streamer()->signal_error().connect([this](std::string) {
            //not okay
            m_builder.get_widget<Gtk::Widget>("spinner")->hide();
        });
        m_player->get_streamer()->signal_update().connect([this](int,int,const std::vector<unsigned char>&){
            m_builder.get_widget<Gtk::Widget>("spinner")->hide();
            m_builder.get_widget<Gtk::Revealer>("revealer_video")->set_reveal_child(true);
        });
        m_player->get_streamer()->emit_update_signal();
    });

    m_builder.get_widget("file_speed", m_file_speed);
    m_builder.get_widget("file_time", m_file_time);
    size_t dummy;
    m_recv.get_progress(m_last_position, dummy);
    m_update_interval = Glib::signal_timeout().connect([this](){
        size_t position, size;
        m_recv.get_progress(position, size);
        size_t diff = position - m_last_position;
        m_last_position = position;

        double s  = diff / 0.5; //each 500 ms

        double sf = s;
        int unit = 0;
        while (sf > 1024 && unit < 4) {
            sf /= 1024;
            unit += 1;
        }

        m_file_speed->set_text(Glib::ustring::format(std::fixed, std::setprecision(2), sf)
                               + " " + s_units[unit] + "/s");

        if (s < 1) { //1 Bytes/s
            m_file_time->set_text(_("FILE_RECV_LESS_THAN_1_BYTES_EACH_SEC"));
            return true;
        }

        //calculate time it takes to download rest
        s = (size - position) / s;

        int sec = int(s) % 60;
        int min = (int(s) / 60) % 60;
        int h   = (int(s) / 60) / 60;

        m_file_time->set_text(
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(h)) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(min)) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(sec)));

        return true;
    }, 500);

    m_builder.get_widget<Gtk::Button>("file_dir")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::get_default_for_type("inode/directory", true)->launch_uri("file://"+m_recv.get_path());
        } catch (...) {
            //TODO: display error !
        }
    });

    m_builder.get_widget<Gtk::Button>("file_open")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::launch_default_for_uri("file://"+m_recv.get_path());
        } catch (...) {
            //TODO: display error !
        }
    });

    m_builder.get_widget("file_recv_video_play", m_file_video_play);
    m_builder.get_widget("file_recv_video_pause", m_file_video_pause);
    m_builder.get_widget("file_recv_video_stop", m_file_video_stop);
    m_file_video_play->signal_toggled().connect([this](){
        if (m_file_video_play->get_active()) {
            if (m_file_video_stop->get_active()) {
                m_file_video_stop->set_active(false);
            }
            if (m_file_video_pause->get_active()) {
                m_file_video_pause->set_active(false);
            }
            m_file_video_pause->set_sensitive(true);
            m_file_video_play->set_sensitive(false);
            m_player->play();
        } else {
            m_file_video_play->set_sensitive(true);
        }
    });
    m_file_video_stop->signal_toggled().connect([this](){
        if (m_file_video_stop->get_active()) {
            if (m_file_video_play->get_active()) {
                m_file_video_play->set_active(false);
            }
            if (m_file_video_pause->get_active()) {
                m_file_video_pause->set_active(false);
            }
            m_file_video_pause->set_sensitive(false);
            m_file_video_stop->set_sensitive(false);
            m_player->stop();
        } else {
            m_file_video_stop->set_sensitive(true);
        }
    });
    m_file_video_pause->signal_toggled().connect([this](){
        if (m_file_video_pause->get_active()) {
            if (m_file_video_stop->get_active()) {
                m_file_video_stop->set_active(false);
            }
            if (m_file_video_play->get_active()) {
                m_file_video_play->set_active(false);
            }
            m_file_video_pause->set_sensitive(false);
            m_player->pause();
        } else {
            m_file_video_pause->set_sensitive(true);
        }
    });
    m_file_video_stop->set_active(true);

    m_recv.emit_progress();
}

void WidgetChatFileRecv::observer_handle(const ToxEvent& event) {
    //nothing yet
    if (event.type() == typeid(gToxFileRecv::EventFileProgress)) {
        auto data = event.get<gToxFileRecv::EventFileProgress>();

        if (m_file_number < 0 && (!m_first_emit || m_finish)) {
            return;
        }

        if (data.file_number != m_file_number || data.nr != m_friend_nr || data.file_path != m_recv.get_path()) {
            return;
        }

        m_file_progress->set_fraction(data.file_position / (double)data.file_size);

        if (data.file_position == data.file_size) {
            m_finish = true;

            //finish !
            m_update_interval.disconnect();

            //display
            m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(false);

            //check if file exists
            if (Glib::file_test(m_recv.get_path(), Glib::FILE_TEST_IS_REGULAR)) {
                //TODO: check file size for preview generation ?
                m_builder.get_widget<Gtk::Widget>("spinner")->show();
                m_builder.get_widget<Gtk::Widget>("file_open_bar")->show();

                //try load file
                m_thread = Glib::Thread::create([this](){
                    auto target_size = 512;
                    //try image;
                    try {
                        auto image = Gdk::Pixbuf::create_from_file(m_recv.get_path());
                        if (image->get_width() > target_size || image->get_height() > target_size) {
                            int w, h;
                            if (image->get_width() > image->get_height()) {
                                w = target_size;
                                h = image->get_height()*target_size/image->get_width();
                            } else {
                                h = target_size;
                                w = image->get_width()*target_size/image->get_height();
                            }
                            image = image->scale_simple(w, h, Gdk::InterpType::INTERP_BILINEAR);
                        }
                        if (image) {
                            std::lock_guard<std::mutex> lg(m_mutex);
                            m_signal_set_image.emit(image);
                            return;
                        }
                    } catch (...) {}

                    //try video
                    std::lock_guard<std::mutex> lg(m_mutex);
                    m_signal_try_video.emit();
                });
            } else {
                //nothing todo
            }
        } else if (m_first_emit) {
            m_first_emit = false;
            if (!m_finish) {
                //auto start TODO:check in config
                if (tox().get_status(m_friend_nr) != Toxmm::OFFLINE) {
                    if (m_file_size < 1024*1024) {
                        m_file_resume->clicked();
                    }
                } else {
                    m_file_resume->set_sensitive(false);
                }
            }
        }
    } else if (event.type() == typeid(Toxmm::EventUserStatus)) {
        auto data = event.get<Toxmm::EventUserStatus>();

        if (data.nr != m_friend_nr || m_finish) {
            return;
        }

        if (tox().get_status(m_friend_nr) != Toxmm::OFFLINE) {
            m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(true);
            m_file_resume->set_sensitive(true);
            m_first_emit = true;
            m_recv.emit_progress();
        } else {
            m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(false);
            m_file_pause->clicked();
            m_file_resume->set_sensitive(false);
        }
    }
}

WidgetChatFileRecv* WidgetChatFileRecv::create(gToxObservable* instance, Toxmm::EventFileRecv file) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetChatFileRecv>("chat_filerecv",
                                                    instance,
                                                    file);
}

WidgetChatFileRecv::~WidgetChatFileRecv() {
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_set_image_connection.disconnect();
    }
    if (m_thread != nullptr) {
        //wait for thread
        m_thread->join();
    }
    m_player->stop();
}
