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

using namespace toxmm2;

contact::type_signal_receipt            contact::signal_receipt() { return m_signal_receipt; }
contact::type_signal_recv_message       contact::signal_recv_message() { return m_signal_recv_message; }
contact::type_signal_recv_action        contact::signal_recv_action() { return m_signal_recv_action; }
contact::type_signal_send_message       contact::signal_send_message() { return m_signal_send_message; }
contact::type_signal_send_action        contact::signal_send_action() { return m_signal_send_action; }
contact::type_signal_send_file_chunk_rq contact::signal_send_file_chunk_request() { return m_signal_send_file_chunk_rq; }
contact::type_signal_recv_file          contact::signal_recv_file() { return m_signal_recv_file; }
contact::type_signal_recv_file_chunk    contact::signal_recv_file_chunk() { return m_signal_recv_file_chunk; }
contact::type_signal_recv_file_control  contact::signal_recv_file_control() { return m_signal_recv_file_control; }

Glib::PropertyProxy_ReadOnly<contactNr>         contact::property_nr()
{ return Glib::PropertyProxy_ReadOnly<contactNr>(this, "contact-nr"); }
Glib::PropertyProxy_ReadOnly<contactAddrPublic> contact::property_addr_public()
{ return Glib::PropertyProxy_ReadOnly<contactAddrPublic>(this, "contact-addr"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>     contact::property_name()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "contact-name"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>     contact::property_name_or_addr()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "contact-name-or-addr"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>     contact::property_status_message()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "contact-status-message"); }
Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>   contact::property_status()
{ return Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>(this, "contact-status"); }
Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    contact::property_connection()
{ return Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>(this, "contact-connection"); }
Glib::PropertyProxy_ReadOnly<bool>              contact::property_typing()
{ return Glib::PropertyProxy_ReadOnly<bool>(this, "contact-typing"); }

contact::contact(std::shared_ptr<toxmm2::contact_manager> manager, contactNr nr):
    Glib::ObjectBase(typeid(contact)),
    m_contact_manager(manager),
    m_property_nr  (*this, "contact-nr"),
    m_property_addr(*this, "contact-addr"),
    m_property_name(*this, "contact-name"),
    m_property_name_or_addr(*this, "contact-name-or-addr"),
    m_property_status_message(*this, "contact-status-message"),
    m_property_status(*this, "contact-status"),
    m_property_connection(*this, "contact-connection"),
    m_property_typing(*this, "contact-typing") {

    auto update_name_or_addr = [this]() {
        if (property_name().get_value().empty()) {
            m_property_name_or_addr = Glib::ustring(m_property_addr.get_value());
        } else {
            m_property_name_or_addr = property_name().get_value();
        }
    };
    property_name().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));
    property_addr_public().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));

    m_property_nr = nr;
    m_property_addr = toxcore_get_addr();
    m_property_name = toxcore_get_name();
    m_property_status_message = toxcore_get_status_message();
    m_property_status = toxcore_get_status();
    m_property_connection = toxcore_get_connection();
}

void contact::init() {
    //start sub systems:
    m_file_manager = std::shared_ptr<toxmm2::file_manager>(new toxmm2::file_manager(shared_from_this()));
    m_file_manager->init();
}

contactAddrPublic contact::toxcore_get_addr() {
    contactAddrPublic addr;
    TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
    auto res = tox_friend_get_public_key(core()->toxcore(), m_property_nr.get_value(), addr, &error);
    if (error != TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK) {
        throw exception(error);
    }
    if (!res) {
        throw exception(TOX_ERR_FRIEND_GET_PUBLIC_KEY(~0));
    }
    return addr;
}

Glib::ustring contact::toxcore_get_name() {
    TOX_ERR_FRIEND_QUERY error;
    auto size = tox_friend_get_name_size(core()->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (size == SIZE_MAX) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    std::string name(size, 0);
    auto res = tox_friend_get_name(core()->toxcore(), m_property_nr.get_value(), (uint8_t*)name.data(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (!res) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

Glib::ustring contact::toxcore_get_status_message() {
    auto size = tox_self_get_status_message_size(core()->toxcore());
    std::string name(size, 0);
    tox_self_get_status_message(core()->toxcore(), (unsigned char*)name.data());
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

TOX_USER_STATUS contact::toxcore_get_status() {
    TOX_ERR_FRIEND_QUERY error;
    auto status = tox_friend_get_status(core()->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return status;
}

TOX_CONNECTION contact::toxcore_get_connection() {
    TOX_ERR_FRIEND_QUERY error;
    auto con = tox_friend_get_connection_status(core()->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return con;
}

std::shared_ptr<receipt> contact::send_message(const Glib::ustring& message) {
    TOX_ERR_FRIEND_SEND_MESSAGE error;
    auto receipt = tox_friend_send_message(core()->toxcore(), property_nr().get_value(), TOX_MESSAGE_TYPE_NORMAL, (const uint8_t*)message.data(), message.size(), &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw exception(error);
    }
    auto r = std::shared_ptr<toxmm2::receipt>(new toxmm2::receipt(shared_from_this(), receipt));
    m_signal_send_message(message, r);
    return r;
}

std::shared_ptr<receipt> contact::send_action (const Glib::ustring& action) {
    TOX_ERR_FRIEND_SEND_MESSAGE error;
    auto receipt = tox_friend_send_message(core()->toxcore(), property_nr().get_value(), TOX_MESSAGE_TYPE_ACTION, (const uint8_t*)action.data(), action.size(), &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw exception(error);
    }
    auto r = std::shared_ptr<toxmm2::receipt>(new toxmm2::receipt(shared_from_this(), receipt));
    m_signal_send_action(action, r);
    return r;
}

std::shared_ptr<toxmm2::core> contact::core() {
    return contact_manager()->core();
}

std::shared_ptr<toxmm2::contact_manager> contact::contact_manager() {
    return m_contact_manager;
}

std::shared_ptr<toxmm2::file_manager> contact::file_manager() {
    return m_file_manager;
}
