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
#ifndef WIDGETCHATFILERECV_H
#define WIDGETCHATFILERECV_H

#include <gtkmm.h>
#include "Helper/gToxObserver.h"
#include "Helper/gToxBuilder.h"
#include "Helper/gToxFileRecv.h"
#include "../VideoPlayer.h"

class WidgetChatFileRecv: public Gtk::Frame, public gToxObserver {
    private:
        gToxBuilder m_builder;
        gToxFileRecv m_recv;

        uint32_t m_friend_nr;
        long long m_file_number;
        uint64_t m_file_size;

        Gtk::ToggleButton* m_file_resume;
        Gtk::ToggleButton* m_file_cancel;
        Gtk::ToggleButton* m_file_pause;

        Gtk::ToggleButton* m_file_video_play;
        Gtk::ToggleButton* m_file_video_stop;
        Gtk::ToggleButton* m_file_video_pause;

        Gtk::ProgressBar* m_file_progress;

        Gtk::Label* m_file_speed;
        Gtk::Label* m_file_time;

        Glib::Thread* m_thread;
        std::mutex m_mutex;

        typedef sigc::signal<void, Glib::RefPtr<Gdk::Pixbuf>> type_signal_set_image;
        type_signal_set_image signal_set_image() {
            return m_signal_set_image;
        }
        typedef sigc::signal<void> type_signal_try_video;
        type_signal_try_video signal_try_video() {
            return m_signal_try_video;
        }

        sigc::connection m_set_image_connection;
        sigc::connection m_try_video;
        sigc::connection m_update_interval;

        VideoPlayer* m_player;

        size_t m_last_position;

        bool m_first_emit = true;
        bool m_finish = false;

    public:
        WidgetChatFileRecv(BaseObjectType* cobject,
                           gToxBuilder builder,
                           gToxObservable* observable,
                           Toxmm::EventFileRecv file);

        static WidgetChatFileRecv* create(gToxObservable* instance,
                                          Toxmm::EventFileRecv file);

        void observer_handle(const ToxEvent&) override;

        ~WidgetChatFileRecv();

    protected:
        type_signal_set_image m_signal_set_image;
        type_signal_try_video m_signal_try_video;
};

#endif
