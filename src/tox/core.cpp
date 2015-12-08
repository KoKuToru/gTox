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
#include "core.h"
#include "contact/manager.h"
#include "exception.h"
#include "av.h"
#include <iostream>

using namespace toxmm;

struct bootstrap_node {
        std::string ipv4;
        std::string ipv6;
        unsigned short port;
        std::string pubkey;
};

//TODO: hardcoded for now, somehow move it into a config
static const std::vector<bootstrap_node>& get_bootstrap_nodes() {
    static std::vector<bootstrap_node> bootstrap_nodes = {{
        {"44.76.60.215",    "2a01:4f8:191:64d6::1",               33445, "04119E835DF3E78BACF0F84235B300546AF8B936F035185E2A8E9E0A67C8924F"},
        {"23.226.230.47",   "2604:180:1::3ded:b280",              33445, "A09162D68618E742FFBCA1C2C70385E6679604B2D80EA6E84AD0996A1AC8A074"},
        {"178.21.112.187",  "2a02:2308::216:3eff:fe82:eaef",      33445, "4B2C19E924972CB9B57732FB172F8A8604DE13EEDA2A6234E348983344B23057"},
        {"195.154.119.113", "2001:bc8:3698:101::1",               33445, "E398A69646B8CEACA9F0B84F553726C1C49270558C57DF5F3C368F05A7D71354"},
        {"192.210.149.121", "",                                   33445, "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67"},
        {"46.38.239.179",   "",                                   33445, "F5A1A38EFB6BD3C2C8AF8B10D85F0F89E931704D349F1D0720C3C4059AF2440A"},
        {"178.62.250.138",  "2a03:b0c0:2:d0::16:1",               33445, "788236D34978D1D5BD822F0A5BEBD2C53C64CC31CD3149350EE27D4D9A2F9B6B"},
        {"130.133.110.14",  "2001:6f8:1c3c:babe::14:1",           33445, "461FA3776EF0FA655F1A05477DF1B3B614F7D6B124F7DB1DD4FE3C08B03B640F"},
        {"104.167.101.29",  "",                                   33445, "5918AC3C06955962A75AD7DF4F80A5D7C34F7DB9E1498D2E0495DE35B3FE8A57"},
        {"205.185.116.116", "",                                   33445, "A179B09749AC826FF01F37A9613F6B57118AE014D4196A0E1105A98F93A54702"},
        {"198.98.51.198",   "2605:6400:1:fed5:22:45af:ec10:f329", 33445, "1D5A5F2F5D6233058BF0259B09622FB40B482E4FA0931EB8FD3AB8E7BF7DAF6F"},
        {"80.232.246.79",   "",                                   33445, "CF6CECA0A14A31717CC8501DA51BE27742B70746956E6676FF423A529F91ED5D"},
        {"108.61.165.198",  "",                                   33445, "8E7D0B859922EF569298B4D261A8CCB5FEA14FB91ED412A7603A585A25698832"},
        {"212.71.252.109",  "2a01:7e00::f03c:91ff:fe69:9912",     33445, "C4CEB8C7AC607C6B374E2E782B3C00EA3A63B80D4910B8649CCACDD19F260819"},
        {"194.249.212.109", "2001:1470:fbfe::109",                33445, "3CEE1F054081E7A011234883BC4FC39F661A55B73637A5AC293DDF1251D9432B"},
        {"185.25.116.107",  "2a00:7a60:0:746b::3",                33445, "DA4E4ED4B697F2E9B000EEFE3A34B554ACD3F45F5C96EAEA2516DD7FF9AF7B43"},
        {"192.99.168.140",  "2607:5300:100:200::::667",           33445, "6A4D0607A296838434A6A7DDF99F50EF9D60A2C510BBF31FE538A25CB6B4652F"},
        {"46.101.197.175",  "2a03:b0c0:3:d0::ac:5001",              443, "CD133B521159541FB1D326DE9850F5E56A6C724B5B8E5EB5CD8D950408E95707"}
    }};
    return bootstrap_nodes;
}

