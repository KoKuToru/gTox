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
#include "contact.h"
#include "tox/contact/contact.h"
#include "dialog/chat.h"
#include "dialog/main.h"

using namespace widget;

utils::builder::ref<contact> contact::create(Glib::RefPtr<dialog::main> main, std::shared_ptr<toxmm2::contact> contact, bool for_notify) {
    return utils::builder(Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_contact.ui"))
            .get_widget_derived<widget::contact>("contact_list_item",
                                                 main,
                                                 contact,
                                                 for_notify);
}


contact::contact(BaseObjectType* cobject,
                 utils::builder builder,
                 Glib::RefPtr<dialog::main> main,
                 std::shared_ptr<toxmm2::contact> contact,
                 bool for_active_chats)
    : Gtk::ListBoxRow(cobject),
      m_main(main),
      m_contact(contact),
      m_for_active_chats(for_active_chats) {

    builder.get_widget("contact_list_grid_mini", m_contact_list_grid_mini);
    builder.get_widget("contact_list_grid", m_contact_list_grid);
    builder.get_widget("revealer", m_revealer);

    //builder.get_widget("avatar", m_avatar);
    m_avatar = builder.get_widget_derived<widget::avatar>("avatar", contact->property_addr_public());
    builder.get_widget("name", m_name);
    builder.get_widget("status", m_status_msg);
    builder.get_widget("status_icon", m_status_icon);
    builder.get_widget("spinner", m_spin);

    m_avatar_mini = builder.get_widget_derived<widget::avatar>("avatar_mini", contact->property_addr_public());
    builder.get_widget("name_mini", m_name_mini);
    builder.get_widget("status_mini", m_status_msg_mini);
    builder.get_widget("status_icon_mini", m_status_icon_mini);
    builder.get_widget("spinner_mini", m_spin_mini);

    m_bindings[0] = Glib::Binding::bind_property(m_status_icon->property_icon_name(),
                                                 m_status_icon_mini->property_icon_name(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[1] = Glib::Binding::bind_property(m_status_icon->property_icon_size(),
                                                 m_status_icon_mini->property_icon_size(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[2] = Glib::Binding::bind_property(m_spin->property_active(),
                                                 m_spin_mini->property_active(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_BIDIRECTIONAL |
                                                 Glib::BINDING_SYNC_CREATE);
    m_spin->stop();

    m_bindings[3] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                 m_name->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[4] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                 m_name_mini->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[5] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                 m_status_msg->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_SYNC_CREATE);
    m_bindings[6] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                 m_status_msg_mini->property_label(),
                                                 Glib::BINDING_DEFAULT |
                                                 Glib::BINDING_SYNC_CREATE);

    m_status_icon->set_from_icon_name("status_offline", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);

    m_contact->property_status().signal_changed().connect(sigc::track_obj([this]() {
        switch (m_contact->property_status().get_value()) {
            case TOX_USER_STATUS_AWAY:
                m_status_icon->set_from_icon_name("status_away", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
            case TOX_USER_STATUS_BUSY:
                m_status_icon->set_from_icon_name("status_busy", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
            case TOX_USER_STATUS_NONE:
                m_status_icon->set_from_icon_name("status_online", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
        }
        m_status_icon->reset_style();
        m_status_icon->queue_resize();
    }, *this));

    m_contact->property_connection().signal_changed().connect(sigc::track_obj([this]() {
        if (m_contact->property_connection().get_value() == TOX_CONNECTION_NONE) {
            m_status_icon->set_from_icon_name("status_offline", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
            m_status_icon->reset_style();
            m_status_icon->queue_resize();
        }
    }, *this));

    auto display_spinner = [this]() {
        m_spin->start();
    };

    m_contact->signal_new_message().connect(sigc::hide(sigc::track_obj(display_spinner, *this)));
    m_contact->signal_new_action().connect(sigc::hide(sigc::track_obj(display_spinner, *this)));
    m_contact->signal_new_file().connect(sigc::track_obj(display_spinner, *this));

    if (!for_active_chats) {
        m_contact_list_grid_mini->hide();
        m_contact_list_grid->show();
        show();
    } else {
        m_contact_list_grid->hide();
        m_contact_list_grid_mini->show();
    }
}

/*
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
}*/


contact::~contact() {
}

int contact::compare(contact* other) {
    auto name_a = this ->m_contact->property_name_or_addr().get_value().lowercase();
    auto name_b = other->m_contact->property_name_or_addr().get_value().lowercase();
    if (name_a < name_b) {
        return -1;
    } else if (name_a > name_b) {
        return 1;
    }
    return 0;
}

void contact::on_show() {
    Gtk::Widget::on_show();
    m_dispatcher.emit([this](){
        m_revealer->set_reveal_child(true);
    });
}

void contact::on_hide() {
    if (m_revealer->get_reveal_child()) {
        m_revealer->set_reveal_child(false);
        utils::dispatcher::ref dispatcher = m_dispatcher;
        Glib::signal_timeout().connect_once([this, dispatcher]() {
            dispatcher.emit([this]() {
                if (!m_revealer->get_reveal_child()) {
                    Gtk::Widget::on_hide();
                }
            });
        }, m_revealer->get_transition_duration());
    } else {
        Gtk::Widget::on_hide();
    }
}

std::shared_ptr<toxmm2::contact> contact::get_contact() {
    return m_contact;
}

void contact::activated() {
    if (m_chat) {
        m_chat->activated();
    } else {
        m_chat = Glib::RefPtr<dialog::chat>(new dialog::chat(m_main, m_contact));
    }
}
