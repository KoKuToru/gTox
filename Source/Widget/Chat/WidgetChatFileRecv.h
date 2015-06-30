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
#include "Helper/Dispatcher.h"
#include "WidgetChatFilePreview.h"

class WidgetChatFileRecv: public Gtk::Frame, public gToxObserver {
    private:
        Dispatcher m_dispatcher;

        gToxFileRecv m_recv;

        uint32_t  m_friend_nr;
        long long m_file_number;
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

    public:
        WidgetChatFileRecv(BaseObjectType* cobject,
                           gToxBuilder builder,
                           gToxObservable* observable,
                           Toxmm::EventFileRecv file);

        static gToxBuilderRef<WidgetChatFileRecv> create(gToxObservable* instance,
                                          Toxmm::EventFileRecv file);

        void observer_handle(const ToxEvent&) override;

        void before_deconstructor();
        ~WidgetChatFileRecv();
};

#endif
