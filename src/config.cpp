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
#include "resources/flatbuffers/generated/ConfigGlobal_generated.h"
#include "resources/flatbuffers/generated/Config_generated.h"

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

config_global::config_global():
    Glib::ObjectBase(typeid(class config_global)),

    m_config_file(Glib::build_filename(Glib::get_user_config_dir(), "gtox", "config.bin")),

    m_property_connection_udp(*this, "config-connection-udp", true),
    m_property_connection_tcp(*this, "config-connection-tcp", true),
    m_property_proxy_type(*this, "config-proxy-type"),
    m_property_proxy_host(*this, "config-proxy-host"),
    m_property_proxy_port(*this, "config-proxy-post"),
    m_property_theme_color(*this, "config-theme-color", 0),
    m_property_profile_remember(*this, "config-profile-remember", false),
    m_property_video_default_device(*this, "config-video-default-device")
{
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    load_flatbuffer();

    //setup properties
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
        property.signal_changed().connect(sigc::track_obj([this]() {
            this->save_flatbuffer();
        }, *this));
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
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { config_file.raw() });
    load_flatbuffer();

    //setup settings:
    for_each_in_tuple(std::make_tuple(
                          property_systemtray_visible(),
                          property_systemtray_on_start(),
                          property_systemtray_on_close(),
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
        property.signal_changed().connect(sigc::track_obj([this]() {
            this->save_flatbuffer();
        }, *this));
    });
}

class config_global& config::global() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    static class config_global global;
    return global;
}

void config::load_flatbuffer() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (!Glib::file_test(m_config_file, Glib::FILE_TEST_IS_REGULAR)) {
        save_flatbuffer();
    }
    auto file = Gio::File::create_for_path(m_config_file);
    auto stream = file->read();
    std::vector<uint8_t> content(stream->query_info()->get_size());
    gsize size;
    stream->read_all((void*)content.data(), content.size(), size);

    auto verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::Config::VerifyConfigBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Config::VerifyGlobalBuffer failed");
    }

    auto conf = flatbuffers::Config::GetConfig(content.data());

    property_systemtray_visible() = conf->systemtray_visible();
    property_systemtray_on_start() = conf->systemtray_on_start();
    property_systemtray_on_close() = conf->systemtray_on_close();

    property_chat_notification_on_message() = conf->chat_notification_on_message();
    property_chat_notification_with_audio() = conf->chat_notification_with_audio();
    property_chat_auto_away() = conf->chat_auto_away();
    property_chat_send_typing() = conf->chat_send_typing();
    property_chat_logging() = conf->chat_logging();
    property_file_save_path() = std::string(conf->file_save_path()->begin(),
                                            conf->file_save_path()->end());
    property_file_auto_accept() = conf->file_auto_accept();
    property_file_display_inline() = conf->file_display_inline();
    property_contacts_compact_list() = conf->contacts_compact_list();
    property_contacts_display_active() = conf->contacts_display_active();

}

void config::save_flatbuffer() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    flatbuffers::FlatBufferBuilder fbb;

    auto file_save_path = fbb.CreateString(property_file_save_path().get_value());

    auto builder = flatbuffers::Config::ConfigBuilder(fbb);

    builder.add_systemtray_visible(property_systemtray_visible());
    builder.add_systemtray_on_start(property_systemtray_on_start());
    builder.add_systemtray_on_close(property_systemtray_on_close());

    builder.add_chat_notification_on_message(property_chat_notification_on_message());
    builder.add_chat_notification_with_audio(property_chat_notification_with_audio());
    builder.add_chat_auto_away(property_chat_auto_away());
    builder.add_chat_send_typing(property_chat_send_typing());
    builder.add_chat_logging(property_chat_logging());

    builder.add_file_save_path(file_save_path);
    builder.add_file_auto_accept(property_file_auto_accept());
    builder.add_file_display_inline(property_file_display_inline());

    builder.add_contacts_compact_list(property_contacts_compact_list());
    builder.add_contacts_display_active(property_contacts_display_active());

    flatbuffers::Config::FinishConfigBuffer(fbb, builder.Finish());

    //write to file
    auto file = Gio::File::create_for_path(m_config_file);
    auto parent = file->get_parent();
    if (parent) {
        if (!Glib::file_test(parent->get_path(), Glib::FILE_TEST_IS_DIR)) {
            parent->make_directory_with_parents();
        }
    }
    auto stream = file->replace();
    stream->truncate(0);
    stream->write_bytes(Glib::Bytes::create(fbb.GetBufferPointer(), fbb.GetSize()));
    stream->close();
}

void config_global::load_flatbuffer() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (!Glib::file_test(m_config_file, Glib::FILE_TEST_IS_REGULAR)) {
        save_flatbuffer();
    }
    auto file = Gio::File::create_for_path(m_config_file);
    auto stream = file->read();
    std::vector<uint8_t> content(stream->query_info()->get_size());
    gsize size;
    stream->read_all((void*)content.data(), content.size(), size);

    auto verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::Config::VerifyGlobalBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Config::VerifyGlobalBuffer failed");
    }

    auto conf = flatbuffers::Config::GetGlobal(content.data());

    property_connection_udp() = conf->connection_udp();
    property_connection_tcp() = conf->connection_tcp();

    property_proxy_type() = conf->proxy_type();
    property_proxy_host() = std::string(conf->proxy_host()->begin(),
                                        conf->proxy_host()->end());
    property_proxy_port() = conf->proxy_port();

    property_theme_color() = conf->theme_color();

    property_video_default_device() = std::string(
                                          conf->av_video_default()->begin(),
                                          conf->av_video_default()->end());
}

void config_global::save_flatbuffer() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    flatbuffers::FlatBufferBuilder fbb;

    auto proxy_host = fbb.CreateString(property_proxy_host().get_value());
    auto video_default = fbb.CreateString(property_video_default_device().get_value());

    auto builder = flatbuffers::Config::GlobalBuilder(fbb);

    builder.add_connection_udp(property_connection_udp());
    builder.add_connection_tcp(property_connection_tcp());

    builder.add_proxy_type(property_proxy_type());
    builder.add_proxy_host(proxy_host);
    builder.add_proxy_port(property_proxy_port());

    builder.add_theme_color(property_theme_color());

    builder.add_av_video_default(video_default);

    flatbuffers::Config::FinishGlobalBuffer(fbb, builder.Finish());

    //write to file
    auto file = Gio::File::create_for_path(m_config_file);
    auto parent = file->get_parent();
    if (parent) {
        if (!Glib::file_test(parent->get_path(), Glib::FILE_TEST_IS_DIR)) {
            parent->make_directory_with_parents();
        }
    }
    auto stream = file->replace();
    stream->truncate(0);
    stream->write_bytes(Glib::Bytes::create(fbb.GetBufferPointer(), fbb.GetSize()));
    stream->close();
}
