/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
    Copyright (C) 2014  Maurice Mohlek

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
#ifndef WIDGETCHATLINE_H
#define WIDGETCHATLINE_H

#include <gtkmm.h>
#include "WidgetChatBubbleRow.h"
#include <vector>
#include "Tox/Toxmm.h"
#include "Helper/gToxObserver.h"
#include "Widget/WidgetAvatar.h"
#include "Widget/Chat/WidgetChatFileRecv.h"
#include "Widget/Chat/WidgetChatFileSend.h"

class WidgetChatLabel;

class WidgetChatBubble : public Gtk::Box, public gToxObserver {
    public:
        enum Side {
            NONE,
            LEFT,
            RIGHT
        };

        WidgetChatBubble(gToxObservable* observable, Toxmm::FriendNr nr, Side side);
        ~WidgetChatBubble();

        Side get_side();

        struct Line {
                bool error;
                bool wait_for_receipt;
                Toxmm::ReceiptNr receipt;
                unsigned long long timestamp;
                Toxmm::FriendNr nr;
                Glib::ustring message;
        };

        void add_message(Line new_line);
        void add_filerecv(Toxmm::EventFileRecv file);
        void add_filesend(Toxmm::FriendNr nr, Glib::ustring uri);

        unsigned long long last_timestamp();

        sigc::connection signal_update_avatar_size;

        std::vector<WidgetChatFileRecv*> m_filerecv;
        std::vector<WidgetChatFileSend*> m_filesend;

    private:
      Side m_side;

      int m_row_count;

      unsigned long long m_last_timestamp;

      Gtk::Grid m_grid;
      WidgetAvatar m_avatar;

      void on_size_allocate(Gtk::Allocation& allocation);

      std::vector<WidgetChatBubbleRow> rows;
};

#endif
