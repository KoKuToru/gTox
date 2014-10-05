/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
    Copyright (C) 2014  Maurice Mohlek

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
#include "Tox.h"
#include <fstream>

std::recursive_mutex Tox::m_mtx;
Tox* Tox::m_instance = nullptr;

Tox::Tox(): m_tox(nullptr) {

}

Tox::~Tox() {
    if (m_tox != nullptr) {
        //SAVE STATE ?
        tox_kill(m_tox);
        m_tox = nullptr;
    }
}

Tox& Tox::instance() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_instance == nullptr) {
        m_instance = new Tox;
    }
    return *m_instance;
}

void Tox::destroy() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void Tox::init(const Glib::ustring& statefile) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox != nullptr) {
        throw "ERROR";
    }
    Tox_Options options;
    options.ipv6enabled = true;
    options.udp_disabled = false;
    options.proxy_enabled = false;
    m_tox = tox_new(&options);

    //install callbacks
    tox_callback_friend_request(m_tox, Tox::callback_friend_request, nullptr);
    tox_callback_friend_message(m_tox, Tox::callback_friend_message, nullptr);
    tox_callback_friend_action (m_tox, Tox::callback_friend_action , nullptr);
    tox_callback_name_change   (m_tox, Tox::callback_name_change   , nullptr);
    tox_callback_status_message(m_tox, Tox::callback_status_message       , nullptr);
    tox_callback_user_status   (m_tox, Tox::callback_user_status          , nullptr);
    tox_callback_typing_change (m_tox, Tox::callback_typing_change        , nullptr);
    tox_callback_read_receipt  (m_tox, Tox::callback_read_receipt         , nullptr);
    tox_callback_connection_status (m_tox, Tox::callback_connection_status, nullptr);

    //load state
    if (statefile != "") {
        std::ifstream oi(statefile);
        if (!oi.is_open()) {
            throw "ERROR";
        }

        uint32_t state_size;
        oi.read((char*)&state_size, 4);
        std::vector<unsigned char> state(state_size);
        oi.read((char*)state.data(), state.size());

        if (tox_load(m_tox, state.data(), state.size()) != -1) {
            throw "ERROR";
        }
    }

    unsigned char pub_key[] = {
        0xA0, 0x91, 0x62, 0xD6, 0x86, 0x18, 0xE7, 0x42, 0xFF, 0xBC, 0xA1, 0xC2, 0xC7, 0x03, 0x85, 0xE6, 0x67, 0x96, 0x04, 0xB2, 0xD8, 0x0E, 0xA6, 0xE8, 0x4A, 0xD0, 0x99, 0x6A, 0x1A, 0xC8, 0xA0, 0x74
    };
    if (!tox_bootstrap_from_address(m_tox, "23.226.230.47", 33445, pub_key)) { // connect to a bootstrap to get into the network
        throw "ERROR";
    }
}

int Tox::update_optimal_interval() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    return tox_do_interval(m_tox);
}

bool Tox::update(Tox::SEvent& ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    tox_do(m_tox);
    if (events.empty()) {
        return false;
    }
    ev = events.front();
    events.pop_front();
    return true;
}

std::vector<Tox::FriendNr> Tox::get_friendlist() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    std::vector<FriendNr> tmp(tox_count_friendlist(m_tox));
    tmp.resize(tox_get_friendlist(m_tox, tmp.data(), tmp.size()));
    return tmp;
}

Tox::FriendAddr Tox::get_address() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    FriendAddr tmp;
    tox_get_address(m_tox, tmp.data());
    return tmp;
}

Tox::FriendNr Tox::add_friend(Tox::FriendAddr addr, const Glib::ustring& message) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    FriendNr res = tox_add_friend(m_tox, addr.data(), reinterpret_cast<const unsigned char*>(message.data()), message.bytes());
    if (res < 0) {
        throw "ERROR";
    }
    return res;
}

Tox::FriendNr Tox::add_friend_norequest(FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    FriendNr res = tox_add_friend_norequest(m_tox, addr.data());
    if (res < 0) {
        throw "ERROR";
    }
    return res;
}

Tox::FriendNr Tox::get_friend_number(Tox::FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    FriendNr res = tox_get_friend_number(m_tox, addr.data());
    if (res < 0) {
        throw "ERROR";
    }
    return res;
}

void Tox::del_friend(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    if (tox_del_friend(m_tox, nr) != 0) {
        throw "ERROR";
    }
}

Tox::ReceiptNr Tox::send_message(Tox::FriendNr nr, const Glib::ustring& message) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    Tox::ReceiptNr res = tox_send_message(m_tox, nr, reinterpret_cast<const unsigned char*>(message.data()), message.bytes());
    if (res == 0) {
        throw "ERROR";
    }
    return res;
}

