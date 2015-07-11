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
#include "config.h"
#include <iostream>
#include <giomm.h>
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/rapidjson/allocators.h"

//PROFILE:
Glib::PropertyProxy<bool> config::property_systemtray_visible()
{ return m_property_systemtray_visible.get_proxy(); }

Glib::PropertyProxy<bool> config::property_systemtray_on_start()
{ return m_property_systemtray_on_start.get_proxy(); }

Glib::PropertyProxy<bool> config::property_systemtray_on_close()
{ return m_property_systemtray_on_close.get_proxy(); }

Glib::PropertyProxy<bool> config::property_chat_notification_on_message()
{ return m_property_chat_notification_on_message.get_proxy(); }

Glib::PropertyProxy<bool> config::property_chat_notification_with_audio()
{ return m_property_chat_notification_with_audio.get_proxy(); }

Glib::PropertyProxy<int>  config::property_chat_auto_away()
{ return m_property_chat_auto_away.get_proxy(); }

Glib::PropertyProxy<bool> config::property_chat_send_typing()
{ return m_property_chat_send_typing.get_proxy(); }

Glib::PropertyProxy<bool> config::property_chat_logging()
{ return m_property_chat_logging.get_proxy(); }

Glib::PropertyProxy<Glib::ustring> config::property_file_save_path()
{ return m_property_file_save_path.get_proxy(); }

Glib::PropertyProxy<bool> config::property_file_auto_accept()
{ return m_property_file_auto_accept.get_proxy(); }

Glib::PropertyProxy<bool> config::property_file_display_inline()
{ return m_property_file_display_inline.get_proxy(); }

Glib::PropertyProxy<bool> config::property_contacts_compact_list()
{ return m_property_contacts_compact_list.get_proxy(); }

Glib::PropertyProxy<bool> config::property_contacts_display_active()
{ return m_property_contacts_display_active.get_proxy(); }

//GLOBAL:
Glib::PropertyProxy<bool> config_global::property_connection_udp()
{ return m_property_connection_udp.get_proxy(); }

Glib::PropertyProxy<bool> config_global::property_connection_tcp()
{ return m_property_connection_tcp.get_proxy(); }

Glib::PropertyProxy<int> config_global::property_proxy_type()
{ return m_property_proxy_type.get_proxy(); }

Glib::PropertyProxy<Glib::ustring> config_global::property_proxy_host()
{ return m_property_proxy_host.get_proxy(); }

Glib::PropertyProxy<int> config_global::property_proxy_port()
{ return m_property_proxy_port.get_proxy(); }

Glib::PropertyProxy<int> config_global::property_theme_color()
{ return m_property_theme_color.get_proxy(); }

Glib::PropertyProxy<bool> config_global::property_profile_remember()
{ return m_property_profile_remember.get_proxy(); }

Glib::PropertyProxy<Glib::ustring> config_global::property_video_default_device()
{ return m_property_video_default_device.get_proxy(); }


template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I == sizeof...(Ts)>::type
for_each_in_tuple(const std::tuple<Ts...> &, Func) {}

template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I < sizeof...(Ts)>::type
for_each_in_tuple(const std::tuple<Ts...> & tpl, Func func) {
    func(std::get<I>(tpl));
    for_each_in_tuple<I + 1>(tpl,func);
}

Glib::ustring json_getValue(rapidjson::Value& value, const Glib::ustring&) {
    return value.GetString();
}

bool json_getValue(rapidjson::Value& value, bool) {
    return value.GetBool();
}

int json_getValue(rapidjson::Value& value, int) {
    return value.GetInt();
}

void json_setValue(rapidjson::Value& value, const Glib::ustring& data, rapidjson::MemoryPoolAllocator<>& allocator) {
    value.SetString(data.c_str(), data.size(), allocator);
}

void json_setValue(rapidjson::Value& value, int data, rapidjson::MemoryPoolAllocator<>&) {
    value.SetInt(data);
}

void json_setValue(rapidjson::Value& value, bool data, rapidjson::MemoryPoolAllocator<>&) {
    value.SetBool(data);
}

