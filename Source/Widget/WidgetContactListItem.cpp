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
#include "gTox.h"
#include <glibmm/i18n.h>

gToxBuilderRef<WidgetContactListItem> WidgetContactListItem::create(gToxObservable* instance, Toxmm::FriendNr nr, bool for_notify) {
    gToxObserver dummy(instance);
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_contact.ui");
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
        if (!m_spin->property_active() && !m_chat) {
            m_spin->start();
        }
        if (!m_for_notify && !m_chat) {
            notify(tox().get_name_or_address(m_friend_nr),
                   (ev.type() == typeid(Toxmm::EventFriendMessage)) ?
                       ev.get<Toxmm::EventFriendMessage>().message :
                       ev.get<Toxmm::EventFriendAction>().message);
        }
    } else if (ev.type() == typeid(Toxmm::EventFileRecv)) {
        auto data = ev.get<Toxmm::EventFileRecv>();
        if (data.nr != m_friend_nr || data.kind != TOX_FILE_KIND_DATA) {
            return;
        }
        if (!m_spin->property_active() && !m_chat) {
            m_spin->start();
        }
        if (!m_for_notify && !m_chat) {
            notify(tox().get_name_or_address(m_friend_nr),
                   Glib::ustring::compose(_("FRIEND_NEW_FILE_RECV"),
                                          data.filename,
                                          data.file_size));
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
    } else if (ev.type() == typeid(EventUpdateCompact)) {
        auto data = ev.get<EventUpdateCompact>();
        bool compact = data.compact;

        if (compact || m_for_notify) {
            m_contact_list_grid_mini->show();
            m_contact_list_grid->hide();
        } else {
            m_contact_list_grid_mini->hide();
            m_contact_list_grid->show();
        }
    } else if (ev.type() == typeid(EventUpdateDisplayActive)) {
        auto data = ev.get<EventUpdateDisplayActive>();
        m_display_active = data.display;

        m_spin->set_visible(m_display_active);
        m_spin_mini->set_visible(m_display_active);
    }
}

WidgetContactListItem::WidgetContactListItem(BaseObjectType* cobject, gToxBuilder builder,
                                             gToxObservable* observable,
                                             Toxmm::FriendNr nr,
                                             bool for_notify)
    : Gtk::ListBoxRow(cobject),
      gToxObserver(observable),
      m_friend_nr(nr),
      m_for_notify(for_notify) {

    builder.get_widget("contact_list_grid_mini", m_contact_list_grid_mini);
    builder.get_widget("contact_list_grid", m_contact_list_grid);
    builder.get_widget("revealer", m_revealer);

    //builder.get_widget("avatar", m_avatar);
    m_avatar = builder.get_widget_derived<WidgetAvatar>("avatar", observable, nr);
    builder.get_widget("name", m_name);
    builder.get_widget("status", m_status_msg);
    builder.get_widget("status_icon", m_status_icon);
    builder.get_widget("spinner", m_spin);

    m_avatar_mini = builder.get_widget_derived<WidgetAvatar>("avatar_mini", observable, nr);
    builder.get_widget("name_mini", m_name_mini);
    builder.get_widget("status_mini", m_status_msg_mini);
    builder.get_widget("status_icon_mini", m_status_icon_mini);
    builder.get_widget("spinner_mini", m_spin_mini);

    m_bindings[0] = Glib::Binding::bind_property(m_name->property_label(),
                                                 m_name_mini->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[1] = Glib::Binding::bind_property(m_status_msg->property_label(),
                                                 m_status_msg_mini->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[2] = Glib::Binding::bind_property(m_status_icon->property_icon_name(),
                                                 m_status_icon_mini->property_icon_name(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[3] = Glib::Binding::bind_property(m_status_icon->property_icon_size(),
                                                 m_status_icon_mini->property_icon_size(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[4] = Glib::Binding::bind_property(m_spin->property_active(),
                                                 m_spin_mini->property_active(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_spin->stop();

    refresh();

    bool compact = gTox::instance()->database().config_get("SETTINGS_CONTACTLIST_USE_COMPACT", false);
    observer_handle(ToxEvent(EventUpdateCompact{compact}));

    bool display = gTox::instance()->database().config_get("SETTINGS_CONTACTLIST_DISPLAY_ACTIVE", true);
    observer_handle(ToxEvent(EventUpdateDisplayActive{display}));

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
    m_revealer->reference();
    Glib::signal_idle().connect_once([this](){
        m_revealer->set_reveal_child(true);
        m_revealer->unreference();
    });
}

void WidgetContactListItem::on_hide() {
    if (m_revealer->get_reveal_child()) {
        m_revealer->set_reveal_child(false);
        m_revealer->reference();
        reference();
        Glib::signal_timeout().connect_once([this](){
            if (!m_revealer->get_reveal_child()) {
                hide();
                unreference();
            }
            m_revealer->unreference();
        }, m_revealer->get_transition_duration());
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
            //auto res = m_notify->get_closed_reason();
            //Broken in GNOME 3.16
            //gives reason 2 onclick and on window activation
            //sucks
            //observer_notify(ToxEvent(EventActivated{m_friend_nr}));
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
