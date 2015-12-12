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
#include <giomm.h>
#include "av.h"
#include "utils.h"

namespace toxmm {
    class file_manager;
    class receipt;
    class call;
    class contact : public Glib::Object, public std::enable_shared_from_this<contact> {
            friend class contact_manager;
            friend class file_manager;
        public:
            //functions
            std::shared_ptr<receipt> send_message(const std::string& message);
            std::shared_ptr<receipt> send_action (const std::string& action);

            std::shared_ptr<toxmm::core> core();
            std::shared_ptr<toxmm::contact_manager> contact_manager();
            std::shared_ptr<toxmm::file_manager> file_manager();
            std::shared_ptr<toxmm::call> call();

        private:
            std::weak_ptr<toxmm::contact_manager> m_contact_manager;
            std::shared_ptr<toxmm::file_manager>  m_file_manager;
            std::shared_ptr<toxmm::call>          m_call;

            std::shared_ptr<toxmm::file>  m_avatar_send;
            Glib::RefPtr<Gio::FileMonitor> m_avatar_send_monitor;

            contact(std::shared_ptr<toxmm::contact_manager> manager, contactNr nr);
            contact(const contact&) = delete;
            void operator=(const contact&) = delete;

            void init();

            contactAddrPublic toxcore_get_addr();
            Glib::ustring     toxcore_get_name();
            Glib::ustring     toxcore_get_status_message();
            TOX_USER_STATUS   toxcore_get_status();
            TOX_CONNECTION    toxcore_get_connection();

            // Install all properties
            INST_PROP_RO (contactNr        , property_nr, "contact-nr")
            INST_PROP_RO (contactAddrPublic, property_addr_public, "contact-addr-public")
            INST_PROP_RO (Glib::ustring    , property_name, "contact-name")
            INST_PROP_RO (Glib::ustring    , property_name_or_addr, "contact-name-or-addr")
            INST_PROP_RO (Glib::ustring    , property_status_message, "contact-status-message")
            INST_PROP_RO (TOX_USER_STATUS  , property_status, "contact-status")
            INST_PROP_RO (TOX_CONNECTION   , property_connection, "contact-connection")
            INST_PROP    (bool             , property_typing, "contact-typing")
            INST_PROP_RO (bool             , property_remote_typing, "contact-remote-typing")

            // Install all signals
            INST_SIGNAL (signal_receipt                , void, receiptNr)
            INST_SIGNAL (signal_recv_message           , void, Glib::ustring)
            INST_SIGNAL (signal_recv_action            , void, Glib::ustring)
            INST_SIGNAL (signal_send_message           , void, Glib::ustring, std::shared_ptr<receipt>)
            INST_SIGNAL (signal_send_action            , void, Glib::ustring, std::shared_ptr<receipt>)
            INST_SIGNAL (signal_send_file_chunk_request, void, fileNr, uint64_t, size_t)
            INST_SIGNAL (signal_recv_file              , void, fileNr, TOX_FILE_KIND, size_t, Glib::ustring)
            INST_SIGNAL (signal_recv_file_chunk        , void, fileNr, uint64_t, const std::vector<uint8_t>&)
            INST_SIGNAL (signal_recv_file_control      , void, fileNr, TOX_FILE_CONTROL)
    };

}

#endif