config_global::config_global():
    Glib::ObjectBase(typeid(class config_global)),

    m_config_file(Glib::build_filename(Glib::get_user_config_dir(), "gtox", "config.json")),

    m_property_connection_udp(*this, "config-connection-udp", true),
    m_property_connection_tcp(*this, "config-connection-tcp", true),
    m_property_proxy_type(*this, "config-proxy-type"),
    m_property_proxy_host(*this, "config-proxy-host"),
    m_property_proxy_port(*this, "config-proxy-post"),
    m_property_theme_color(*this, "config-theme-color", 0),
    m_property_profile_remember(*this, "config-profile-remember", false),
    m_property_video_default_device(*this, "config-video-default-device")
{
    //load json
    if (Glib::file_test(m_config_file, Glib::FILE_TEST_IS_REGULAR)) {
        auto file = Gio::File::create_for_path(m_config_file);
        auto stream = file->read();
        std::vector<char> json(stream->query_info()->get_size() + 1);
        gsize size;
        stream->read_all((void*)json.data(), json.size(), size);
        if (size+1 != json.size()) {
            //some thing went wrong
            return;
        }
        m_config_json.Parse(json.data());
    } else {
        m_config_json.Parse("{}");
    }

    //load settings
    for_each_in_tuple(std::make_tuple(
                          property_connection_udp(),
                          property_connection_tcp(),
                          property_proxy_type(),
                          property_proxy_host(),
                          property_proxy_port(),
                          property_theme_color(),
                          property_theme_color(),
                          property_profile_remember(),
                          property_video_default_device()),
                      [this](auto property) {
        if (m_config_json.HasMember(property.get_name())) {
            rapidjson::Value& value = m_config_json[property.get_name()];
            if (value.IsNull() == false) {
                property.set_value(json_getValue(value, property.get_value()));
            }
        } else {
            //thats pretty ugly
            rapidjson::Value name(property.get_name(), m_config_json.GetAllocator());
            m_config_json.AddMember(name, rapidjson::Value(), m_config_json.GetAllocator());
        }
        property.signal_changed().connect([this, property]() {
            rapidjson::Value& value = m_config_json[property.get_name()];
            json_setValue(value, property.get_value(), m_config_json.GetAllocator());

            //stringfy
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_config_json.Accept(writer);

            auto file = Gio::File::create_for_path(m_config_file);
            auto parent = file->get_parent();
            if (parent) {
                if (!Glib::file_test(parent->get_path(), Glib::FILE_TEST_IS_DIR)) {
                    parent->make_directory_with_parents();
                }
            }
            auto stream = file->replace();
            stream->truncate(0);
            stream->write(buffer.GetString());
            stream->close();
        });
    });
}

config::config(Glib::ustring config_file):
    Glib::ObjectBase(typeid(config)),
    m_config_file(config_file),

    m_property_systemtray_visible(*this, "config-systemtray-visible", true),
    m_property_systemtray_on_start(*this, "config-systemtray-on-start", false),
    m_property_systemtray_on_close(*this, "config-systemtray-on-close", true),
    m_property_chat_notification_on_message(*this, "config-chat-notification-on-message", true),
    m_property_chat_notification_with_audio(*this, "config-chat-notification-with-audio", true),
    m_property_chat_auto_away(*this, "property-chat-auto-away", 0),
    m_property_chat_send_typing(*this, "property-chat-send-typing", true),
    m_property_chat_logging(*this, "property-chat-logging", true),
    m_property_file_save_path(*this, "config-file-save-path", Glib::get_user_special_dir(GUserDirectory::G_USER_DIRECTORY_DOWNLOAD)),
    m_property_file_auto_accept(*this, "config-file-auto-accept", true),
    m_property_file_display_inline(*this, "config-file-display-inline", true),
    m_property_contacts_compact_list  (*this, "config-contacts-compact-list", false),
    m_property_contacts_display_active(*this, "config-contacts-display-active", true)
{
    //load json
    if (Glib::file_test(m_config_file, Glib::FILE_TEST_IS_REGULAR)) {
        auto file = Gio::File::create_for_path(m_config_file);
        auto stream = file->read();
        std::vector<char> json(stream->query_info()->get_size() + 1);
        gsize size;
        stream->read_all((void*)json.data(), json.size(), size);
        if (size+1 != json.size()) {
            //some thing went wrong
            return;
        }
        m_config_json.Parse(json.data());
    } else {
        m_config_json.Parse("{}");
    }

    //load settings:
    for_each_in_tuple(std::make_tuple(
                          property_systemtray_visible(),
                          property_systemtray_on_start(),
                          property_file_display_inline(),
                          property_chat_notification_on_message(),
                          property_chat_notification_with_audio(),
                          property_chat_auto_away(),
                          property_chat_send_typing(),
                          property_chat_logging(),
                          property_file_save_path(),
                          property_file_auto_accept(),
                          property_file_display_inline(),
                          property_contacts_compact_list(),
                          property_contacts_display_active()),
                      [this](auto property) {
        if (m_config_json.HasMember(property.get_name())) {
            rapidjson::Value& value = m_config_json[property.get_name()];
            if (value.IsNull() == false) {
                property.set_value(json_getValue(value, property.get_value()));
            }
        } else {
            //thats pretty ugly
            rapidjson::Value name(property.get_name(), m_config_json.GetAllocator());
            m_config_json.AddMember(name, rapidjson::Value(), m_config_json.GetAllocator());
        }
        property.signal_changed().connect([this, property]() {
            rapidjson::Value& value = m_config_json[property.get_name()];
            json_setValue(value, property.get_value(), m_config_json.GetAllocator());

            //stringfy
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_config_json.Accept(writer);

            auto file = Gio::File::create_for_path(m_config_file);
            auto parent = file->get_parent();
            if (parent) {
                if (!Glib::file_test(parent->get_path(), Glib::FILE_TEST_IS_DIR)) {
                    parent->make_directory_with_parents();
                }
            }
            auto stream = file->replace();
            stream->truncate(0);
            stream->write(buffer.GetString());
            stream->close();
        });
    });
}

class config_global& config::global() {
    static class config_global global;
    return global;
}
