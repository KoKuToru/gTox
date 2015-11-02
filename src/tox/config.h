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

namespace toxmm {
    class config : public Glib::Object, public std::enable_shared_from_this<config> {
        public:
            //props
            Glib::PropertyProxy<Glib::ustring> property_download_path();
            Glib::PropertyProxy<Glib::ustring> property_avatar_path();

            Glib::PropertyProxy<bool> property_connection_udp();
            Glib::PropertyProxy<bool> property_connection_tcp();

            Glib::PropertyProxy<TOX_PROXY_TYPE> property_proxy_type();
            Glib::PropertyProxy<Glib::ustring>  property_proxy_host();
            Glib::PropertyProxy<int>            property_proxy_port();

            static std::shared_ptr<config> create(const std::shared_ptr<toxmm::storage> storage);

        private:
            void load_flatbuffer();
            void save_flatbuffer();

            std::shared_ptr<toxmm::storage> m_storage;

            Glib::Property<Glib::ustring> m_property_download_path;
            Glib::Property<Glib::ustring> m_property_avatar_path;

            Glib::Property<bool> m_property_connection_udp;
            Glib::Property<bool> m_property_connection_tcp;

            Glib::Property<TOX_PROXY_TYPE> m_property_proxy_type;
            Glib::Property<Glib::ustring>  m_property_proxy_host;
            Glib::Property<int>            m_property_proxy_port;

            config(const std::shared_ptr<toxmm::storage> storage);
            config(const config&) = delete;
            void operator=(const config&) = delete;
    };
}

#endif
