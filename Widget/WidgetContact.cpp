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
#include "../Popover/PopoverContextContact.h"

WidgetContact::WidgetContact() : Glib::ObjectBase("WidgetContact") {
    this->add(m_list);  //, true, true);

    m_list.set_activate_on_single_click(false);
    m_list.signal_row_activated().connect([this](Gtk::ListBoxRow* it) {
        WidgetContactListItem* item = dynamic_cast<WidgetContactListItem*>(it);
        DialogContact::instance().activate_chat(item->get_friend_nr());
    });
    m_list.signal_button_press_event().connect(
        sigc::mem_fun(this, &WidgetContact::on_button_press));
    for (auto fr : Tox::instance().get_friendlist()) {
        add_contact(fr);
    }
}

WidgetContact::~WidgetContact() {
}

void WidgetContact::add_contact(Tox::FriendNr nr) {
    Gtk::Widget* w = Gtk::manage(new WidgetContactListItem(this, nr));
    m_list.add(*w);
    w->show_all();
}

bool WidgetContact::on_button_press(GdkEventButton* event) {
    // check for right click
    if (event->button != 3) {
        return false;
    }
    WidgetContactListItem* item
        = dynamic_cast<WidgetContactListItem*>(m_list.get_row_at_y(event->y));
    if (item == nullptr) {
        return false;
    }
    auto popover
        = Gtk::manage(new PopoverContextContact(item->get_friend_nr()));
    popover->set_relative_to(*item);
    popover->set_visible(true);
    return true;
}

void WidgetContact::delete_contact(Tox::FriendNr nr) {
    for (Gtk::Widget* it : m_list.get_children()) {
        WidgetContactListItem* item = dynamic_cast<WidgetContactListItem*>(it);
        if (item->get_friend_nr() == nr) {
            m_list.remove(*item);
            // free memory
            delete item;
            return;
        }
    }
}
