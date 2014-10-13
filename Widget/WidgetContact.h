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
#ifndef WIDGETCONTACT_H
#define WIDGETCONTACT_H

#include <gtkmm.h>

#include "WidgetContactListItem.h"

class WidgetContact: public Gtk::ScrolledWindow {
    private:
        Gtk::ListBox m_list;

        bool on_button_press(GdkEventButton* event);
    public:
        WidgetContact();
        ~WidgetContact();

        void load_list();
        void add_contact(Tox::FriendNr nr);
        void refresh_contact(Tox::FriendNr nr);
        void delete_contact(Tox::FriendNr nr);
};

#endif
