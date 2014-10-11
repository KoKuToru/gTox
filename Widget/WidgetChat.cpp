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
#include "WidgetChat.h"
#include "Tox/Tox.h"

WidgetChat::WidgetChat(Tox::FriendNr nr): m_nr(nr) {

    m_output.set_editable(false);
    pack1(m_output, true, false);
    //pack2(input, false, true);

    m_btn_send.set_label("Send");

    m_hbox.pack_start(m_input, true, true);
    m_hbox.pack_end(m_btn_send, false, false);
    pack2(m_hbox, false, false);

    set_position(400);

    m_btn_send.signal_clicked().connect([this](){
        Tox::instance().send_message(get_friend_nr(), m_input.get_buffer()->get_text());
        add_line("me:" + m_input.get_buffer()->get_text() + "\n");
    });
}

WidgetChat::~WidgetChat() {

}

void WidgetChat::focus() {
    m_input.grab_focus();
}

Tox::FriendNr WidgetChat::get_friend_nr() const {
    return m_nr;
}

void WidgetChat::add_line(Glib::ustring text) {
    m_output.add_line(text);
}
