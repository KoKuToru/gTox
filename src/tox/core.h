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
#ifndef TOXMM_CORE_H
#define TOXMM_CORE_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "profile.h"
#include "types.h"

namespace toxmm2 {
    class core : public Glib::Object, public std::enable_shared_from_this<core> {
        public:
            static std::shared_ptr<core> create(std::string path);
            void save();

            void update();
            uint32_t update_optimal_interval();

            static Glib::ustring fix_utf8(const std::string& input);
            static Glib::ustring fix_utf8(const uint8_t* input, int size);
            static Glib::ustring fix_utf8(const int8_t* input, int size);
            static Glib::ustring to_hex(const uint8_t* data, size_t len);
            static std::vector<uint8_t> from_hex(std::string data);

            void destroy();
            ~core();

            Tox* toxcore();
            std::shared_ptr<toxmm2::contact_manager> contact_manager();

            //props
            Glib::PropertyProxy_ReadOnly<contactAddr>       property_addr();
            Glib::PropertyProxy_ReadOnly<contactAddrPublic> property_addr_public();
            Glib::PropertyProxy<Glib::ustring>              property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name_or_addr();
            Glib::PropertyProxy<Glib::ustring>              property_status_message();
            Glib::PropertyProxy<TOX_USER_STATUS>            property_status();
            Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    property_connection();

            //Signals
            typedef sigc::signal<void, contactAddr, Glib::ustring> type_signal_contact_request;
            typedef sigc::signal<void, contactNr, Glib::ustring>   type_signal_contact_message;
            typedef sigc::signal<void, contactNr, Glib::ustring>   type_signal_contact_action;
            typedef sigc::signal<void, contactNr, Glib::ustring>   type_signal_contact_name;
            typedef sigc::signal<void, contactNr, Glib::ustring>   type_signal_contact_status_message;
            typedef sigc::signal<void, contactNr, TOX_USER_STATUS> type_signal_contact_status;
            typedef sigc::signal<void, contactNr, bool>            type_signal_contact_typing;
            typedef sigc::signal<void, contactNr, receiptNr>       type_signal_contact_read_receipt;
            typedef sigc::signal<void, contactNr, TOX_CONNECTION>  type_signal_contact_connection_status;

            type_signal_contact_request           signal_contact_request();
            type_signal_contact_message           signal_contact_message();
            type_signal_contact_action            signal_contact_action();
            type_signal_contact_name              signal_contact_name();
            type_signal_contact_status_message    signal_contact_status_message();
            type_signal_contact_status            signal_contact_status();
            type_signal_contact_typing            signal_contact_typing();
            type_signal_contact_read_receipt      signal_contact_read_receipt();
            type_signal_contact_connection_status signal_contact_connection_status();

            static void try_load(std::string path, Glib::ustring& out_name, Glib::ustring& out_status, contactAddrPublic& out_addr, bool& out_writeable);
            static std::vector<uint8_t> create_state(std::string name, std::string status, contactAddrPublic& out_addr);

        private:
            Tox* m_toxcore;
            std::string m_path;
            profile m_profile;
            std::shared_ptr<toxmm2::contact_manager> m_contact_manager;

            core(std::string path);
            core(const core&) = delete;
            void operator=(const core&) = delete;

            void init();

            Glib::Property<contactAddr>       m_property_addr;
            Glib::Property<contactAddrPublic> m_property_addr_public;
            Glib::Property<Glib::ustring>     m_property_name;
            Glib::Property<Glib::ustring>     m_property_name_or_addr;
            Glib::Property<Glib::ustring>     m_property_status_message;
            Glib::Property<TOX_USER_STATUS>   m_property_status;
            Glib::Property<TOX_CONNECTION>    m_property_connection;

            type_signal_contact_request           m_signal_contact_request;
            type_signal_contact_message           m_signal_contact_message;
            type_signal_contact_action            m_signal_contact_action;
            type_signal_contact_name              m_signal_contact_name;
            type_signal_contact_status_message    m_signal_contact_status_message;
            type_signal_contact_status            m_signal_contact_status;
            type_signal_contact_typing            m_signal_contact_typing;
            type_signal_contact_read_receipt      m_signal_contact_read_receipt;
            type_signal_contact_connection_status m_signal_contact_connection_status;
    };

}

#endif
