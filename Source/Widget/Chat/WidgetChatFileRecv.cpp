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
    m_recv(observable, file),
    m_friend_nr(file.nr),
    m_file_number(file.file_number),
    m_file_size(file.file_size) {

    builder.get_widget<Gtk::Label>("file_name")->set_text(file.filename);
    double size  = file.file_size;
    int unit = 0;
    while (size > 1024 && unit < 4) {
        size /= 1024;
        unit += 1;
    }
    auto file_size = Glib::ustring::format(std::fixed, std::setprecision(2), size)
                     + " " + s_units[unit];
    builder.get_widget<Gtk::Label>("file_size")->set_text(file_size);

    builder.get_widget("file_resume", m_file_resume);
    builder.get_widget("file_cancel", m_file_cancel);
    builder.get_widget("file_pause", m_file_pause);
    builder.get_widget("revealer_download", m_revealer_download);
    builder.get_widget("spinner", m_spinner);
    builder.get_widget("file_open_bar", m_file_open_bar);

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
    auto file_info_box_1 = builder.get_widget<Gtk::Widget>("file_info_box_1");
    auto file_info_box_2 = builder.get_widget<Gtk::Widget>("file_info_box_2");
    m_file_cancel->signal_toggled().connect([this, file_info_box_1, file_info_box_2](){
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
            file_info_box_1->hide();
            file_info_box_2->hide();
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

    builder.get_widget("file_progress", m_file_progress);

    builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(true);

    auto image = builder.get_widget<Gtk::Image>("image");
    auto revealer_image = builder.get_widget<Gtk::Revealer>("revealer_image");
    m_set_image_connection = signal_set_image().connect([this, image, revealer_image](Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        image->property_pixbuf() = pixbuf;
        m_spinner->hide();
        revealer_image->set_reveal_child(true);
    });

    m_player = builder.get_widget_derived<VideoPlayer>("video");
    auto revelear_video = builder.get_widget<Gtk::Revealer>("revealer_video");
    auto video_seek = builder.get_widget<Gtk::Scale>("video_seek");
    auto video_position = builder.get_widget<Gtk::Label>("video_position");
    auto video_duration = builder.get_widget<Gtk::Label>("video_duration");
    auto video_pos_dur = builder.get_widget<Gtk::Widget>("video_pos_dur");
    auto video_volume = builder.get_widget<Gtk::VolumeButton>("video_volume");
    m_try_video = signal_try_video().connect([this, revelear_video, video_seek, video_position, video_duration, video_pos_dur, video_volume](bool has_video, bool has_audio) {
        has_video = false;
        has_audio = false;
        if (has_video || has_audio) {
            m_player->set_uri(Glib::filename_to_uri(m_recv.get_path()), has_video);
            m_player->set_auto_reset(false);
            m_player->set_volume(0.5);
            video_volume->get_adjustment()->set_value(0.5);
            video_volume->get_adjustment()->signal_value_changed().connect([this, video_volume](){
                m_player->set_volume(video_volume->get_adjustment()->get_value());
            });
            m_player->get_streamer()->signal_error().connect([this, revelear_video](std::string) {
                //not okay
                m_spinner->hide();
                revelear_video->set_reveal_child(false);
            });
            if (has_video) {
                m_update_video = m_player->get_streamer()->signal_update().connect([this, revelear_video](int,int,const std::vector<unsigned char>&) {
                    //m_update_video.disconnect();
                    m_spinner->hide();
                    revelear_video->set_reveal_child(true);
                });
                m_player->get_streamer()->emit_update_signal();
            } else {
                //audio only
                revelear_video->set_reveal_child(true);
                m_spinner->hide();
            }

            m_update_video_interval = Glib::signal_timeout().connect([this, video_seek, video_position, video_duration, video_pos_dur]() {
                gint64 pos = 0, dur = 0;
                auto streamer = m_player->get_streamer();
                if (streamer && streamer->get_progress(pos, dur)) {
                    video_seek->get_adjustment()->set_upper(dur);
                    video_seek->get_adjustment()->set_value(pos);
                    video_position->set_text(
                                Glib::ustring::format(std::setw(3), std::setfill(L'0'), std::right, Gst::get_hours(pos)) + ":" +
                                Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, Gst::get_minutes(pos)) + ":" +
                                Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, Gst::get_seconds(pos)));
                    video_duration->set_text(
                                Glib::ustring::format(std::setw(3), std::setfill(L'0'), std::right, Gst::get_hours(dur)) + ":" +
                                Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, Gst::get_minutes(dur)) + ":" +
                                Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, Gst::get_seconds(dur)));
                    video_pos_dur->show();
                } else {
                    video_pos_dur->hide();
                }
                return true;
            }, 1000);
        } else {
            m_spinner->hide();
        }
    });

    builder.get_widget("file_speed", m_file_speed);
    builder.get_widget("file_time", m_file_time);
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
                    Glib::ustring::format(std::setw(3), std::setfill(L'0'), std::right, h) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, min) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, sec));

        return true;
    }, 500);

    builder.get_widget<Gtk::Button>("file_dir")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::get_default_for_type("inode/directory", true)->launch_uri(Glib::filename_to_uri(m_recv.get_path()));
        } catch (...) {
            //TODO: display error !
        }
    });

    builder.get_widget<Gtk::Button>("file_open")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::launch_default_for_uri(Glib::filename_to_uri(m_recv.get_path()));
        } catch (...) {
            //TODO: display error !
        }
    });

    builder.get_widget("file_recv_video_play", m_file_video_play);
    builder.get_widget("file_recv_video_pause", m_file_video_pause);
    builder.get_widget("file_recv_video_stop", m_file_video_stop);
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
#include <iostream>
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
            m_revealer_download->set_reveal_child(false);

            //check if file exists
            if (Glib::file_test(m_recv.get_path(), Glib::FILE_TEST_IS_REGULAR)) {
                //TODO: check file size for preview generation ?
                m_spinner->show();
                m_file_open_bar->show();

                //try load file
                m_thread = Glib::Thread::create([this](){
                    static std::mutex mutex;
                    std::lock_guard<std::mutex> lg2(mutex); //don't do too much at the same time

                    auto target_size = 512;
                    //try image
                    try {
                        auto ani = Gdk::PixbufAnimation::create_from_file(m_recv.get_path());
                        if (!ani || ani->is_static_image()) {
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
                        }
                    } catch (...) {}

                    //try video
                    bool has_audio;
                    bool has_video;
                    std::tie(has_video, has_audio) = gStreamerVideo::has_video_audio(Glib::filename_to_uri(m_recv.get_path()));
                    std::lock_guard<std::mutex> lg(m_mutex);
                    m_signal_try_video.emit(has_video, has_audio);
                    std::clog << m_recv.get_path() << " has_video:" << has_video << " has_audio:" << has_audio << std::endl;
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
            m_revealer_download->set_reveal_child(true);
            m_file_resume->set_sensitive(true);
            m_first_emit = true;
            m_recv.emit_progress();
        } else {
            m_revealer_download->set_reveal_child(false);
            m_file_pause->clicked();
            m_file_resume->set_sensitive(false);
        }
    }
}

gToxBuilderRef<WidgetChatFileRecv> WidgetChatFileRecv::create(gToxObservable* instance, Toxmm::EventFileRecv file) {
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
        m_update_video_interval.disconnect();
        m_update_video.disconnect();
        m_try_video.disconnect();
    }
    if (m_thread != nullptr) {
        //wait for thread
        m_thread->join();
    }
}
