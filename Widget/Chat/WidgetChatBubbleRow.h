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
#ifndef WIDGETCHATBUBBLEROW_H
#define WIDGETCHATBUBBLEROW_H

#include <string>
#include <gtkmm.h>
#include "Helper/ToxEventCallback.h"

class WidgetChatBubbleRow {
    private:
        Gtk::Frame* m_frame_left;
        Gtk::Frame* m_frame_right;
    public:
        ToxEventCallback tox_callback;
        WidgetChatBubbleRow(Gtk::Grid &grid, int row, Gtk::Widget &left, Gtk::Widget& right);
        void set_class(const std::string& css_class);
        void set_class(bool is_top, bool is_bottom);
};

#endif
