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

WidgetContactListItem::WidgetContactListItem(WidgetContact* contact,
                                             Tox::FriendNr nr)
    : Glib::ObjectBase("WidgetContactListItem"),
      m_contact(contact),
      m_friend_nr(nr),
      spin(false) {
    set_name("WidgetContactListItem");

    m_name.set_text(Tox::instance().get_name_or_address(nr));
    m_status_msg.set_text(Tox::instance().get_status_message(nr));

    m_tox_callback = [this, nr](const ToxEvent& ev) {
        if ((ev.type() == typeid(Tox::EventName)          && ev.get<Tox::EventName>().nr == nr) ||
            (ev.type() == typeid(Tox::EventStatusMessage) && ev.get<Tox::EventStatusMessage>().nr == nr) ||
            (ev.type() == typeid(Tox::EventUserStatus)    && ev.get<Tox::EventUserStatus>().nr == nr)) {
            refresh();
        } else if ((ev.type() == typeid(Tox::EventFriendMessage) && ev.get<Tox::EventFriendMessage>().nr == nr) ||
                   (ev.type() == typeid(Tox::EventFriendAction)  && ev.get<Tox::EventFriendAction>().nr == nr) ||
                   (ev.type() == typeid(EventStopSpin)           && ev.get<EventStopSpin>().nr == nr)) {
            if (spin) {
                spin = false;
                refresh();
            }
        }
    };

    m_name.set_line_wrap(false);
    m_name.set_ellipsize(Pango::ELLIPSIZE_END);
    m_name.set_name("Name");

    m_status_msg.set_line_wrap(false);
    m_status_msg.set_ellipsize(Pango::ELLIPSIZE_END);
    m_status_msg.set_name("Status");

    m_avatar.set(ICON::load_icon(ICON::avatar)->scale_simple(
        64,
        64,
        Gdk::INTERP_BILINEAR));  // i would like to resize this depending
                                 // on font-scale settings
    m_avatar.set_name("Avatar");

    m_status_icon.set(ICON::load_icon(ICON::status_offline));
    m_status_icon.set_name("StatusIcon");

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

        const std::string* status = nullptr;

        switch (Tox::instance().get_status(m_friend_nr)) {
            case Tox::EUSERSTATUS::BUSY:
                status = &ICON::status_busy;
                break;
            case Tox::EUSERSTATUS::NONE:
                status = &ICON::status_online;
                break;
            case Tox::EUSERSTATUS::AWAY:
                status = &ICON::status_away;
                break;
            default:
                status = &ICON::status_offline;
                break;
        }

        if (spin) {
            m_status_icon.set(generate_animation(ICON::load_icon(*status)));
        } else {
            m_status_icon.set(ICON::load_icon(*status));
        }
    } catch (...) {
    }
}

std::string replace(std::string str,
                    const std::string& from,
                    const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return str;
    str.replace(start_pos, from.length(), to);
    return str;
}
#include <iostream>
Glib::RefPtr<Gdk::PixbufAnimation> WidgetContactListItem::generate_animation(
    const Glib::RefPtr<Gdk::Pixbuf>& icon) {
    auto ani = gdk_pixbuf_simple_anim_new(24, 24, 30);
    static std::string frames[36];
    for (int ang = 0; ang < 36; ++ang) {
        if (frames[ang].empty()) {
            frames[ang] = replace(ICON::status_message,
                                  "id=\"rotate_me\"",
                                  "id=\"rotate_me\" transform=\"rotate("
                                  + std::to_string(ang * 10) + " 12 12)\"");
        }
        auto front = ICON::load_icon(frames[ang]);
        auto back = icon->copy();
        front->composite(back,
                         0,
                         0,
                         24,
                         24,
                         0,
                         0,
                         1,
                         1,
                         Gdk::InterpType::INTERP_NEAREST,
                         255);
        gdk_pixbuf_simple_anim_add_frame(ani, back->gobj());
    }
    gdk_pixbuf_simple_anim_set_loop(ani, true);
    return Glib::wrap(GDK_PIXBUF_ANIMATION(ani));
}
