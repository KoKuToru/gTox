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

namespace toxmm2 {
    class file_manager;
    class contact_manager;
    class contact;
    class core;

    class file: public Glib::Object, public std::enable_shared_from_this<file> {
            friend class file_manager;
            friend class file_recv;

        public:
            //props
            Glib::PropertyProxy_ReadOnly<fileId>           property_id();
            Glib::PropertyProxy_ReadOnly<fileNr>           property_nr();
            Glib::PropertyProxy_ReadOnly<TOX_FILE_KIND>    property_kind();
            Glib::PropertyProxy_ReadOnly<uint64_t>         property_position();
            Glib::PropertyProxy_ReadOnly<uint64_t>         property_size();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>    property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>    property_path();
            Glib::PropertyProxy<TOX_FILE_CONTROL>          property_state();
            Glib::PropertyProxy_ReadOnly<TOX_FILE_CONTROL> property_state_remote();

            std::shared_ptr<toxmm2::core> core();
            std::shared_ptr<toxmm2::file_manager> file_manager();
            std::shared_ptr<toxmm2::contact_manager> contact_manager();
            std::shared_ptr<toxmm2::contact> contact();

            ~file() {}

        protected:
            virtual void resume() = 0;
            virtual void send_chunk_request(uint64_t position, size_t length) = 0;
            virtual void recv_chunk(uint64_t position, const std::vector<uint8_t>& data) = 0;
            virtual void finish() = 0;
            virtual void abort() = 0;
            virtual bool is_recv() = 0;

        private:
            std::weak_ptr<toxmm2::file_manager> m_file_manager;

            Glib::Property<fileId>           m_property_id;
            Glib::Property<fileNr>           m_property_nr;
            Glib::Property<TOX_FILE_KIND>    m_property_kind;
            Glib::Property<uint64_t>         m_property_position;
            Glib::Property<uint64_t>         m_property_size;
            Glib::Property<Glib::ustring>    m_property_name;
            Glib::Property<Glib::ustring>    m_property_path;
            Glib::Property<TOX_FILE_CONTROL> m_property_state;
            Glib::Property<TOX_FILE_CONTROL> m_property_state_remote;

            file(std::shared_ptr<toxmm2::file_manager> manager);
            file(const file&) = delete;
            void operator=(const file&) = delete;

            void init();
    };
}
#endif