//SIGNALS
core::type_signal_contact_request           core::signal_contact_request() { return m_signal_contact_request; }
core::type_signal_contact_message           core::signal_contact_message() { return m_signal_contact_message; }
core::type_signal_contact_action            core::signal_contact_action()  { return m_signal_contact_action; }
core::type_signal_contact_name              core::signal_contact_name()    { return m_signal_contact_name; }
core::type_signal_contact_status_message    core::signal_contact_status_message()  { return m_signal_contact_status_message; }
core::type_signal_contact_status            core::signal_contact_status()  { return m_signal_contact_status; }
core::type_signal_contact_typing            core::signal_contact_typing()  { return m_signal_contact_typing; }
core::type_signal_contact_read_receipt      core::signal_contact_read_receipt()  { return m_signal_contact_read_receipt; }
core::type_signal_contact_connection_status core::signal_contact_connection_status()  { return m_signal_contact_connection_status; }
core::type_signal_file_chunk_request        core::signal_file_chunk_request()  { return m_signal_file_chunk_request; }
core::type_signal_file_recv                 core::signal_file_recv()  { return m_signal_file_recv; }
core::type_signal_file_recv_chunk           core::signal_file_recv_chunk()  { return m_signal_file_recv_chunk; }
core::type_signal_file_recv_control         core::signal_file_recv_control() { return m_signal_file_recv_control; }

Glib::PropertyProxy_ReadOnly<contactAddr> core::property_addr() {
    return {this, "self-addr"};
}
Glib::PropertyProxy_ReadOnly<contactAddrPublic> core::property_addr_public() {
    return {this, "self-addr-public"};
}
Glib::PropertyProxy<Glib::ustring> core::property_name() {
    return m_property_name.get_proxy();
}
Glib::PropertyProxy_ReadOnly<Glib::ustring> core::property_name_or_addr() {
    return {this, "self-name-or-addr"};
}
Glib::PropertyProxy<Glib::ustring> core::property_status_message() {
    return m_property_status_message.get_proxy();
}
Glib::PropertyProxy<TOX_USER_STATUS> core::property_status() {
    return m_property_status.get_proxy();
}
Glib::PropertyProxy_ReadOnly<TOX_CONNECTION> core::property_connection() {
    return {this, "self-connection"};
}

core::core(const std::string& profile_path,
           const std::shared_ptr<toxmm::storage>& storage):
    Glib::ObjectBase(typeid(core)),
    m_profile_path(profile_path),
    m_storage(storage),
    m_property_addr(*this, "self-addr"),
    m_property_addr_public(*this, "self-addr-public"),
    m_property_name(*this, "self-name"),
    m_property_name_or_addr(*this, "self-name-or-addr"),
    m_property_status_message(*this, "self-status-message"),
    m_property_status(*this, "self-status"),
    m_property_connection(*this, "self-connection") {
    //rest is done in init()
}

void core::save() {
    std::vector<uint8_t> state(tox_get_savedata_size(m_toxcore));
    tox_get_savedata(m_toxcore, state.data());
    if (m_profile.can_write()) {
        m_profile.write(state);
    }
}

void core::destroy() {
    m_contact_manager->destroy();
    m_contact_manager.reset();
    m_av.reset();
}

core::~core() {
    if (m_toxcore) {
        save();
        tox_kill(m_toxcore);
        m_toxcore = nullptr;
    }
}

std::shared_ptr<core> core::create(const std::string& profile_path,
                                   const std::shared_ptr<toxmm::storage>& storage) {
    auto tmp = std::shared_ptr<core>(new core(profile_path, storage));
    tmp->init();
    return tmp;
}

