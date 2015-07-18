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
#ifndef TOXMM_FILE_RECV_H
#define TOXMM_FILE_RECV_H
#include <tox/tox.h>
#include <glibmm.h>
#include <giomm.h>
#include "file.h"

namespace toxmm2 {
    class file_recv: virtual public Glib::Object, public file {
            friend class file_manager;
        protected:
            void resume();
            void send_chunk_request(uint64_t, size_t);
            void recv_chunk(uint64_t position, const std::vector<uint8_t>& chunk);
            void finish();
            void abort();
            bool is_recv();

        private:
            Glib::RefPtr<Gio::FileOutputStream> m_stream;

            file_recv(std::shared_ptr<toxmm2::file_manager> manager);
            file_recv(const file_recv&) = delete;
            void operator=(const file_recv&) = delete;
    };
}
#endif
