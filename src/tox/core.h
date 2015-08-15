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
#include "storage.h"

namespace toxmm {
    class core : public Glib::Object, public std::enable_shared_from_this<core> {
        public:
            static Glib::ustring fix_utf8(const std::string& input);
            static Glib::ustring fix_utf8(const uint8_t* input, int size);
            static Glib::ustring fix_utf8(const int8_t* input, int size);
            static Glib::ustring to_hex(const uint8_t* data, size_t len);
            static std::vector<uint8_t> from_hex(std::string data);

            //props
            Glib::PropertyProxy_ReadOnly<contactAddr>       property_addr();
            Glib::PropertyProxy_ReadOnly<contactAddrPublic> property_addr_public();
            Glib::PropertyProxy<Glib::ustring>              property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name_or_addr();
            Glib::PropertyProxy<Glib::ustring>              property_status_message();
            Glib::PropertyProxy<TOX_USER_STATUS>            property_status();
            Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    property_connection();
            Glib::PropertyProxy<Glib::ustring>              property_download_path();
            Glib::PropertyProxy<Glib::ustring>              property_avatar_path();

            //Signals
            using type_signal_contact_request           = sigc::signal<void, contactAddrPublic, Glib::ustring>;
            using type_signal_contact_message           = sigc::signal<void, contactNr, Glib::ustring>;
            using type_signal_contact_action            = sigc::signal<void, contactNr, Glib::ustring>;
            using type_signal_contact_name              = sigc::signal<void, contactNr, Glib::ustring>;
            using type_signal_contact_status_message    = sigc::signal<void, contactNr, Glib::ustring>;
            using type_signal_contact_status            = sigc::signal<void, contactNr, TOX_USER_STATUS>;
            using type_signal_contact_typing            = sigc::signal<void, contactNr, bool>;
            using type_signal_contact_read_receipt      = sigc::signal<void, contactNr, receiptNr>;
            using type_signal_contact_connection_status = sigc::signal<void, contactNr, TOX_CONNECTION>;
            using type_signal_file_chunk_request        = sigc::signal<void, contactNr, fileNr, uint64_t, size_t>;
            using type_signal_file_recv                 = sigc::signal<void, contactNr, fileNr, TOX_FILE_KIND, size_t, Glib::ustring>;
            using type_signal_file_recv_chunk           = sigc::signal<void, contactNr, fileNr, uint64_t, const std::vector<uint8_t>&>;
            using type_signal_file_recv_control         = sigc::signal<void, contactNr, fileNr, TOX_FILE_CONTROL>;

            type_signal_contact_request           signal_contact_request();
            type_signal_contact_message           signal_contact_message();
            type_signal_contact_action            signal_contact_action();
            type_signal_contact_name              signal_contact_name();
            type_signal_contact_status_message    signal_contact_status_message();
            type_signal_contact_status            signal_contact_status();
            type_signal_contact_typing            signal_contact_typing();
            type_signal_contact_read_receipt      signal_contact_read_receipt();
            type_signal_contact_connection_status signal_contact_connection_status();
            type_signal_file_chunk_request        signal_file_chunk_request();
            type_signal_file_recv                 signal_file_recv();
            type_signal_file_recv_chunk           signal_file_recv_chunk();
            type_signal_file_recv_control         signal_file_recv_control();

            static std::shared_ptr<core> create(const std::string& profile_path,
                                                const std::shared_ptr<storage>& storage);
            void save();
            void update();
            uint32_t update_optimal_interval();
            void destroy();
            ~core();
            Tox* toxcore();
            std::shared_ptr<toxmm::contact_manager> contact_manager();
            std::shared_ptr<toxmm::storage> storage();
            static void try_load(std::string path, Glib::ustring& out_name, Glib::ustring& out_status, contactAddrPublic& out_addr, bool& out_writeable);
            static std::vector<uint8_t> create_state(std::string name, std::string status, contactAddrPublic& out_addr);

            //toxcore functionality
            toxmm::hash hash(const std::vector<uint8_t>& data);

        private:
            Tox* m_toxcore;
            std::string m_profile_path;
            std::shared_ptr<toxmm::storage> m_storage;
            std::shared_ptr<toxmm::contact_manager> m_contact_manager;
            profile m_profile;

            Glib::Property<contactAddr>       m_property_addr;
            Glib::Property<contactAddrPublic> m_property_addr_public;
            Glib::Property<Glib::ustring>     m_property_name;
            Glib::Property<Glib::ustring>     m_property_name_or_addr;
            Glib::Property<Glib::ustring>     m_property_status_message;
            Glib::Property<TOX_USER_STATUS>   m_property_status;
            Glib::Property<TOX_CONNECTION>    m_property_connection;
            Glib::Property<Glib::ustring>     m_property_download_path;
            Glib::Property<Glib::ustring>     m_property_avatar_path;

            type_signal_contact_request           m_signal_contact_request;
            type_signal_contact_message           m_signal_contact_message;
            type_signal_contact_action            m_signal_contact_action;
            type_signal_contact_name              m_signal_contact_name;
            type_signal_contact_status_message    m_signal_contact_status_message;
            type_signal_contact_status            m_signal_contact_status;
            type_signal_contact_typing            m_signal_contact_typing;
            type_signal_contact_read_receipt      m_signal_contact_read_receipt;
            type_signal_contact_connection_status m_signal_contact_connection_status;
            type_signal_file_chunk_request        m_signal_file_chunk_request;
            type_signal_file_recv                 m_signal_file_recv;
            type_signal_file_recv_chunk           m_signal_file_recv_chunk;
            type_signal_file_recv_control         m_signal_file_recv_control;

            core(const std::string& profile_path, const std::shared_ptr<toxmm::storage>& storage);
            core(const core&) = delete;
            void operator=(const core&) = delete;

            void init();

    };

}

#endif
