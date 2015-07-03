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
    class contact : public Glib::Object, public std::enable_shared_from_this<contact> {
            friend class contact_manager;
        public:
            //signals
            typedef sigc::signal<void, receiptNr>     type_signal_receipt;
            typedef sigc::signal<void, Glib::ustring> type_signal_new_message;
            typedef sigc::signal<void, Glib::ustring> type_signal_new_action;
            typedef sigc::signal<void>                type_signal_new_file;

            type_signal_receipt     signal_receipt();
            type_signal_new_message signal_new_message();
            type_signal_new_action  signal_new_action();
            type_signal_new_file    signal_new_file();

            //props
            Glib::PropertyProxy_ReadOnly<contactNr>         property_nr();
            Glib::PropertyProxy_ReadOnly<contactPublicAddr> property_addr();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_status_message();
            Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>   property_status();
            Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    property_connection();
            Glib::PropertyProxy_ReadOnly<bool>              property_typing();

        private:
            std::shared_ptr<core> m_core;

            Glib::Property<contactNr>         m_property_nr;
            Glib::Property<contactPublicAddr> m_property_addr;
            Glib::Property<Glib::ustring>     m_property_name;
            Glib::Property<Glib::ustring>     m_property_status_message;
            Glib::Property<TOX_USER_STATUS>   m_property_status;
            Glib::Property<TOX_CONNECTION>    m_property_connection;
            Glib::Property<bool>              m_property_typing;

            type_signal_receipt     m_signal_receipt;
            type_signal_new_message m_signal_new_message;
            type_signal_new_action  m_signal_new_action;
            type_signal_new_file    m_signal_new_file;

            contact(std::shared_ptr<core> core, contactNr nr);
            contact(const contact&) = delete;
            void operator=(const contact&) = delete;

            void init();

            contactPublicAddr toxcore_get_addr();
            Glib::ustring     toxcore_get_name();
            Glib::ustring     toxcore_get_status_message();
            TOX_USER_STATUS   toxcore_get_status();
            TOX_CONNECTION    toxcore_get_connection();
    };

}

#endif
