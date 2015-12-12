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
#include "config.h"
#include "utils.h"

namespace toxmm {
    class core : public Glib::Object, public std::enable_shared_from_this<core> {
        public:
            static Glib::ustring fix_utf8(const std::string& input);
            static Glib::ustring fix_utf8(const uint8_t* input, int size);
            static Glib::ustring fix_utf8(const int8_t* input, int size);
            static Glib::ustring to_hex(const uint8_t* data, size_t len);
            static std::vector<uint8_t> from_hex(std::string data);

            static std::shared_ptr<core> create(const std::string& profile_path,
                                                const std::shared_ptr<storage>& storage);
            void save();
            void update();
            uint32_t update_optimal_interval();
            void destroy();
            ~core();
            Tox* toxcore();
            std::shared_ptr<toxmm::contact_manager> contact_manager();
            std::shared_ptr<toxmm::config> config();
            std::shared_ptr<toxmm::storage> storage();
            std::shared_ptr<toxmm::av> av();
            static void try_load(std::string path, Glib::ustring& out_name, Glib::ustring& out_status, contactAddrPublic& out_addr, bool& out_writeable);
            static std::vector<uint8_t> create_state(std::string name, std::string status, contactAddrPublic& out_addr);

            //toxcore functionality
            toxmm::hash hash(const std::vector<uint8_t>& data);

        private:
            Tox* m_toxcore;
            std::string m_profile_path;
            std::shared_ptr<toxmm::config> m_config;
            std::shared_ptr<toxmm::storage> m_storage;
            std::shared_ptr<toxmm::contact_manager> m_contact_manager;
            std::shared_ptr<toxmm::av> m_av;

            profile m_profile;
            Glib::Timer m_bootstrap_timer;

            core(const std::string& profile_path,
                 const std::shared_ptr<toxmm::storage>& storage);
            core(const core&) = delete;
            void operator=(const core&) = delete;

            void init();

            // Install properties
            INST_PROP_RO (contactAddr       , property_addr, "core-addr")
            INST_PROP_RO (contactAddrPublic , property_addr_public, "core-add-public")
            INST_PROP    (Glib::ustring     , property_name, "core-name")
            INST_PROP_RO (Glib::ustring     , property_name_or_addr, "core-name-or-addr")
            INST_PROP    (Glib::ustring     , property_status_message, "core-status-message")
            INST_PROP    (TOX_USER_STATUS   , property_status, "core-status")
            INST_PROP_RO (TOX_CONNECTION    , property_connection, "core-connection")

            // Install signals
            INST_SIGNAL (signal_contact_request           , void, contactAddrPublic, Glib::ustring)
            INST_SIGNAL (signal_contact_message           , void, contactNr, Glib::ustring)
            INST_SIGNAL (signal_contact_action            , void, contactNr, Glib::ustring)
            INST_SIGNAL (signal_contact_name              , void, contactNr, Glib::ustring)
            INST_SIGNAL (signal_contact_status_message    , void, contactNr, Glib::ustring)
            INST_SIGNAL (signal_contact_status            , void, contactNr, TOX_USER_STATUS)
            INST_SIGNAL (signal_contact_typing            , void, contactNr, bool)
            INST_SIGNAL (signal_contact_read_receipt      , void, contactNr, receiptNr)
            INST_SIGNAL (signal_contact_connection_status , void, contactNr, TOX_CONNECTION)
            INST_SIGNAL (signal_file_chunk_request        , void, contactNr, fileNr, uint64_t, size_t)
            INST_SIGNAL (signal_file_recv                 , void, contactNr, fileNr, TOX_FILE_KIND, size_t, Glib::ustring)
            INST_SIGNAL (signal_file_recv_chunk           , void, contactNr, fileNr, uint64_t, const std::vector<uint8_t>&)
            INST_SIGNAL (signal_file_recv_control         , void, contactNr, fileNr, TOX_FILE_CONTROL)
    };

}

#endif
