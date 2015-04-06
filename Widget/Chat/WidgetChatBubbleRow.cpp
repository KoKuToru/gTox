/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#include "WidgetChatBubbleRow.h"

WidgetChatBubbleRow::WidgetChatBubbleRow(Gtk::Grid &grid, int row, Gtk::Widget &left, Gtk::Widget& right) {
    m_frame_left = Gtk::manage(new Gtk::Frame());
    m_frame_right = Gtk::manage(new Gtk::Frame());
    m_frame_left->get_style_context()->add_class("bubble_chat_line");
    m_frame_right->get_style_context()->add_class("bubble_chat_line");
    grid.attach(*m_frame_left, 0, row, 1, 1);
    grid.attach(*m_frame_right, 1, row, 1, 1);
    m_frame_left->add(left);
    m_frame_right->add(right);
    m_frame_left->show();
    m_frame_right->show();
}

void WidgetChatBubbleRow::set_class(const std::string& css_class) {
    for(auto w : {m_frame_left, m_frame_right}) {
        auto css = w->get_style_context();
        for(auto c : css->list_classes()) {
            css->remove_class(c);
        }
        css->add_class("bubble_chat_line");
        css->add_class(css_class);
    }
}

void WidgetChatBubbleRow::set_class(bool is_top, bool is_bottom) {
    for(auto w : {m_frame_left, m_frame_right}) {
        auto css = w->get_style_context();
        css->remove_class("bubble_chat_line");
        css->add_class("bubble_chat_line");
        css->remove_class("top_left");
        css->remove_class("top_right");
        css->remove_class("bottom_left");
        css->remove_class("bottom_right");
        if (w == m_frame_left) {
            if (is_top) {
                css->add_class("top_left");
            }
            if (is_bottom) {
                css->add_class("bottom_left");
            }
        } else {
            if (is_top) {
                css->add_class("top_right");
            }
            if (is_bottom) {
                css->add_class("bottom_right");
            }
        }
    }
}
