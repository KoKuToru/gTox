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
#include "WidgetChatLine.h"

WidgetChat::WidgetChat(Tox::FriendNr nr): m_nr(nr) {

    m_output.set_editable(false);
    m_scrolled.add(m_vbox);
    m_vbox.set_spacing(5);
    auto frame = Gtk::manage(new Gtk::Frame());
    frame->add(m_scrolled);
    pack1(*frame, true, false);
    //pack2(input, false, true);

    m_btn_send.set_label("Send");

    m_hbox.pack_start(m_input, true, true);
    m_hbox.pack_end(m_btn_send, false, false);
    pack2(m_hbox, false, false);

    //set_position(400);
    m_hbox.set_size_request(-1, 80);

    m_btn_send.signal_clicked().connect([this](){
        try {
            Tox::instance().send_message(get_friend_nr(), m_input.get_buffer()->get_text());
            add_line(0, false, m_input.get_buffer()->get_text());
            m_input.get_buffer()->set_text("");
        } catch(...) {
            //not online ?
        }
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

void WidgetChat::add_line(unsigned long long timestamp, bool left_side, const Glib::ustring& message) {
    std::vector<Gtk::Widget*> childs = m_vbox.get_children();
    if (!childs.empty()) {
        WidgetChatLine* item = dynamic_cast<WidgetChatLine*>(childs.back());
        if (item != nullptr) {
            if (item->get_side() == left_side) {
                item->add_line(timestamp, message);
                //scroll down
                auto adj = m_scrolled.get_vadjustment();
                adj->set_value(adj->get_upper() - adj->get_page_size());
                return;
            }
        }
    }
    //add new line
    auto new_line = Gtk::manage(new WidgetChatLine(left_side));
    new_line->add_line(timestamp, message);
    new_line->show_all();
    m_vbox.pack_start(*new_line, false, false);
    //scroll down
    auto adj = m_scrolled.get_vadjustment();
    adj->set_value(adj->get_upper() - adj->get_page_size());
}