std::vector<uint8_t> core::create_state(std::string name, std::string status, contactAddrPublic& out_addr_public) {
    TOX_ERR_OPTIONS_NEW nerror;
    auto options = std::shared_ptr<Tox_Options>(tox_options_new(&nerror),
                                                [](Tox_Options* p) {
                                                    tox_options_free(p);
                                                });
    if (nerror != TOX_ERR_OPTIONS_NEW_OK) {
        throw exception(nerror);
    }
    options->savedata_type   = TOX_SAVEDATA_TYPE_NONE;
    TOX_ERR_NEW error;
    Tox* m_toxcore = tox_new(options.get(), &error);
    if (error != TOX_ERR_NEW_OK) {
        throw exception(error);
    }
    //set name
    TOX_ERR_SET_INFO serror;
    tox_self_set_name(m_toxcore, (const uint8_t*)name.data(), name.size(), &serror);
    if (serror != TOX_ERR_SET_INFO_OK) {
        throw exception(error);
    }
    //set status
    tox_self_set_status_message(m_toxcore, (const uint8_t*)status.data(), status.size(), &serror);
    if (serror != TOX_ERR_SET_INFO_OK) {
        throw exception(error);
    }
    //get addr
    tox_self_get_public_key(m_toxcore, out_addr_public);
    //get state
    std::vector<uint8_t> state(tox_get_savedata_size(m_toxcore));
    tox_get_savedata(m_toxcore, state.data());
    return state;
}

void core::try_load(std::string path, Glib::ustring& out_name, Glib::ustring& out_status, contactAddrPublic& out_addr_public, bool& out_writeable) {
    profile m_profile;
    m_profile.open(path);
    if (!m_profile.can_read()) {
        throw std::runtime_error("Couldn't read toxcore profile");
    }
    TOX_ERR_OPTIONS_NEW nerror;
    auto options = std::shared_ptr<Tox_Options>(tox_options_new(&nerror),
                                                [](Tox_Options* p) {
                                                    tox_options_free(p);
                                                });
    if (nerror != TOX_ERR_OPTIONS_NEW_OK) {
        throw exception(nerror);
    }
    auto state = m_profile.read();
    if (state.empty()) {
        throw std::runtime_error("Toxcore profile is empty, 0 bytes");
    }
    options->savedata_type   = TOX_SAVEDATA_TYPE_TOX_SAVE;
    options->savedata_data   = state.data();
    options->savedata_length = state.size();
    TOX_ERR_NEW error;
    Tox* m_toxcore = tox_new(options.get(), &error);
    if (error != TOX_ERR_NEW_OK) {
        throw exception(error);
    }
    //get name
    auto size = tox_self_get_name_size(m_toxcore);
    out_name.resize(size);
    tox_self_get_name(m_toxcore, (uint8_t*)out_name.raw().data());
    out_name = fix_utf8(out_name);
    //get status
    size = tox_self_get_status_message_size(m_toxcore);
    out_status.resize(size);
    tox_self_get_status_message(m_toxcore, (uint8_t*)out_status.raw().data());
    out_status = fix_utf8(out_status);
    //get addr
    tox_self_get_public_key(m_toxcore, out_addr_public);
    //check writeable
    out_writeable = m_profile.can_write();
}

