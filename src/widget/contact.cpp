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
#include "tox/contact/file/file.h"
#include "tox/contact/file/manager.h"
#include <flatbuffers/flatbuffers.h>
#include "dialog/detachable_window.h"

using namespace widget;

utils::builder::ref<contact> contact::create(dialog::main& main, std::shared_ptr<toxmm::contact> contact, bool for_notify) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { contact->property_name_or_addr().get_value().raw() });
    return utils::builder::create_ref<widget::contact>(
                "/org/gtox/ui/list_item_contact.ui",
                "contact_list_item",
                main,
                contact,
                for_notify);
}


contact::contact(BaseObjectType* cobject,
                 utils::builder builder,
                 dialog::main& main,
                 std::shared_ptr<toxmm::contact> contact,
                 bool for_active_chats)
    : Gtk::ListBoxRow(cobject),
      m_main(main),
      m_contact(contact),
      m_for_active_chats(for_active_chats) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { contact->property_name_or_addr().get_value().raw() });
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
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
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
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_contact->property_connection().get_value() == TOX_CONNECTION_NONE) {
            m_status_icon->set_from_icon_name("status_offline", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
            m_status_icon->reset_style();
            m_status_icon->queue_resize();
        }
    }, *this));

    auto display_spinner = [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_spin->start();
    };

    m_contact->signal_recv_message().connect(sigc::hide(sigc::track_obj(display_spinner, *this)));
    m_contact->signal_recv_action().connect(sigc::hide(sigc::track_obj(display_spinner, *this)));

    auto update_visibility = [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (!m_for_active_chats &&
                !m_main.config()->property_contacts_compact_list().get_value()) {
            m_contact_list_grid_mini->hide();
            m_contact_list_grid->show();
        } else {
            m_contact_list_grid->hide();
            m_contact_list_grid_mini->show();
        }
        if (!m_for_active_chats) {
            show();
        }
    };
    update_visibility();
    m_main.config()->property_contacts_compact_list()
            .signal_changed().connect(sigc::track_obj(update_visibility, *this));


    if (m_for_active_chats) {
        return;
    }

    //callbacks for logging
    m_contact->signal_send_message().connect(sigc::track_obj([this](Glib::ustring message, std::shared_ptr<toxmm::receipt>) {
       utils::debug::scope_log log(DBG_LVL_2("gtox"), { message.raw() });
       dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
           return flatbuffers::Log::CreateItem(fbb,
                                               fbb.CreateString(m_main.tox()->property_addr_public().get_value()),
                                               Glib::DateTime::create_now_utc().to_unix(),
                                               flatbuffers::Log::Data_Message,
                                               flatbuffers::Log::CreateMessage(
                                                    fbb,
                                                    fbb.CreateString(message))
                                                     .Union());
       });
    }, *this));
    m_contact->signal_recv_message().connect(sigc::track_obj([this](Glib::ustring message) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { message.raw() });
        dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            return flatbuffers::Log::CreateItem(fbb,
                                                fbb.CreateString(m_contact->property_addr_public().get_value()),
                                                Glib::DateTime::create_now_utc().to_unix(),
                                                flatbuffers::Log::Data_Message,
                                                flatbuffers::Log::CreateMessage(
                                                     fbb,
                                                     fbb.CreateString(message))
                                                     .Union());
        });
    }, *this));
    m_contact->signal_send_action().connect(sigc::track_obj([this](Glib::ustring action, std::shared_ptr<toxmm::receipt>) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { action.raw() });
        dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            return flatbuffers::Log::CreateItem(fbb,
                                                fbb.CreateString(m_main.tox()->property_addr_public().get_value()),
                                                Glib::DateTime::create_now_utc().to_unix(),
                                                flatbuffers::Log::Data_Action,
                                                flatbuffers::Log::CreateAction(
                                                     fbb,
                                                     fbb.CreateString(action))
                                                      .Union());
        });
    }, *this));
    m_contact->signal_recv_action().connect(sigc::track_obj([this](Glib::ustring action) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { action.raw() });
        dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            return flatbuffers::Log::CreateItem(fbb,
                                                fbb.CreateString(m_contact->property_addr_public().get_value()),
                                                Glib::DateTime::create_now_utc().to_unix(),
                                                flatbuffers::Log::Data_Action,
                                                flatbuffers::Log::CreateAction(
                                                     fbb,
                                                     fbb.CreateString(action))
                                                     .Union());
        });
    },*this));
    auto fm = m_contact->file_manager();
    if (fm) {
        fm->signal_send_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm::file>& file) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            if (file->property_kind() != TOX_FILE_KIND_DATA) {
                return;
            }
            dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
                utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                return flatbuffers::Log::CreateItem(fbb,
                                                    fbb.CreateString(m_main.tox()->property_addr_public().get_value()),
                                                    Glib::DateTime::create_now_utc().to_unix(),
                                                    flatbuffers::Log::Data_File,
                                                    flatbuffers::Log::CreateFile(
                                                         fbb,
                                                         fbb.CreateString(file->property_uuid().get_value()),
                                                         fbb.CreateString(file->property_name().get_value()),
                                                         fbb.CreateString(file->property_path().get_value()),
                                                         flatbuffers::Log::FileStatus_PENDING,
                                                         fbb.CreateString(m_contact->property_addr_public().get_value()))
                                                          .Union());
            });
        }, *this));
        fm->signal_recv_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm::file>& file) {
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            if (file->property_kind() != TOX_FILE_KIND_DATA) {
                return;
            }
            dialog::chat::add_log(m_main.tox()->storage(), m_contact, [&](flatbuffers::FlatBufferBuilder& fbb) {
                utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                return flatbuffers::Log::CreateItem(fbb,
                                                    fbb.CreateString(m_contact->property_addr_public().get_value()),
                                                    Glib::DateTime::create_now_utc().to_unix(),
                                                    flatbuffers::Log::Data_File,
                                                    flatbuffers::Log::CreateFile(
                                                         fbb,
                                                         fbb.CreateString(file->property_uuid().get_value()),
                                                         fbb.CreateString(file->property_name().get_value()),
                                                         fbb.CreateString(file->property_path().get_value()),
                                                         flatbuffers::Log::FileStatus_PENDING,
                                                         fbb.CreateString(m_contact->property_addr_public().get_value()))
                                                          .Union());
            });
        }, *this));
    }
}

