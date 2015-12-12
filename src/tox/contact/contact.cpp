/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#include "manager.h"
#include "core.h"
#include "exception.h"
#include "receipt.h"
#include "file/manager.h"
#include "file/file.h"
#include "call.h"

using namespace toxmm;

contact::contact(std::shared_ptr<toxmm::contact_manager> manager, contactNr nr):
    Glib::ObjectBase(typeid(contact)),
    m_contact_manager(manager) {

    auto update_name_or_addr = [this]() {
        if (property_name().get_value().empty()) {
            m_property_name_or_addr = Glib::ustring(m_property_addr_public.get_value());
        } else {
            m_property_name_or_addr = property_name().get_value();
        }
    };
    property_name().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));
    property_addr_public().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));

    m_property_nr = nr;
    m_property_addr_public = toxcore_get_addr();
    m_property_name = toxcore_get_name();
    m_property_status_message = toxcore_get_status_message();
    m_property_status = toxcore_get_status();
    m_property_connection = toxcore_get_connection();
    m_property_typing = false;
    m_property_remote_typing = false;
}

void contact::init() {
    //start sub systems:
    m_file_manager = std::shared_ptr<toxmm::file_manager>(new toxmm::file_manager(shared_from_this()));
    m_file_manager->init();

    property_connection().signal_changed().connect(sigc::track_obj([this]() {
        auto c = core();
        if (!c) {
            return;
        }

        if (m_avatar_send) {
            //handle disconnect
            if (property_connection() == TOX_CONNECTION_NONE) {
                m_avatar_send->property_state() = TOX_FILE_CONTROL_CANCEL;
                m_avatar_send.reset();
            }
        } else if (property_connection() != TOX_CONNECTION_NONE) {
            auto path = Glib::build_filename(
                            c->config()->property_avatar_path().get_value(),
                            std::string(c->property_addr_public()
                                        .get_value()) + ".png");
            if (Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR)) {
                m_avatar_send = file_manager()->send_file(path, true);
            }
            m_avatar_send_monitor = Gio::File::create_for_path(path)
                                    ->monitor_file();
            m_avatar_send_monitor->signal_changed()
                    .connect(sigc::track_obj(
                                 [this, path](
                                      const Glib::RefPtr<Gio::File>&,
                                      const Glib::RefPtr<Gio::File>&,
                                      Gio::FileMonitorEvent event_type) {
                switch (event_type) {
                    case Gio::FILE_MONITOR_EVENT_CREATED:
                    case Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                    case Gio::FILE_MONITOR_EVENT_DELETED:
                        //resend:
                        if (m_avatar_send) {
                            m_avatar_send->property_state() = TOX_FILE_CONTROL_CANCEL;
                            m_avatar_send.reset();
                        }
                        if (event_type == Gio::FILE_MONITOR_EVENT_DELETED) {
                            m_avatar_send = file_manager()->send_file("", true);
                        } else {
                            m_avatar_send = file_manager()->send_file(path, true);
                        }
                        break;
                    default:
                        //ignore
                        break;
                }
            }, *this));
        }
    }, *this));

    m_call = std::shared_ptr<toxmm::call>(new toxmm::call(shared_from_this()));
}

contactAddrPublic contact::toxcore_get_addr() {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    contactAddrPublic addr;
    TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
    auto res = tox_friend_get_public_key(c->toxcore(), m_property_nr.get_value(), addr, &error);
    if (error != TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK) {
        throw exception(error);
    }
    if (!res) {
        throw exception(TOX_ERR_FRIEND_GET_PUBLIC_KEY(~0));
    }
    return addr;
}

Glib::ustring contact::toxcore_get_name() {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_QUERY error;
    auto size = tox_friend_get_name_size(c->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (size == SIZE_MAX) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    std::string name(size, 0);
    auto res = tox_friend_get_name(c->toxcore(), m_property_nr.get_value(), (uint8_t*)name.data(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (!res) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

Glib::ustring contact::toxcore_get_status_message() {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_QUERY error;
    auto size = tox_friend_get_status_message_size(c->toxcore(),
                                                   m_property_nr.get_value(),
                                                   &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    std::string name(size, 0);
    tox_friend_get_status_message(c->toxcore(),
                                  m_property_nr.get_value(),
                                  (unsigned char*)name.data(),
                                  &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

TOX_USER_STATUS contact::toxcore_get_status() {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_QUERY error;
    auto status = tox_friend_get_status(c->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return status;
}

TOX_CONNECTION contact::toxcore_get_connection() {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_QUERY error;
    auto con = tox_friend_get_connection_status(c->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return con;
}

std::shared_ptr<receipt> contact::send_message(const std::string& message) {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    auto receipt = tox_friend_send_message(c->toxcore(),
                                           property_nr().get_value(),
                                           TOX_MESSAGE_TYPE_NORMAL,
                                           (const uint8_t*)message.data(),
                                           message.size(),
                                           &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw exception(error);
    }
    auto r = std::shared_ptr<toxmm::receipt>(new toxmm::receipt(shared_from_this(), receipt));
    m_signal_send_message(message, r);
    return r;
}

std::shared_ptr<receipt> contact::send_action (const std::string& action) {
    auto c = core();
    if (!c) {
        throw std::runtime_error("core() is nullptr");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    auto receipt = tox_friend_send_message(c->toxcore(),
                                           property_nr().get_value(),
                                           TOX_MESSAGE_TYPE_ACTION,
                                           (const uint8_t*)action.data(),
                                           action.size(),
                                           &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw exception(error);
    }
    auto r = std::shared_ptr<toxmm::receipt>(new toxmm::receipt(shared_from_this(), receipt));
    m_signal_send_action(action, r);
    return r;
}

std::shared_ptr<toxmm::core> contact::core() {
    auto m = contact_manager();
    return m ? m->core() : nullptr;
}

std::shared_ptr<toxmm::contact_manager> contact::contact_manager() {
    return m_contact_manager.lock();
}

std::shared_ptr<toxmm::file_manager> contact::file_manager() {
    return m_file_manager;
}

std::shared_ptr<toxmm::call> contact::call() {
    return m_call;
}