void core::init() {
    m_profile.open(m_profile_path);
    if (!m_profile.can_read()) {
        throw std::runtime_error("Couldn't read toxcore profile");
    }

    TOX_ERR_OPTIONS_NEW nerror;
    auto options = std::shared_ptr<Tox_Options>(tox_options_new(&nerror),
                                                [](Tox_Options* p) {
                                                    tox_options_free(p);
                                                });
    if (nerror != TOX_ERR_OPTIONS_NEW_OK) {
        throw exception(nerror);
    }

    //first load with disabled, just to get the pub id
    options->ipv6_enabled = false;
    options->udp_enabled  = false;

    std::vector<uint8_t> state = m_profile.read();
    if (state.empty()) {
        options->savedata_type   = TOX_SAVEDATA_TYPE_NONE;
        //TODO this shall throw an error ?
    } else {
        options->savedata_type   = TOX_SAVEDATA_TYPE_TOX_SAVE;
        options->savedata_data   = state.data();
        options->savedata_length = state.size();
    }
    TOX_ERR_NEW error;
    m_toxcore = tox_new(options.get(), &error);
    if (error != TOX_ERR_NEW_OK) {
        throw exception(error);
    }

    //set prefix for storage
    {
        contactAddrPublic addr_public;
        tox_self_get_public_key(m_toxcore, addr_public);
        m_storage->set_prefix_key(addr_public);
        m_config = config::create(m_storage);
    }

    //reload client witht the right settings now
    tox_kill(m_toxcore);
    options->ipv6_enabled = true;
    options->udp_enabled  = m_config->property_connection_udp();
    options->proxy_type   = m_config->property_proxy_type().get_value();
    options->proxy_host   = m_config->property_proxy_host().get_value().c_str();
    options->proxy_port   = m_config->property_proxy_port();

    m_toxcore = tox_new(options.get(), &error);
    if (error == TOX_ERR_NEW_PROXY_BAD_HOST ||
        error == TOX_ERR_NEW_PROXY_BAD_TYPE ||
        error == TOX_ERR_NEW_PROXY_BAD_PORT ||
        error == TOX_ERR_NEW_PROXY_NOT_FOUND) {
        //TODO: display error to user

        //try without proxy
        options->proxy_type = TOX_PROXY_TYPE_NONE;
        m_toxcore = tox_new(options.get(), &error);
    }

    if (error != TOX_ERR_NEW_OK) {
        throw exception(error);
    }

    m_bootstrap_timer.reset();

    //install events:
    tox_callback_friend_request(toxcore(), [](Tox*, const uint8_t* addr, const uint8_t* data, size_t len, void* _this) {
        ((core*)_this)->m_signal_contact_request(contactAddrPublic(addr), core::fix_utf8(data, len));
    }, this);
    tox_callback_friend_message(toxcore(), [](Tox*, uint32_t nr, TOX_MESSAGE_TYPE type, const uint8_t* message, size_t len, void* _this) {
        if (type == TOX_MESSAGE_TYPE_NORMAL) {
            ((core*)_this)->m_signal_contact_message(contactNr(nr), core::fix_utf8(message, len));
        } else {
            ((core*)_this)->m_signal_contact_action(contactNr(nr), core::fix_utf8(message, len));
        }
    }, this);
    tox_callback_friend_name(toxcore(), [](Tox*, uint32_t nr, const uint8_t* name, size_t len, void* _this) {
        ((core*)_this)->m_signal_contact_name(contactNr(nr), core::fix_utf8(name, len));
        ((core*)_this)->save(); //TODO: delay this save
    }, this);
    tox_callback_friend_status_message(toxcore(), [](Tox*, uint32_t nr, const uint8_t* status_message, size_t len, void* _this) {
        ((core*)_this)->m_signal_contact_status_message(contactNr(nr), core::fix_utf8(status_message, len));
        ((core*)_this)->save(); //TODO: delay this save
    }, this);
    tox_callback_friend_status(toxcore(), [](Tox*, uint32_t nr, TOX_USER_STATUS status, void* _this) {
        ((core*)_this)->m_signal_contact_status(contactNr(nr), status);
    }, this);
    tox_callback_friend_typing(toxcore(), [](Tox*, uint32_t nr, bool is_typing, void* _this) {
        ((core*)_this)->m_signal_contact_typing(contactNr(nr), is_typing);
    }, this);
    tox_callback_friend_read_receipt(toxcore(), [](Tox*, uint32_t nr, uint32_t receipt, void* _this) {
        ((core*)_this)->m_signal_contact_read_receipt(contactNr(nr), receiptNr(receipt));
    }, this);
    tox_callback_friend_connection_status(toxcore(), [](Tox*, uint32_t nr, TOX_CONNECTION status, void* _this) {
        ((core*)_this)->m_signal_contact_connection_status(contactNr(nr), status);
    }, this);
    tox_callback_self_connection_status(toxcore(), [](Tox *, TOX_CONNECTION connection_status, void* _this) {
        ((core*)_this)->m_property_connection = connection_status;
    }, this);
    tox_callback_file_chunk_request(toxcore(), [](Tox*, uint32_t nr, uint32_t file_number, uint64_t position, size_t length, void *_this) {
        ((core*)_this)->m_signal_file_chunk_request(contactNr(nr), fileNr(file_number), position, length);
    }, this);
    tox_callback_file_recv(toxcore(), [](Tox*, uint32_t nr, uint32_t file_number, uint32_t kind, uint64_t file_size, const uint8_t *filename, size_t filename_length, void *_this) {
        ((core*)_this)->m_signal_file_recv(contactNr(nr), fileNr(file_number), TOX_FILE_KIND(kind), file_size, core::fix_utf8(filename, filename_length));
    }, this);
    tox_callback_file_recv_chunk(toxcore(), [](Tox*, uint32_t nr, uint32_t file_number, uint64_t position, const uint8_t *data, size_t length, void *_this) {
        ((core*)_this)->m_signal_file_recv_chunk(contactNr(nr), fileNr(file_number), position, std::vector<uint8_t>(data, data+length));
    }, this);
    tox_callback_file_recv_control(toxcore(), [](Tox*, uint32_t nr, uint32_t file_number, TOX_FILE_CONTROL state, void* _this) {
       ((core*)_this)->m_signal_file_recv_control(contactNr(nr), fileNr(file_number), state);
    }, this);

    //install logic for name_or_addr
    auto update_name_or_addr = [this]() {
        if (property_name().get_value().empty()) {
            m_property_name_or_addr = Glib::ustring(m_property_addr.get_value());
        } else {
            m_property_name_or_addr = property_name().get_value();
        }
        save();
    };
    property_name().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));
    property_addr().signal_changed().connect(sigc::track_obj(update_name_or_addr, *this));

    //get addr
    contactAddr addr;
    tox_self_get_address(m_toxcore, addr);
    m_property_addr = addr;
    contactAddrPublic addr_public;
    tox_self_get_public_key(m_toxcore, addr_public);
    m_property_addr_public = addr_public;
    //get name
    std::string name(tox_self_get_name_size(m_toxcore), 0);
    tox_self_get_name(m_toxcore, (uint8_t*)name.data());
    m_property_name = fix_utf8(name);
    //get status_message
    std::string status_message(tox_self_get_status_message_size(m_toxcore), 0);
    tox_self_get_status_message(m_toxcore, (uint8_t*)status_message.data());
    m_property_status_message = fix_utf8(status_message);
    //get status
    m_property_status = tox_self_get_status(m_toxcore);
    //get connection
    m_property_connection = tox_self_get_connection_status(m_toxcore);

    //install change events for properties
    property_name().signal_changed().connect(sigc::track_obj([this]() {
        const std::string& name = property_name().get_value();
        TOX_ERR_SET_INFO error;
        tox_self_set_name(m_toxcore, (const uint8_t*)name.data(), name.size(), &error);
        if (error != TOX_ERR_SET_INFO_OK) {
            throw exception(error);
        }
    }, *this));
    property_status_message().signal_changed().connect(sigc::track_obj([this]() {
        const std::string& status_message = property_status_message().get_value();
        TOX_ERR_SET_INFO error;
        tox_self_set_status_message(m_toxcore, (const uint8_t*)status_message.data(), status_message.size(), &error);
        if (error != TOX_ERR_SET_INFO_OK) {
            throw exception(error);
        }
    }, *this));
    property_status().signal_changed().connect(sigc::track_obj([this]() {
        tox_self_set_status(m_toxcore, property_status());
    }, *this));

    //start sub systems:
    m_av = std::shared_ptr<toxmm::av>(new toxmm::av(shared_from_this()));
    m_av->init();
    m_contact_manager = std::shared_ptr<toxmm::contact_manager>(new toxmm::contact_manager(shared_from_this()));
    m_contact_manager->init();

    m_contact_manager->signal_added().connect(
                sigc::track_obj(sigc::hide([this]() {
        save();
    }), *this));
    m_contact_manager->signal_removed().connect(
                sigc::track_obj(sigc::hide([this]() {
        save();
    }), *this));
}

