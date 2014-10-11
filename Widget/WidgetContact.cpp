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
#include "WidgetContact.h"
#include "Tox/Tox.h"
#include "Dialog/DialogContact.h"

WidgetContact::WidgetContact() {
    this->add(m_list);//, true, true);

    m_list.set_activate_on_single_click(false);
    m_list.signal_row_activated().connect([this](Gtk::ListBoxRow* it) {
        WidgetContactListItem* item = dynamic_cast<WidgetContactListItem*>(it);
        DialogContact::instance().activate_chat(item->get_friend_nr());
    });
}

WidgetContact::~WidgetContact() {

}

void WidgetContact::load_list() {
    for(auto fr : Tox::instance().get_friendlist()) {
        add_contact(fr);
    }
}

void WidgetContact::add_contact(Tox::FriendNr nr) {
    Gtk::Widget* w = Gtk::manage(new WidgetContactListItem(this, nr));
    m_list.add(*w);
    w->show_all();
}

void WidgetContact::refresh_contact(Tox::FriendNr nr) {
    for(Gtk::Widget* it : m_list.get_children()) {
        WidgetContactListItem* item = dynamic_cast<WidgetContactListItem*>(it);
        if (item->get_friend_nr() == nr) {
            item->refresh();
        }
    }
}
