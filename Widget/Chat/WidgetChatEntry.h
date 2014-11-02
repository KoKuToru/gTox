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
#ifndef WIDGETCHATBOX_H
#define WIDGETCHATBOX_H

#include <gtkmm.h>
#include "Tox/Tox.h"

class WidgetChat;
class WidgetChatEntry : public Gtk::Box {
  private:
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TextView m_text_view;
    Glib::RefPtr<Gtk::TextBuffer> m_TextBuffer;

  public:
    void set_editable(bool b);
    void focus();

    WidgetChatEntry();
    ~WidgetChatEntry();

    void add_line(const Glib::ustring& text);
};

#endif
