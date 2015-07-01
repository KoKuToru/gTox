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
#include "WidgetChatFilePreview.h"
#include <iomanip>
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

WidgetChatFilePreview::WidgetChatFilePreview(BaseObjectType* cobject,
                                       gToxBuilder builder,
                                       Glib::ustring uri):
    Gtk::Revealer(cobject),
    m_uri(uri) {

    auto image = builder.get_widget<Gtk::Image>("image");
    m_set_image_connection = signal_set_image().connect([this, image](Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        image->property_pixbuf() = pixbuf;
        image->show();
        set_reveal_child(true);
        m_signal_loaded.emit(true);
    });

    m_player = builder.get_widget_derived<VideoPlayer>("video");
    auto video_control = builder.get_widget<Gtk::Widget>("video_control");
    auto video_seek = builder.get_widget<Gtk::Scale>("video_seek");
    auto video_position = builder.get_widget<Gtk::Label>("video_position");
    auto video_duration = builder.get_widget<Gtk::Label>("video_duration");
    auto video_pos_dur = builder.get_widget<Gtk::Widget>("video_pos_dur");
    auto video_volume = builder.get_widget<Gtk::VolumeButton>("video_volume");
    m_try_video = signal_try_video().connect([this, video_control, video_seek, video_position, video_duration, video_pos_dur, video_volume](bool has_video, bool has_audio) {
        if (has_video || has_audio) {
            m_player->set_uri(m_uri, has_video);
            m_player->set_auto_reset(false);
            m_player->set_volume(0.5);
            video_volume->get_adjustment()->set_value(0.5);
            video_volume->get_adjustment()->signal_value_changed().connect([this, video_volume](){
                m_player->set_volume(video_volume->get_adjustment()->get_value());
            });
            m_player->get_streamer()->signal_error().connect([this, video_control](std::string) {
                //not okay
                m_signal_loaded.emit(false);
            });
            if (has_video) {
                m_update_video = m_player->get_streamer()->signal_update().connect([this, video_control](int,int,const std::vector<unsigned char>&) {
                    video_control->show();
                    set_reveal_child(true);
                    m_signal_loaded.emit(true);
                });
                m_player->get_streamer()->emit_update_signal();
            } else {
                //audio only
                video_control->show();
                set_reveal_child(true);
                m_signal_loaded.emit(true);
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
            m_signal_loaded.emit(false);
        }
    });
    auto video_control_revealer = builder.get_widget<Gtk::Revealer>("video_control_revealer");
    auto video_control_box = builder.get_widget<Gtk::Widget>("video_control_box");
    int min_height, min_width;
    int natural_height, natural_width;
    video_control_box->get_preferred_height(min_height, natural_height);
    video_control_box->get_preferred_width(min_width, natural_width);
    m_player->set_size_request(min_width + 10, min_height + 10); //+10 because padding 5 px in xml..
    video_control->add_events(Gdk::POINTER_MOTION_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    video_control->signal_enter_notify_event().connect_notify([this, video_control_revealer](GdkEventCrossing*) {
        video_control_revealer->set_reveal_child(true);
    });
    video_control->signal_leave_notify_event().connect_notify([this, video_control_revealer](GdkEventCrossing*) {
        video_control_revealer->set_reveal_child(false);
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
}

void WidgetChatFilePreview::start_loading() {
    //check if file exists
    auto path = Glib::filename_from_uri(m_uri);
    if (Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR)) {
        //TODO: check file size for preview generation ?
        //try load file
        m_thread = Glib::Thread::create([this, path](){
            static std::mutex mutex;
            std::lock_guard<std::mutex> lg2(mutex); //don't do too much at the same time
            if (!m_run) {
                return;
            }

            auto target_size = 512;
            //try image
            try {
                auto ani = Gdk::PixbufAnimation::create_from_file(path);
                if (!ani || ani->is_static_image()) {
                    auto image = Gdk::Pixbuf::create_from_file(path);
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
                        m_dispatcher.emit([this, image](){
                            m_signal_set_image.emit(image);
                        });
                        return;
                    }
                }
            } catch (...) {}

            if (!m_run) {
                return;
            }

            //try video
            bool has_audio;
            bool has_video;
            std::tie(has_video, has_audio) = gStreamerVideo::has_video_audio(m_uri);
            m_dispatcher.emit([this, has_audio, has_video](){
                m_signal_try_video.emit(has_video, has_audio);
            });
        });
    } else {
        m_signal_loaded.emit(false);
    }
}

gToxBuilderRef<WidgetChatFilePreview> WidgetChatFilePreview::create(Glib::ustring uri) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filepreview.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetChatFilePreview>("chat_filepreview", uri);
}

WidgetChatFilePreview::~WidgetChatFilePreview() {
    before_deconstructor();
    if (m_thread != nullptr) {
        //wait for thread
        m_thread->join();
    }
}

void WidgetChatFilePreview::before_deconstructor() {
    m_set_image_connection.disconnect();
    m_update_video_interval.disconnect();
    m_update_video.disconnect();
    m_try_video.disconnect();
    m_run = false;
}
