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
#ifndef WIDGETCHATFILESEND_H
#define WIDGETCHATFILESEND_H

#include <gtkmm.h>
#include "Helper/gToxObserver.h"
#include "Helper/gToxBuilder.h"
#include "Helper/gToxFileRecv.h"
#include "../VideoPlayer.h"
#include "Helper/Dispatcher.h"
#include "WidgetChatFilePreview.h"

class WidgetChatFileSend: public Gtk::Frame, public gToxObserver {
    private:
        Dispatcher m_dispatcher;

        //gToxFileSend m_send;

        Toxmm::FriendNr m_friend_nr;
        std::string m_path;
        Toxmm::FileNr   m_file_number;

        uint64_t  m_file_size;

        Gtk::ToggleButton* m_file_resume;
        Gtk::ToggleButton* m_file_cancel;
        Gtk::ToggleButton* m_file_pause;

        Gtk::ProgressBar* m_file_progress;
        Gtk::Revealer* m_revealer_download;
        Gtk::Spinner* m_spinner;
        Gtk::Widget* m_file_open_bar;

        Gtk::Label* m_file_speed;
        Gtk::Label* m_file_time;

        sigc::connection m_update_interval;
        sigc::connection m_loaded;

        WidgetChatFilePreview* m_preview;

        size_t m_last_position;

        bool m_first_emit = true;
        bool m_finish = false;

        void init(gToxBuilder builder);

    public:
        WidgetChatFileSend(BaseObjectType* cobject,
                           gToxBuilder builder,
                           gToxObservable* observable,
                           Toxmm::FriendNr nr,
                           Glib::ustring path);

        WidgetChatFileSend(BaseObjectType* cobject,
                           gToxBuilder builder,
                           gToxObservable* observable,
                           Toxmm::FriendNr nr,
                           Glib::ustring path,
                           Toxmm::FileId id,
                           uint64_t filesize);

        static gToxBuilderRef<WidgetChatFileSend> create(gToxObservable* instance,
                                                         Toxmm::FriendNr nr,
                                                         Glib::ustring uri);
        static gToxBuilderRef<WidgetChatFileSend> create(gToxObservable* instance,
                                                         Toxmm::FriendNr nr,
                                                         Glib::ustring uri,
                                                         Toxmm::FileId id,
                                                         uint64_t filesize);

        void observer_handle(const ToxEvent&) override;

        void before_deconstructor();
        ~WidgetChatFileSend();
};

#endif
