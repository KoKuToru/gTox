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

using namespace toxmm2;

contact::type_signal_receipt     contact::signal_receipt()     { return m_signal_receipt; }
contact::type_signal_new_message contact::signal_new_message() { return m_signal_new_message; }
contact::type_signal_new_action  contact::signal_new_action()  { return m_signal_new_action; }
contact::type_signal_new_file    contact::signal_new_file()    { return m_signal_new_file; }

Glib::PropertyProxy_ReadOnly<contactNr>         contact::property_nr()
{ return Glib::PropertyProxy_ReadOnly<contactNr>(this, "contact_nr"); }
Glib::PropertyProxy_ReadOnly<contactPublicAddr> contact::property_addr()
{ return Glib::PropertyProxy_ReadOnly<contactPublicAddr>(this, "contact_addr"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>     contact::property_name()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "contact_name"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>     contact::property_status_message()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "contact_status_message"); }
Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>   contact::property_status()
{ return Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>(this, "contact_status"); }
Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    contact::property_connection()
{ return Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>(this, "contact_connection"); }
Glib::PropertyProxy_ReadOnly<bool>              contact::property_typing()
{ return Glib::PropertyProxy_ReadOnly<bool>(this, "contact_typing"); }

contact::contact(std::shared_ptr<core> core, contactNr nr):
    m_core(core),
    m_property_nr  (*this, "contact_nr"),
    m_property_addr(*this, "contact_addr"),
    m_property_name(*this, "contact_name"),
    m_property_status_message(*this, "contact_status_message"),
    m_property_status(*this, "contact_status"),
    m_property_connection(*this, "contact_connection"),
    m_property_typing(*this, "contact_typing")
{
    m_property_nr = nr;
    m_property_addr = toxcore_get_addr();
    m_property_name = toxcore_get_name();
    m_property_status_message = toxcore_get_status_message();
    m_property_status = toxcore_get_status();
    m_property_connection = toxcore_get_connection();
}

void contact::init() {

}

contactPublicAddr contact::toxcore_get_addr() {
    contactPublicAddr addr;
    TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
    auto res = tox_friend_get_public_key(m_core->toxcore(), m_property_nr.get_value(), addr, &error);
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
    auto size = tox_friend_get_name_size(m_core->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (size == SIZE_MAX) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    std::string name(size, 0);
    auto res = tox_friend_get_name(m_core->toxcore(), m_property_nr.get_value(), (uint8_t*)name.data(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    if (!res) {
        throw exception(TOX_ERR_FRIEND_QUERY(~0));
    }
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

Glib::ustring contact::toxcore_get_status_message() {
    auto size = tox_self_get_status_message_size(m_core->toxcore());
    std::string name(size, 0);
    tox_self_get_status_message(m_core->toxcore(), (unsigned char*)name.data());
    return core::fix_utf8((uint8_t*)name.data(), name.size());
}

TOX_USER_STATUS contact::toxcore_get_status() {
    TOX_ERR_FRIEND_QUERY error;
    auto status = tox_friend_get_status(m_core->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return status;
}

TOX_CONNECTION contact::toxcore_get_connection() {
    TOX_ERR_FRIEND_QUERY error;
    auto con = tox_friend_get_connection_status(m_core->toxcore(), m_property_nr.get_value(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw exception(error);
    }
    return con;
}
