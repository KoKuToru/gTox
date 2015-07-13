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
#ifndef GTOX_CONFIG_H
#define GTOX_CONFIG_H
#include <glibmm.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include "libs/rapidjson/document.h"

class config_global: public Glib::Object {
        friend class config;
    public:
        Glib::PropertyProxy<bool> property_connection_udp();
        Glib::PropertyProxy<bool> property_connection_tcp();

        Glib::PropertyProxy<int>           property_proxy_type();
        Glib::PropertyProxy<Glib::ustring> property_proxy_host();
        Glib::PropertyProxy<int>           property_proxy_port();

        Glib::PropertyProxy<int> property_theme_color();

        Glib::PropertyProxy<bool> property_profile_remember();

        Glib::PropertyProxy<Glib::ustring> property_video_default_device();

    private:
        config_global();

        std::string m_config_file;
        rapidjson::Document m_config_json;

        Glib::Property<bool> m_property_connection_udp;
        Glib::Property<bool> m_property_connection_tcp;

        Glib::Property<int>           m_property_proxy_type;
        Glib::Property<Glib::ustring> m_property_proxy_host;
        Glib::Property<int>           m_property_proxy_port;

        Glib::Property<int> m_property_theme_color;

        Glib::Property<bool> m_property_profile_remember;

        Glib::Property<Glib::ustring> m_property_video_default_device;
};

class config: public Glib::Object {
    public:
        Glib::PropertyProxy<bool> property_systemtray_visible();
        Glib::PropertyProxy<bool> property_systemtray_on_start();
        Glib::PropertyProxy<bool> property_systemtray_on_close();

        Glib::PropertyProxy<bool> property_chat_notification_on_message();
        Glib::PropertyProxy<bool> property_chat_notification_with_audio();
        Glib::PropertyProxy<int>  property_chat_auto_away();
        Glib::PropertyProxy<bool> property_chat_send_typing();
        Glib::PropertyProxy<bool> property_chat_logging();

        Glib::PropertyProxy<Glib::ustring> property_file_save_path();
        Glib::PropertyProxy<bool>          property_file_auto_accept();
        Glib::PropertyProxy<bool>          property_file_display_inline();

        Glib::PropertyProxy<bool> property_contacts_compact_list();
        Glib::PropertyProxy<bool> property_contacts_display_active();

        config(Glib::ustring config_file);

        static config_global& global();

    private:
        std::string m_config_file;
        rapidjson::Document m_config_json;

        Glib::Property<bool> m_property_systemtray_visible;
        Glib::Property<bool> m_property_systemtray_on_start;
        Glib::Property<bool> m_property_systemtray_on_close;

        Glib::Property<bool> m_property_chat_notification_on_message;
        Glib::Property<bool> m_property_chat_notification_with_audio;
        Glib::Property<int>  m_property_chat_auto_away;
        Glib::Property<bool> m_property_chat_send_typing;
        Glib::Property<bool> m_property_chat_logging;

        Glib::Property<Glib::ustring> m_property_file_save_path;
        Glib::Property<bool>          m_property_file_auto_accept;
        Glib::Property<bool>          m_property_file_display_inline;

        Glib::Property<bool> m_property_contacts_compact_list;
        Glib::Property<bool> m_property_contacts_display_active;
};

#endif
