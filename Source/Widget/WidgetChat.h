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
#ifndef WIDGETCHAT_H
#define WIDGETCHAT_H

#include <gtkmm.h>
#include "Chat/WidgetChatEntry.h"
#include "Tox/Tox.h"
#include "Chat/WidgetChatLayout.h"
#include "Helper/ToxEventCallback.h"
#include "WidgetChatLine.h"
// Content of DialogChat
class WidgetChat : public Gtk::VPaned {
  private:
    Gtk::TextView m_input;
    WidgetChatEntry m_output;

    Gtk::HBox m_hbox;
    Gtk::Button m_btn_send;

    Tox::FriendNr m_nr;

    Gtk::ScrolledWindow m_scrolled;
    WidgetChatLayout m_vbox;

    bool m_autoscroll;

    ToxEventCallback m_tox_callback;

    void add_line(Glib::ustring text);

    void add_line(bool left_side, WidgetChatLine::Line new_line);

  public:
    WidgetChat(Tox::FriendNr nr);
    ~WidgetChat();

    Tox::FriendNr get_friend_nr() const;
    void focus();
};

#endif
