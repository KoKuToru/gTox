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
#include "flatbuffers/generated/Config_generated.h"

using namespace toxmm;

Glib::PropertyProxy<Glib::ustring> config::property_download_path() {
    return m_property_download_path.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> config::property_avatar_path() {
    return m_property_avatar_path.get_proxy();
}

Glib::PropertyProxy<bool> config::property_connection_udp() {
    return m_property_connection_udp.get_proxy();
}

Glib::PropertyProxy<bool> config::property_connection_tcp() {
    return m_property_connection_tcp.get_proxy();
}

Glib::PropertyProxy<TOX_PROXY_TYPE> config::property_proxy_type() {
    return m_property_proxy_type.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> config::property_proxy_host() {
    return m_property_proxy_host.get_proxy();
}

Glib::PropertyProxy<int>           config::property_proxy_port() {
    return m_property_proxy_port.get_proxy();
}

template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I == sizeof...(Ts)>::type
for_each_in_tuple(const std::tuple<Ts...> &, Func) {}

template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I < sizeof...(Ts)>::type
for_each_in_tuple(const std::tuple<Ts...> & tpl, Func func) {
    func(std::get<I>(tpl));
    for_each_in_tuple<I + 1>(tpl,func);
}

config::config(const std::shared_ptr<toxmm::storage> storage):
    Glib::ObjectBase(typeid(this)),
    m_storage(storage),
    m_property_download_path(*this, "toxmm-config-download-path",
                             Glib::get_user_special_dir(
                                 GUserDirectory::G_USER_DIRECTORY_DOWNLOAD)),
    m_property_avatar_path(*this, "toxmm-config-avatar-path",
                           Glib::build_filename(
                               Glib::get_user_config_dir(),
                               "tox",
                               "avatars")),
    m_property_connection_udp(*this, "toxmm-config-connection-udp", true),
    m_property_connection_tcp(*this, "toxmm-config-connection-tcp", true),
    m_property_proxy_type(*this, "toxmm-config-proxy-type"),
    m_property_proxy_host(*this, "toxmm-config-proxy-host", ""),
    m_property_proxy_port(*this, "toxmm-config-proxy-port") {
    load_flatbuffer();

    //setup properties
    for_each_in_tuple(std::make_tuple(
                          property_download_path(),
                          property_avatar_path(),
                          property_connection_udp(),
                          property_connection_tcp(),
                          property_proxy_type(),
                          property_proxy_host(),
                          property_proxy_port()),
                      [this](auto property) {
        property.signal_changed().connect(sigc::track_obj([this]() {
            this->save_flatbuffer();
        }, *this));
    });
}

void config::load_flatbuffer() {
    std::vector<uint8_t> content;
    m_storage->load({"toxmm_config"}, content);

    if (content.empty()) {
        return;
    }

    auto verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::Config::VerifyConfigBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Config::VerifyConfigBuffer failed");
    }

    auto data = flatbuffers::Config::GetConfig(content.data());

    property_download_path() = std::string(data->download_path()->begin(), data->download_path()->end());
    property_avatar_path() = std::string(data->avatar_path()->begin(), data->avatar_path()->end());
    property_connection_udp() = data->connection_udp();
    property_connection_tcp() = data->connection_tcp();
    property_proxy_type() = TOX_PROXY_TYPE(data->proxy_type());
    property_proxy_host() = std::string(data->proxy_host()->begin(), data->proxy_host()->end());
    property_proxy_port() = data->proxy_port();
}

void config::save_flatbuffer() {
    using namespace flatbuffers;

    FlatBufferBuilder fbb;
    Config::ConfigBuilder fb(fbb);
    auto download_path = fbb.CreateString(property_download_path().get_value());
    auto avatar_path   = fbb.CreateString(property_avatar_path().get_value());
    auto proxy_host    = fbb.CreateString(property_proxy_host().get_value());
    fb.add_download_path(download_path);
    fb.add_avatar_path(avatar_path);
    fb.add_connection_udp(property_connection_udp());
    fb.add_connection_tcp(property_connection_tcp());
    fb.add_proxy_type(int(property_proxy_type()));
    fb.add_proxy_host(proxy_host);
    fb.add_proxy_port(property_proxy_port());

    Config::FinishConfigBuffer(fbb, fb.Finish());

    std::vector<uint8_t> content(fbb.GetBufferPointer(),
                                 fbb.GetBufferPointer() + fbb.GetSize());

    m_storage->save({"toxmm_config"}, content);
}

std::shared_ptr<config> config::create(const std::shared_ptr<toxmm::storage> storage) {
    return std::shared_ptr<config>(new config(storage));
}
