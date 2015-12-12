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
#ifndef TOXMM_FILE_MANAGER_H
#define TOXMM_FILE_MANAGER_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"
#include "utils.h"

namespace toxmm {
    class file;
    class core;
    class contact_manager;
    class file_manager: public std::enable_shared_from_this<file_manager> {
            friend class contact;
        public:
            ~file_manager();

            //functions
            std::shared_ptr<file> find(fileNr nr);
            std::shared_ptr<file> find(uniqueId id);

            std::shared_ptr<file> send_file(const Glib::ustring& path, bool avatar = false);

            std::shared_ptr<toxmm::core> core();
            std::shared_ptr<toxmm::contact_manager> contact_manager();
            std::shared_ptr<toxmm::contact> contact();

        private:
            std::weak_ptr<toxmm::contact> m_contact;
            std::vector<std::shared_ptr<file>> m_file;

            file_manager(std::shared_ptr<toxmm::contact> contact);
            file_manager(const file_manager&) = delete;
            void operator=(const file_manager&) = delete;

            void init();
            void load();
            void save();

            // Install signals
            //! Emits when a new file is incoming
            INST_SIGNAL (signal_recv_file          , void, std::shared_ptr<file>&)
            //! Emits when a new file will go out
            INST_SIGNAL (signal_send_file          , void, std::shared_ptr<file>&)
            //! Emits when a chunk for a incoming file was received
            INST_SIGNAL (signal_recv_chunk         , void, std::shared_ptr<file>&, const std::vector<uint8_t>&)
            //! Emits when a chunk for a outgoing file will be send
            INST_SIGNAL (signal_send_chunk         , void, std::shared_ptr<file>&, const std::vector<uint8_t>&)
            //! Emits when a chunk for a outgoing file is requested
            INST_SIGNAL (signal_send_chunk_request , void, std::shared_ptr<file>&, uint64_t, size_t)
            //! Emits when a when a control-command was received
            INST_SIGNAL (signal_recv_control       , void, std::shared_ptr<file>&, TOX_FILE_CONTROL)
            //! Emits when a when a control-command will be send
            INST_SIGNAL (signal_send_control       , void, std::shared_ptr<file>&, TOX_FILE_CONTROL)

    };
}

#endif