Tox::ReceiptNr Tox::send_action(Tox::FriendNr nr, const Glib::ustring& action) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    Tox::ReceiptNr res = tox_send_action(m_tox, nr, reinterpret_cast<const unsigned char*>(action.data()), action.bytes());
    if (res == 0) {
        throw "ERROR";
    }
    return res;
}

void Tox::set_name(const Glib::ustring& name) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    if (tox_set_name(m_tox, reinterpret_cast<const unsigned char*>(name.data()), name.bytes()) != 0) {
        throw "ERROR";
    }
}

Glib::ustring Tox::get_name() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    std::string name(/*MAX_NAME_LENGTH*/128, 0);
    name.resize(tox_get_self_name(m_tox, (unsigned char*)(name.data())));
    if (name.size() == 0) {
        throw "ERROR";
    }
    return name;
}

Glib::ustring Tox::get_status_message() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    int size = tox_get_self_status_message_size(m_tox);
    if (size < 0) {
        throw "ERROR";
    }
    std::string name(size, 0);
    name.resize(tox_get_self_status_message(m_tox, (unsigned char*)name.data(), name.size()));
    if (name.size() == 0) {
        throw "ERROR";
    }
    return name;
}

Glib::ustring Tox::get_status_message(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    int size = tox_get_status_message_size(m_tox, nr);
    if (size < 0) {
        throw "ERROR";
    }
    std::string name(size, 0);
    name.resize(tox_get_status_message(m_tox, nr, (unsigned char*)name.data(), name.size()));
    if (name.size() == 0) {
        throw "ERROR";
    }
    return name;
}

Tox::EUSERSTATUS Tox::get_status() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    int status = tox_isconnected(m_tox);
    if (status < 0) {
        throw "ERROR";
    }
    if (status == 0) {
        return EUSERSTATUS::OFFLINE;
    }

    status = tox_get_self_user_status(m_tox);
    if (status == EUSERSTATUS::INVALID) {
        throw "ERROR";
    }

    return (EUSERSTATUS)status;
}


Tox::EUSERSTATUS Tox::get_status(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    int status = tox_get_friend_connection_status(m_tox, nr);
    if (status < 0) {
        throw "ERROR";
    }
    if (status == 0) {
        return EUSERSTATUS::OFFLINE;
    }

    status = tox_get_user_status(m_tox, nr);
    if (status == EUSERSTATUS::INVALID) {
        throw "ERROR";
    }

    return (EUSERSTATUS)status;
}

unsigned long long Tox::get_last_online(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    unsigned long long t = tox_get_last_online(m_tox, nr);
    if (t == ~0ull) {
        throw "ERROR";
    }
    return t;
}

void Tox::send_typing(FriendNr nr, bool is_typing) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    if (tox_set_user_is_typing(m_tox, nr, is_typing) < 0) {
        throw "ERROR";
    }
}

void Tox::inject_event(const SEvent& ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw "ERROR";
    }
    events.push_back(ev);
}


void Tox::callback_friend_request(Tox *, const unsigned char* addr,const unsigned char* data, unsigned short len, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDREQUEST;
    std::copy(addr, addr+ tmp.friend_request.addr.size(), tmp.friend_request.addr.begin());
    tmp.friend_request.message = Glib::ustring(std::string((const char*)data, len)); //no shortcut ?
    Tox::instance().inject_event(tmp);
}

void Tox::callback_friend_message(Tox *, FriendNr nr, const unsigned char* data, unsigned short len, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDMESSAGE;
    tmp.friend_message.nr = nr;
    tmp.friend_message.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_friend_action(Tox *, FriendNr nr, const unsigned char* data, unsigned short len, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDACTION;
    tmp.friend_action.nr = nr;
    tmp.friend_action.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_name_change(Tox *, FriendNr nr, const unsigned char* data, unsigned short len, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::NAMECHANGE;
    tmp.name_change.nr = nr;
    tmp.name_change.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_status_message(Tox *, FriendNr nr, const unsigned char* data, unsigned short len, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::STATUSMESSAGE;
    tmp.status_message.nr = nr;
    tmp.status_message.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_user_status(Tox *, FriendNr nr, unsigned char data, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::USERSTATUS;
    tmp.user_status.nr = nr;
    tmp.user_status.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_typing_change(Tox *, FriendNr nr, unsigned char data, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::TYPINGCHANGE;
    tmp.typing_change.nr = nr;
    tmp.typing_change.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_read_receipt(Tox *, FriendNr nr, unsigned data, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::READRECEIPT;
    tmp.readreceipt.nr = nr;
    tmp.readreceipt.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_connection_status(Tox * m, FriendNr nr, unsigned char data, void *) {
    Tox::SEvent tmp;
    tmp.event = EEventType::USERSTATUS;
    tmp.user_status.nr = nr;
    if (data == 0) {
        tmp.user_status.data = EUSERSTATUS::OFFLINE;
    } else {
        //went online get user status
        tmp.user_status.data = tox_get_user_status(m, nr);
    }
    Tox::instance().inject_event(tmp);
}
