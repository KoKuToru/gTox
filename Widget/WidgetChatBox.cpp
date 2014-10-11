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
#include "WidgetChatBox.h"

WidgetChatBox::WidgetChatBox(){
    m_ScrolledWindow.add(m_text_view);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_TextBuffer = Gtk::TextBuffer::create();
    m_TextBuffer->set_text("Hello World!");
    m_text_view.set_buffer(m_TextBuffer);
    pack_start(m_ScrolledWindow);
}

void WidgetChatBox::set_editable(bool b){
    m_text_view.set_editable(b);
}

WidgetChatBox::~WidgetChatBox(){
}

void WidgetChatBox::focus() {
    m_text_view.grab_focus();
}