std::shared_ptr<toxmm::contact_manager> core::contact_manager() {
    return m_contact_manager;
}

std::shared_ptr<toxmm::config> core::config() {
    return m_config;
}

std::shared_ptr<toxmm::storage> core::storage() {
    return m_storage;
}

std::shared_ptr<toxmm::av> core::av() {
    return m_av;
}

void core::update() {
    tox_iterate(toxcore());

    auto timer_wait = 10;
    auto timer_connections = 5;
    auto conenction_type = config()->property_connection_udp()
                           ?TOX_CONNECTION_UDP
                           :TOX_CONNECTION_TCP;
    if (property_connection() != conenction_type) {
        timer_wait = 60;
        timer_connections = 1;
    }

    if (m_bootstrap_timer.elapsed() >= timer_wait) {
        auto nodes = get_bootstrap_nodes();
        for (auto i = 0; i < timer_connections; ++i) {
            auto index = rand() % nodes.size();
            TOX_ERR_BOOTSTRAP berror;
            auto pub = from_hex(nodes[index].pubkey);
            auto host = nodes[index].ipv4;
            auto port = nodes[index].port;
            if (!host.empty()) {
                if (!tox_bootstrap(m_toxcore,
                                   host.c_str(),
                                   port, pub.data(),
                                   &berror)) {
                    if (berror == TOX_ERR_BOOTSTRAP_BAD_HOST) {
                        std::cerr << "TOX_ERR_BOOTSTRAP_BAD_HOST on host:" << host << " port:" << port << std::endl;
                    } else if (berror == TOX_ERR_BOOTSTRAP_BAD_PORT) {
                        std::cerr << "TOX_ERR_BOOTSTRAP_BAD_PORT on host:" << host << " port:" << port << std::endl;
                    } else {
                        throw exception(berror);
                    }
                }
            }
            host = nodes[index].ipv6;
            if (!host.empty()) {
                if (!tox_bootstrap(m_toxcore,
                                   host.c_str(),
                                   port, pub.data(),
                                   &berror)) {
                    if (berror == TOX_ERR_BOOTSTRAP_BAD_HOST) {
                        std::cerr << "TOX_ERR_BOOTSTRAP_BAD_HOST on host:" << host << " port:" << port << std::endl;
                    } else if (berror == TOX_ERR_BOOTSTRAP_BAD_PORT) {
                        std::cerr << "TOX_ERR_BOOTSTRAP_BAD_PORT on host:" << host << " port:" << port << std::endl;
                    } else {
                        throw exception(berror);
                    }
                }
            }
        }
        m_bootstrap_timer.reset();
    }
}

