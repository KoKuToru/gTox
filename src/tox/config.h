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
#ifndef TOXMM_CONFIG_H
#define TOXMM_CONFIG_H
#include <glibmm.h>
#include "storage.h"
#include <tox/tox.h>
#include "utils.h"

namespace toxmm {
    class config : public Glib::Object, public std::enable_shared_from_this<config> {
        public:
            static std::shared_ptr<config> create(const std::shared_ptr<toxmm::storage> storage);

        private:
            void load_flatbuffer();
            void save_flatbuffer();

            std::shared_ptr<toxmm::storage> m_storage;

            config(const std::shared_ptr<toxmm::storage> storage);
            config(const config&) = delete;
            void operator=(const config&) = delete;

            // Install properties
            INST_PROP (Glib::ustring , property_download_path, "config-download-path")
            INST_PROP (Glib::ustring , property_avatar_path, "config-avatar-path")
            INST_PROP (bool          , property_connection_udp, "config-connection-udp")
            INST_PROP (TOX_PROXY_TYPE, property_proxy_type, "config-proxy-type")
            INST_PROP (Glib::ustring , property_proxy_host, "config-proxy-host")
            INST_PROP (int           , property_proxy_port, "config-proxy-port")
    };
}

#endif
