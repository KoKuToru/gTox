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
#ifndef TOXMM_CONTACT_H
#define TOXMM_CONTACT_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"

namespace toxmm2 {
    class receipt;
    class contact : public Glib::Object, public std::enable_shared_from_this<contact> {
            friend class contact_manager;
        public:
            //signals
            typedef sigc::signal<void, receiptNr>     type_signal_receipt;
            typedef sigc::signal<void, Glib::ustring> type_signal_recv_message;
            typedef sigc::signal<void, Glib::ustring> type_signal_recv_action;
            typedef sigc::signal<void>                type_signal_recv_file;
            typedef sigc::signal<void, Glib::ustring, std::shared_ptr<receipt>> type_signal_send_message;
            typedef sigc::signal<void, Glib::ustring, std::shared_ptr<receipt>> type_signal_send_action;

            type_signal_receipt      signal_receipt();
            type_signal_recv_message signal_recv_message();
            type_signal_recv_action  signal_recv_action();
            type_signal_recv_file    signal_recv_file();
            type_signal_send_message signal_send_message();
            type_signal_send_action  signal_send_action();

            //props
            Glib::PropertyProxy_ReadOnly<contactNr>         property_nr();
            Glib::PropertyProxy_ReadOnly<contactAddrPublic> property_addr_public();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name_or_addr();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_status_message();
            Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>   property_status();
            Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    property_connection();
            Glib::PropertyProxy_ReadOnly<bool>              property_typing();

            //functions
            std::shared_ptr<receipt> send_message(const Glib::ustring& message);
            std::shared_ptr<receipt> send_action (const Glib::ustring& action);

        private:
            std::shared_ptr<core> m_core;

            Glib::Property<contactNr>         m_property_nr;
            Glib::Property<contactAddrPublic> m_property_addr;
            Glib::Property<Glib::ustring>     m_property_name;
            Glib::Property<Glib::ustring>     m_property_name_or_addr;
            Glib::Property<Glib::ustring>     m_property_status_message;
            Glib::Property<TOX_USER_STATUS>   m_property_status;
            Glib::Property<TOX_CONNECTION>    m_property_connection;
            Glib::Property<bool>              m_property_typing;

            type_signal_receipt      m_signal_receipt;
            type_signal_recv_message m_signal_recv_message;
            type_signal_recv_action  m_signal_recv_action;
            type_signal_recv_file    m_signal_recv_file;
            type_signal_send_message m_signal_send_message;
            type_signal_send_action  m_signal_send_action;

            contact(std::shared_ptr<core> core, contactNr nr);
            contact(const contact&) = delete;
            void operator=(const contact&) = delete;

            contactAddrPublic toxcore_get_addr();
            Glib::ustring     toxcore_get_name();
            Glib::ustring     toxcore_get_status_message();
            TOX_USER_STATUS   toxcore_get_status();
            TOX_CONNECTION    toxcore_get_connection();
    };

}

#endif