contact::~contact() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

int contact::compare(contact* other) {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
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
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    Gtk::Widget::on_show();
    m_dispatcher.emit([this](){
        utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
        m_revealer->set_reveal_child(true);
    });
}

void contact::on_hide() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (m_revealer->get_reveal_child()) {
        m_revealer->set_reveal_child(false);
        utils::dispatcher::ref dispatcher = m_dispatcher;
        Glib::signal_timeout().connect_once([this, dispatcher]() {
            utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
            dispatcher.emit([this]() {
                utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
                if (!m_revealer->get_reveal_child()) {
                    Gtk::Widget::on_hide();
                }
            });
        }, m_revealer->get_transition_duration());
    } else {
        Gtk::Widget::on_hide();
    }
}

std::shared_ptr<toxmm::contact> contact::get_contact() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_contact;
}

void contact::activated() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (m_chat) {
        //m_chat->activated();
    } else {
        auto attach = sigc::mem_fun(m_main,
                                    &dialog::main::detachable_window_add);
        auto detach = sigc::mem_fun(m_main,
                                    &dialog::main::detachable_window_del);
        m_chat = std::make_shared<dialog::chat>(m_main.tox(),
                                                m_contact,
                                                attach,
                                                detach);
        m_chat->signal_close().connect(sigc::track_obj([this]() {
            m_chat.reset();
        }, *this));
    }
}
