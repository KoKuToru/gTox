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
#ifndef TOXMM_FILE_H
#define TOXMM_FILE_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"
#include "utils.h"

namespace toxmm {
    class file_manager;
    class contact_manager;
    class contact;
    class core;

    class file:
            virtual public Glib::Object,
            public std::enable_shared_from_this<file> {

            friend class file_manager;
            friend class file_recv;
            friend class file_send;

        public:
            auto core()            -> std::shared_ptr<toxmm::core>;
            auto file_manager()    -> std::shared_ptr<toxmm::file_manager>;
            auto contact_manager() -> std::shared_ptr<toxmm::contact_manager>;
            auto contact()         -> std::shared_ptr<toxmm::contact>;

            virtual ~file() {}
            virtual bool is_recv() = 0;

            file();
            file(std::shared_ptr<toxmm::file_manager> manager);
            file(const file&) = delete;
            void operator=(const file&) = delete;

        protected:
            virtual void resume() = 0;
            virtual void send_chunk_request(uint64_t position, size_t length) = 0;
            virtual void recv_chunk(uint64_t position, const std::vector<uint8_t>& data) = 0;
            virtual void finish() = 0;
            virtual void abort() = 0;

            void pre_send_chunk_request(uint64_t position, size_t length);
            void pre_recv_chunk(uint64_t position, const std::vector<uint8_t>& data);
            void seek(uint64_t position);

        private:
            std::weak_ptr<toxmm::file_manager> m_file_manager;

            void init();

            // Install properties
            INST_PROP_RO (uniqueId        , property_uuid, "file-uuid")
            INST_PROP_RO (fileId          , property_id, "file-id")
            INST_PROP_RO (fileNr          , property_nr, "file-nr")
            INST_PROP_RO (TOX_FILE_KIND   , property_kind, "file-kind")
            INST_PROP_RO (uint64_t        , property_position, "file-position")
            INST_PROP_RO (uint64_t        , property_size, "file-size")
            INST_PROP_RO (Glib::ustring   , property_name, "file-name")
            INST_PROP_RO (Glib::ustring   , property_path, "file-path")
            INST_PROP    (TOX_FILE_CONTROL, property_state, "file-state")
            INST_PROP_RO (TOX_FILE_CONTROL, property_state_remote, "file-remote")
            INST_PROP_RO (double          , property_progress, "file-progress")
            INST_PROP_RO (bool            , property_complete, "file-complete")
            INST_PROP_RO (bool            , property_active, "file-active")
    };
}
#endif
