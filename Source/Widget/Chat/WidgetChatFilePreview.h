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
#ifndef WIDGETCHATFILEPREVIEW_H
#define WIDGETCHATFILEPREVIEW_H

#include <gtkmm.h>
#include "Helper/gToxBuilder.h"
#include "../VideoPlayer.h"
#include "Helper/Dispatcher.h"

class WidgetChatFilePreview: public Gtk::Revealer{
    private:
        Dispatcher m_dispatcher;
        bool m_run = true;

        Glib::ustring m_uri;

        Gtk::ToggleButton* m_file_video_play;
        Gtk::ToggleButton* m_file_video_stop;
        Gtk::ToggleButton* m_file_video_pause;

        Glib::Thread* m_thread = nullptr;

        typedef sigc::signal<void, Glib::RefPtr<Gdk::Pixbuf>> type_signal_set_image;
        type_signal_set_image signal_set_image() {
            return m_signal_set_image;
        }

        typedef sigc::signal<void, bool, bool> type_signal_try_video;
        type_signal_try_video signal_try_video() {
            return m_signal_try_video;
        }

        typedef sigc::signal<void, bool> type_signal_loaded;

        sigc::connection m_set_image_connection;
        sigc::connection m_try_video;
        sigc::connection m_update_video_interval;
        sigc::connection m_update_video;

        VideoPlayer* m_player;

    public:
        WidgetChatFilePreview(BaseObjectType* cobject,
                           gToxBuilder builder,
                           Glib::ustring uri);

        static gToxBuilderRef<WidgetChatFilePreview> create(Glib::ustring uri);

        void start_loading();
        type_signal_loaded signal_loaded() {
            return m_signal_loaded;
        }

        void before_deconstructor();
        ~WidgetChatFilePreview();

    protected:
        type_signal_set_image m_signal_set_image;
        type_signal_try_video m_signal_try_video;
        type_signal_loaded    m_signal_loaded;
};

#endif
