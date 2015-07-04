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
#include "Tox/Toxmm.h"
#include "Chat/WidgetChatLayout.h"
#include "Chat/WidgetChatBubble.h"
#include "Dialog/DialogContact.h"
#include "Helper/gToxObserver.h"
#include "Chat/WidgetChatTextView.h"

// Content of DialogChat
class WidgetChat : public Gtk::VPaned, public gToxObserver {
  private:
    WidgetChatTextView m_input;

    Gtk::HBox m_hbox;
    Gtk::Button m_btn_send;

    Toxmm::FriendNr m_nr;

    Gtk::ScrolledWindow m_scrolled;
    WidgetChatLayout m_vbox;

    bool m_autoscroll;

    gToxObservable::Handler m_tox_callback;

    unsigned long long m_last_timestamp;
    WidgetChatBubble::Side m_last_side = WidgetChatBubble::NONE;

    void add_message(WidgetChatBubble::Side side, WidgetChatBubble::Line message);
    void add_filerecv(std::shared_ptr<gToxFileTransf> file);
    void add_filesend(ToxLogEntity data);
    void add_widget(Gtk::Widget& widget);

    bool same_bubble(unsigned long long a_timestamp, WidgetChatBubble::Side a_side, unsigned long long b_timestamp, WidgetChatBubble::Side b_side);
    bool need_date(unsigned long long a_timestamp, unsigned long long b_timestamp);

  public:
    WidgetChat(gToxObservable* observable, Toxmm::FriendNr nr);
    ~WidgetChat();

    void add_filesend(Glib::ustring uri);

    Toxmm::FriendNr get_friend_nr() const;
    void focus();


};

#endif
