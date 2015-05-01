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
#include "Helper/Canberra.h"

WidgetContactListItem* WidgetContactListItem::create(gToxObservable* instance, Toxmm::FriendNr nr, bool for_notify) {
    gToxObserver dummy(instance);
    auto builder = Gtk::Builder::create_from_resource(use_mini(&dummy, for_notify)?
                                                          "/org/gtox/ui/list_item_contact_mini.ui":
                                                          "/org/gtox/ui/list_item_contact.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetContactListItem>("contact_list_item",
                                                       instance,
                                                       nr,
                                                       for_notify);
}

void WidgetContactListItem::observer_handle(const ToxEvent& ev) {
    if ((ev.type() == typeid(Toxmm::EventName)          && ev.get<Toxmm::EventName>().nr == m_friend_nr) ||
            (ev.type() == typeid(Toxmm::EventStatusMessage) && ev.get<Toxmm::EventStatusMessage>().nr == m_friend_nr) ||
            (ev.type() == typeid(Toxmm::EventUserStatus)    && ev.get<Toxmm::EventUserStatus>().nr == m_friend_nr)) {
        refresh();
    } else if ((ev.type() == typeid(Toxmm::EventFriendMessage) && ev.get<Toxmm::EventFriendMessage>().nr == m_friend_nr) ||
               (ev.type() == typeid(Toxmm::EventFriendAction)  && ev.get<Toxmm::EventFriendAction>().nr == m_friend_nr)) {
        if (!m_spin->property_active() && (m_for_notify || !m_chat)) {
            m_spin->start();
        }
        if (!m_for_notify && !m_chat) {
            notify(tox().get_name_or_address(m_friend_nr),
                   (ev.type() == typeid(Toxmm::EventFriendMessage)) ?
                       ev.get<Toxmm::EventFriendMessage>().message :
                       ev.get<Toxmm::EventFriendAction>().message);
        }
    } else if (ev.type() == typeid(EventStopSpin) && ev.get<EventStopSpin>().nr == m_friend_nr) {
        if (m_spin->property_active()) {
            m_spin->stop();
        }
    } else if (ev.type() == typeid(EventActivated)) {
        auto data = ev.get<EventActivated>();
        if (data.nr == m_friend_nr) {
            if (!m_for_notify) {
                if (!m_chat) {
                    //Hide notification !
                    if (m_notify) {
                        try {
                            m_notify->close();
                        } catch (...) {
                            //Ignore
                        }
                    }
                    m_chat = std::make_shared<DialogChat>(observable(), m_friend_nr);
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
}

WidgetContactListItem::WidgetContactListItem(BaseObjectType* cobject, gToxBuilder builder,
                                             gToxObservable* observable,
                                             Toxmm::FriendNr nr,
                                             bool for_notify)
    : Gtk::ListBoxRow(cobject),
      gToxObserver(observable),
      m_builder(builder),
      m_friend_nr(nr),
      m_for_notify(for_notify) {

    //m_builder.get_widget("avatar", m_avatar);
    m_avatar = m_builder.get_widget_derived<WidgetAvatar>("avatar", observable, nr);
    m_builder.get_widget("name", m_name);
    m_builder.get_widget("status", m_status_msg);
    m_builder.get_widget("status_icon", m_status_icon);
    m_builder.get_widget("spinner", m_spin);
    m_spin->stop();

    /*
    auto addr = tox().get_address(nr);
    auto avatar_path = Glib::build_filename(Glib::get_user_config_dir(),
                                            "tox", "avatars",
                                            Toxmm::to_hex(addr.data(), TOX_PUBLIC_KEY_SIZE) + ".png");
    if (!Glib::file_test(avatar_path, Glib::FILE_TEST_IS_REGULAR)) {
        m_avatar->set(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg")->scale_simple(
                          use_mini(this, m_for_notify)?32:64,
                          use_mini(this, m_for_notify)?32:64,
                          Gdk::INTERP_BILINEAR));
    } else {
        m_avatar->set(Gdk::Pixbuf::create_from_file(avatar_path)->scale_simple(
                          use_mini(this, m_for_notify)?32:64,
                          use_mini(this, m_for_notify)?32:64,
                          Gdk::INTERP_BILINEAR));
    }*/
    refresh();

    if (!for_notify) {
        show();
    }
}

WidgetContactListItem::~WidgetContactListItem() {
    if (m_notify) {
        try  {
            m_notify->close();
        } catch (...){
            //ignore
        }
    }
}

Toxmm::FriendNr WidgetContactListItem::get_friend_nr() {
    return m_friend_nr;
}

void WidgetContactListItem::refresh() {
    try {
        m_name->set_text(tox().get_name_or_address(m_friend_nr));
        m_status_msg->set_text(tox().get_status_message(m_friend_nr));

        const char* status;

        switch (tox().get_status(m_friend_nr)) {
            case Toxmm::EUSERSTATUS::BUSY:
                status = "status_busy";
                break;
            case Toxmm::EUSERSTATUS::NONE:
                status = "status_online";
                break;
            case Toxmm::EUSERSTATUS::AWAY:
                status = "status_away";
                break;
            default:
                status = "status_offline";
                break;
        }

        m_status_icon->set_from_icon_name(status, Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR);
        m_status_icon->reset_style();
        m_status_icon->queue_resize();
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

int WidgetContactListItem::compare(WidgetContactListItem* other) {
    if (m_name < other->m_name) {
        return -1;
    } else if (m_name > other->m_name) {
        return 1;
    }
    return 0;
}

bool WidgetContactListItem::use_mini(gToxObserver* gchild, bool for_notify) {
    return gchild->tox().database().config_get(for_notify?"VISUAL_CONTACT_NOTIFY":"VISUAL_CONTACT", for_notify?1:0);
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

void WidgetContactListItem::notify(const Glib::ustring& title, const Glib::ustring& message) {
    //Notification
    if (!m_notify) {
        m_notify = std::make_shared<Notify::Notification>(title, message);
        m_notify->set_image_from_pixbuf(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg")
                                        ->scale_simple(
                                            64,
                                            64,
                                            Gdk::INTERP_BILINEAR));
        m_notify->signal_closed().connect([this](){
            //m_notify->get_closed_reason() ???
            observer_notify(ToxEvent(EventActivated{m_friend_nr}));
        });
        try {
            m_notify->show();
        } catch (...) {
            //ignore
        }
    } else {
        m_notify->update(title, message);
        try {
            m_notify->show();
        } catch (...) {
            //ignore
        }
    }
    //Sound:
    Canberra::play("message-new-instant");
}
