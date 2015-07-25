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

namespace toxmm2 {
    class file;
    class core;
    class contact_manager;
    class file_manager: public std::enable_shared_from_this<file_manager> {
            friend class contact;
        public:
            //signals
            using type_signal_recv          = sigc::signal<void, std::shared_ptr<file>&>;
            using type_signal_send          = sigc::signal<void, std::shared_ptr<file>&>;
            using type_signal_recv_chunk    = sigc::signal<void, std::shared_ptr<file>&, const std::vector<uint8_t>&>;
            using type_signal_send_chunk    = sigc::signal<void, std::shared_ptr<file>&, const std::vector<uint8_t>&>;
            using type_signal_send_chunk_rq = sigc::signal<void, std::shared_ptr<file>&, uint64_t, size_t>;
            using type_signal_recv_control  = sigc::signal<void, std::shared_ptr<file>&, TOX_FILE_CONTROL>;
            using type_signal_send_control  = sigc::signal<void, std::shared_ptr<file>&, TOX_FILE_CONTROL>;

            //! Emits when a new file is incoming
            type_signal_recv          signal_recv_file();
            //! Emits when a new file will go out
            type_signal_send          signal_send_file();
            //! Emits when a chunk for a incoming file was received
            type_signal_recv_chunk    signal_recv_chunk();
            //! Emits when a chunk for a outgoing file will be send
            type_signal_send_chunk    signal_send_chunk();
            //! Emits when a chunk for a outgoing file is requested
            type_signal_send_chunk_rq signal_send_chunk_request();
            //! Emits when a when a control-command was received
            type_signal_recv_control  signal_recv_control();
            //! Emits when a when a control-command will be send
            type_signal_send_control  signal_send_control();

            //functions
            std::shared_ptr<file> find(fileNr nr);
            std::shared_ptr<file> find(uniqueId id);

            std::shared_ptr<file> send_file(const Glib::ustring& path, bool avatar = false);

            std::shared_ptr<toxmm2::core> core();
            std::shared_ptr<toxmm2::contact_manager> contact_manager();
            std::shared_ptr<toxmm2::contact> contact();

        private:
            std::weak_ptr<toxmm2::contact> m_contact;
            std::vector<std::shared_ptr<file>> m_file;

            type_signal_recv          m_signal_recv_file;
            type_signal_send          m_signal_send_file;
            type_signal_recv_chunk    m_signal_recv_chunk;
            type_signal_send_chunk    m_signal_send_chunk;
            type_signal_send_chunk_rq m_signal_send_chunk_rq;
            type_signal_recv_control  m_signal_recv_control;
            type_signal_send_control  m_signal_send_control;

            file_manager(std::shared_ptr<toxmm2::contact> contact);
            file_manager(const file_manager&) = delete;
            void operator=(const file_manager&) = delete;

            void init();
            void load();
            void save();
    };
}

#endif
