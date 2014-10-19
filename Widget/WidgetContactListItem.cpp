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
#include "WidgetContactListItem.h"
#include "WidgetContact.h"
#include "Tox/Tox.h"
#include "Dialog/DialogContact.h"
#include "Generated/icon.h"

WidgetContactListItem::WidgetContactListItem(WidgetContact* contact, Tox::FriendNr nr): m_contact(contact), m_friend_nr(nr) {
    set_name("WidgetContactListItem");

    m_name.set_text(Tox::instance().get_name_or_address(nr));
    m_status_msg.set_text(Tox::instance().get_status_message(nr));

    m_name.set_line_wrap(false);
    m_name.set_ellipsize(Pango::ELLIPSIZE_END);
    m_name.set_name("Name");

    m_status_msg.set_line_wrap(false);
    m_status_msg.set_ellipsize(Pango::ELLIPSIZE_END);
    m_status_msg.set_name("Status");

    m_avatar.set(ICON::load_icon(ICON::avatar)->scale_simple(64, 64, Gdk::INTERP_BILINEAR)); //i would like to resize this depending on font-scale settings
    m_avatar.set_name("Avatar");

    m_status_icon.set(ICON::load_icon(ICON::status_offline));
    m_status_icon.set_name("StatusIcon");

    set_sensitive(false);

    m_layout.attach(m_status_icon, 0, 0, 1, 2);
    m_layout.attach(m_name, 1, 0, 1, 1);
    m_layout.attach(m_status_msg, 1, 1, 1, 1);
    m_layout.attach(m_avatar, 2, 0, 1, 2);

    m_name.set_halign(Gtk::Align::ALIGN_END);
    m_status_msg.set_halign(Gtk::Align::ALIGN_END);

    m_name.set_hexpand(true);
    m_status_msg.set_hexpand(true);

    this->add(m_layout);
}

WidgetContactListItem::~WidgetContactListItem() {

}

Tox::FriendNr WidgetContactListItem::get_friend_nr() {
    return m_friend_nr;
}

void WidgetContactListItem::refresh() {
    try {
        m_name.set_text(Tox::instance().get_name_or_address(m_friend_nr));
        m_status_msg.set_text(Tox::instance().get_status_message(m_friend_nr));

        Glib::RefPtr<Gdk::Pixbuf> status;

        switch(Tox::instance().get_status(m_friend_nr)){
            case Tox::EUSERSTATUS::BUSY:
                status = ICON::load_icon(ICON::status_busy);
                break;
            case Tox::EUSERSTATUS::INVALID:
            case Tox::EUSERSTATUS::NONE:
                status = ICON::load_icon(ICON::status_online);
                break;
            case Tox::EUSERSTATUS::AWAY:
                status = ICON::load_icon(ICON::status_away);
                break;
            default :
                status = ICON::load_icon(ICON::status_offline);
                break;
        }
        m_status_icon.set(status);

        set_sensitive(Tox::instance().get_status(m_friend_nr) != Tox::OFFLINE);
    } catch (...) {

    }
}