uint32_t core::update_optimal_interval() {
    return tox_iteration_interval(m_toxcore);
}

Glib::ustring core::fix_utf8(const std::string& input) {
    return fix_utf8((const uint8_t*)input.data(), input.size());
}

Glib::ustring core::fix_utf8(const uint8_t* input, int size) {
    return fix_utf8((const int8_t*)input, size);
}

Glib::ustring core::fix_utf8(const int8_t* input, int size) {
    static const Glib::ustring uFFFD(1, gunichar(0xFFFD));
    std::string fixed;
    fixed.reserve(size);
    const gchar* ginput = (gchar*)input;
    const gchar* str = ginput;
    const gchar* last_valid;
    while(!g_utf8_validate(str, std::distance(str, ginput + size), &last_valid)) {
        fixed.append(str, last_valid);
        fixed.append(uFFFD.raw().begin(), uFFFD.raw().end());
        str = last_valid + 1;
    }
    fixed.append(str, last_valid);
    return fixed;
}

Glib::ustring core::to_hex(const uint8_t* data, size_t len) {
    std::string s;
    for (size_t i = 0; i < len; ++i) {
        static const int8_t hex[] = "0123456789ABCDEF";
        s += hex[(data[i] >> 4) & 0xF];
        s += hex[data[i] & 0xF];
    }
    return s;
}

std::vector<uint8_t> core::from_hex(std::string data) {
    std::vector<uint8_t> tmp;
    tmp.reserve(tmp.size() / 2);
    for (size_t i = 0; i < data.size(); i += 2) {
        tmp.push_back(std::stoi(data.substr(i, 2), 0, 16));  // pretty stupid
    }
    return tmp;
}

Tox* core::toxcore() {
    return m_toxcore;
}

toxmm::hash core::hash(const std::vector<uint8_t>& data) {
    toxmm::hash res;
    tox_hash(res, data.data(), data.size());
    return res;
}
