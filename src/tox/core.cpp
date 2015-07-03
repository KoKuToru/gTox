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

//STATIC LINKING REMOVES THIS, THATS WHY WE NEED TODO IT MANUALY
#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
G_DEFINE_CONSTRUCTOR(resource_constructor)
extern "C" void toxmm_db_register_resource (void);
static void resource_constructor (void)
{
   toxmm_db_register_resource();
}


using namespace toxmm2;

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

core::core(std::string path):
    m_path(path) {
    //load
}

void core::save() {
    //save
}

void core::destroy() {
    m_contact_manager->destroy();
    m_contact_manager.reset();
}

core::~core() {
    tox_kill(m_toxcore);
}

std::shared_ptr<core> core::create(std::string path) {
    auto tmp = std::shared_ptr<core>(new core(path));
    tmp->init();
    return tmp;
}

void core::try_load(std::string path, Glib::ustring& out_name, Glib::ustring& out_status, contactPublicAddr& out_addr, bool& out_writeable) {
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
        throw std::runtime_error("Empty state ?");
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
    //get status
    size = tox_self_get_status_message_size(m_toxcore);
    out_status.resize(size);
    tox_self_get_status_message(m_toxcore, (uint8_t*)out_status.raw().data());
    //get addr
    tox_self_get_public_key(m_toxcore, out_addr);
    //check writeable
    out_writeable = m_profile.can_write();
}

void core::init() {
    m_profile.open(m_path);
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

    options->ipv6_enabled = true;
    options->udp_enabled  = true;

    auto state = m_profile.read();
    if (state.empty()) {
        options->savedata_type   = TOX_SAVEDATA_TYPE_NONE;
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

    auto pub = from_hex(
                   "951C88B7E75C8674"
                   "18ACDB5D27382137"
                   "2BB5BD652740BCDF"
                   "623A4FA293E75D2F");
    auto host = "192.254.75.102";
    TOX_ERR_BOOTSTRAP berror;
    if (!tox_bootstrap(m_toxcore, host, 33445, pub.data(), &berror)) {
        throw exception(berror);
    }

    //install events:
    tox_callback_friend_request(toxcore(), [](Tox*, const uint8_t* addr, const uint8_t* data, size_t len, void* _this) {
        ((core*)_this)->signal_contact_request().emit(contactAddr(addr), core::fix_utf8(data, len));
    }, this);
    tox_callback_friend_message(toxcore(), [](Tox*, uint32_t nr, TOX_MESSAGE_TYPE type, const uint8_t* message, size_t len, void* _this) {
        if (type == TOX_MESSAGE_TYPE_NORMAL) {
            ((core*)_this)->signal_contact_message().emit(contactNr(nr), core::fix_utf8(message, len));
        } else {
            ((core*)_this)->signal_contact_action().emit(contactNr(nr), core::fix_utf8(message, len));
        }
    }, this);
    tox_callback_friend_name(toxcore(), [](Tox*, uint32_t nr, const uint8_t* name, size_t len, void* _this) {
        ((core*)_this)->signal_contact_name().emit(contactNr(nr), core::fix_utf8(name, len));
    }, this);
    tox_callback_friend_status_message(toxcore(), [](Tox*, uint32_t nr, const uint8_t* status_message, size_t len, void* _this) {
        ((core*)_this)->signal_contact_status_message().emit(contactNr(nr), core::fix_utf8(status_message, len));
    }, this);
    tox_callback_friend_status(toxcore(), [](Tox*, uint32_t nr, TOX_USER_STATUS status, void* _this) {
        ((core*)_this)->signal_contact_status().emit(contactNr(nr), status);
    }, this);
    tox_callback_friend_typing(toxcore(), [](Tox*, uint32_t nr, bool is_typing, void* _this) {
        ((core*)_this)->signal_contact_typing().emit(contactNr(nr), is_typing);
    }, this);
    tox_callback_friend_read_receipt(toxcore(), [](Tox*, uint32_t nr, uint32_t receipt, void* _this) {
        ((core*)_this)->signal_contact_read_receipt().emit(contactNr(nr), receiptNr(receipt));
    }, this);
    tox_callback_friend_connection_status(toxcore(), [](Tox*, uint32_t nr, TOX_CONNECTION status, void* _this) {
        ((core*)_this)->signal_contact_connection_status().emit(contactNr(nr), status);
    }, this);

    //start sub systems:
    m_contact_manager = std::shared_ptr<contact_manager>(new contact_manager(shared_from_this()));
    m_contact_manager->init();
}

void core::update() {

}

uint32_t core::update_optima_interval() {
    return tox_iteration_interval(m_toxcore);
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
