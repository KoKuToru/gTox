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

Glib::PropertyProxy<Glib::ustring> config::property_file_save_path()      { return m_property_file_save_path.get_proxy(); }
Glib::PropertyProxy<bool>          config::property_file_auto_accept()    { return m_property_file_auto_accept.get_proxy(); }
Glib::PropertyProxy<bool>          config::property_file_display_inline() { return m_property_file_display_inline.get_proxy(); }

Glib::PropertyProxy<bool> config::property_contacts_compact_list()   { return m_property_contacts_compact_list.get_proxy(); }
Glib::PropertyProxy<bool> config::property_contacts_display_active() { return m_property_contacts_display_active.get_proxy(); }

Glib::PropertyProxy<int> config_global::property_theme_color() { return m_property_theme_color.get_proxy(); }

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

void json_setValue(rapidjson::Value& value, const Glib::ustring& data) {
    value.SetString(data.c_str(), data.size());
}

void json_setValue(rapidjson::Value& value, int data) {
    value.SetInt(data);
}

void json_setValue(rapidjson::Value& value, bool data) {
    value.SetBool(data);
}

config_global::config_global():
    Glib::ObjectBase(typeid(class config_global)),

    m_config_file(Glib::build_filename(Glib::get_user_config_dir(), "gtox", "config.json")),

    m_property_theme_color(*this, "config-theme-color", 0)
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
                          property_theme_color()),
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
            json_setValue(value, property.get_value());

            //stringfy
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_config_json.Accept(writer);

            auto file = Gio::File::create_for_path(m_config_file);
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

    m_property_file_save_path     (*this, "config-file-save-path", Glib::get_user_special_dir(GUserDirectory::G_USER_DIRECTORY_DOWNLOAD)),
    m_property_file_auto_accept   (*this, "config-file-auto-accept", true),
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
            json_setValue(value, property.get_value());

            //stringfy
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_config_json.Accept(writer);

            auto file = Gio::File::create_for_path(m_config_file);
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
