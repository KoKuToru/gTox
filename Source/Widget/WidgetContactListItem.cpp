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
#include "Tox/Toxmm.h"
#include "Dialog/DialogContact.h"

WidgetContactListItem* WidgetContactListItem::create(Toxmm::FriendNr nr, bool for_notify) {
    auto builder = Gtk::Builder::create_from_resource(use_mini(for_notify)?
                                                          "/org/gtox/ui/list_item_contact_mini.ui":
                                                          "/org/gtox/ui/list_item_contact.ui");
    WidgetContactListItem* tmp = nullptr;
    builder->get_widget_derived("contact_list_item", tmp);
    tmp->set_contact(nr);
    tmp->set_for_notify(for_notify);
    if (!for_notify) {
        tmp->show();
    }
    return tmp;
}

void WidgetContactListItem::set_contact(Toxmm::FriendNr nr) {
    m_friend_nr = nr;

    refresh();
}

WidgetContactListItem::WidgetContactListItem(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ListBoxRow(cobject),
      m_builder(builder),
      m_friend_nr(0),
      spin(false) {

    m_builder->get_widget("avatar", m_avatar);
    m_builder->get_widget("name", m_name);
    m_builder->get_widget("status", m_status_msg);
    m_builder->get_widget("status_icon", m_status_icon);

    m_tox_callback = [this](const ToxEvent& ev) {
        if ((ev.type() == typeid(Toxmm::EventName)          && ev.get<Toxmm::EventName>().nr == m_friend_nr) ||
            (ev.type() == typeid(Toxmm::EventStatusMessage) && ev.get<Toxmm::EventStatusMessage>().nr == m_friend_nr) ||
            (ev.type() == typeid(Toxmm::EventUserStatus)    && ev.get<Toxmm::EventUserStatus>().nr == m_friend_nr)) {
            refresh();
        } else if ((ev.type() == typeid(Toxmm::EventFriendMessage) && ev.get<Toxmm::EventFriendMessage>().nr == m_friend_nr) ||
                   (ev.type() == typeid(Toxmm::EventFriendAction)  && ev.get<Toxmm::EventFriendAction>().nr == m_friend_nr)) {
            if (!spin) {
                spin = true;
                refresh();
            }
        } else if (ev.type() == typeid(EventStopSpin) && ev.get<EventStopSpin>().nr == m_friend_nr) {
            if (spin) {
                spin = false;
                refresh();
            }
        } else if (ev.type() == typeid(EventActivated)) {
            auto data = ev.get<EventActivated>();
            if (data.nr == m_friend_nr) {
                if (!m_for_notify) {
                    if (!m_chat) {
                        m_chat = std::make_shared<DialogChat>(m_friend_nr);
                    }
                    m_chat->present();
                } else {
                    show();
                }
                return;
            }
        } else if (ev.type() == typeid(EventDestroyChat)) {
            auto data = ev.get<EventDestroyChat>();
            if (data.nr == m_friend_nr) {
                if (!m_for_notify) {
                    if (m_chat) {
                        m_chat.reset();
                    }
                } else {
                    hide();
                }
            }
        }
    };
}

WidgetContactListItem::~WidgetContactListItem() {
}

Toxmm::FriendNr WidgetContactListItem::get_friend_nr() {
    return m_friend_nr;
}

void WidgetContactListItem::refresh() {
    try {
        m_name->set_text(Toxmm::instance().get_name_or_address(m_friend_nr));
        m_status_msg->set_text(Toxmm::instance().get_status_message(m_friend_nr));

        const char* status;

        switch (Toxmm::instance().get_status(m_friend_nr)) {
            case Toxmm::EUSERSTATUS::BUSY:
                status = "/org/gtox/icon/status_busy.png";
                break;
            case Toxmm::EUSERSTATUS::NONE:
                status = "/org/gtox/icon/status_online.png";
                break;
            case Toxmm::EUSERSTATUS::AWAY:
                status = "/org/gtox/icon/status_away.png";
                break;
            default:
                status = "/org/gtox/icon/status_offline.png";
                break;
        }

        if (spin) {
            m_status_icon->set_from_resource(status);
            //m_status_icon->set(generate_animation(ICON::load_icon(*status)));
        } else {
            m_status_icon->set_from_resource(status);
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

Glib::RefPtr<Gdk::PixbufAnimation> WidgetContactListItem::generate_animation(
    const Glib::RefPtr<Gdk::Pixbuf>& icon) {
    /*
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
    */
    return Glib::RefPtr<Gdk::PixbufAnimation>();
}

int WidgetContactListItem::compare(WidgetContactListItem* other) {
    if (m_name < other->m_name) {
        return -1;
    } else if (m_name > other->m_name) {
        return 1;
    }
    return 0;
}

void WidgetContactListItem::set_for_notify(bool notify) {
    m_for_notify = notify;

    m_avatar->set(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.png")->scale_simple(
        use_mini(m_for_notify)?32:64,
        use_mini(m_for_notify)?32:64,
        Gdk::INTERP_BILINEAR));  // i would like to resize this depending
                                 // on font-scale settings
}

bool WidgetContactListItem::use_mini(bool for_notify) {
    return Toxmm::instance().database().config_get(for_notify?"VISUAL_CONTACT_NOTIFY":"VISUAL_CONTACT", for_notify?1:0);
}

void WidgetContactListItem::on_show() {
    Gtk::Widget::on_show();
    Gtk::Revealer* revealer;
    m_builder->get_widget("revealer", revealer);
    revealer->reference();
    Glib::signal_idle().connect_once([revealer](){
        revealer->set_reveal_child(true);
        revealer->unreference();
    });
}

void WidgetContactListItem::on_hide() {
    Gtk::Revealer* revealer;
    m_builder->get_widget("revealer", revealer);
    if (revealer->get_reveal_child()) {
        revealer->set_reveal_child(false);
        revealer->reference();
        Glib::signal_timeout().connect_once([this, revealer](){
            if (!revealer->get_reveal_child()) {
                hide();
            }
            revealer->unreference();
        }, revealer->get_transition_duration());
    } else {
        Gtk::Widget::on_hide();
    }
}
