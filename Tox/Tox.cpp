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
#include <string>
#include <algorithm>
#include "Generated/database.h"

std::recursive_mutex Tox::m_mtx;
Tox* Tox::m_instance = nullptr;

Tox::Tox() : m_tox(nullptr) {
}

Tox::~Tox() {
    if (m_tox != nullptr) {
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

ToxDatabase& Tox::database() {
    return m_db;
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
        tox_kill(m_tox);
        m_tox = nullptr;
    }
    Tox_Options options;
    options.ipv6enabled = true;
    options.udp_disabled = false;
    m_tox = tox_new(&options);

    // install callbacks
    tox_callback_friend_request(m_tox, Tox::callback_friend_request, nullptr);
    tox_callback_friend_message(m_tox, Tox::callback_friend_message, nullptr);
    tox_callback_friend_action(m_tox, Tox::callback_friend_action, nullptr);
    tox_callback_name_change(m_tox, Tox::callback_name_change, nullptr);
    tox_callback_status_message(m_tox, Tox::callback_status_message, nullptr);
    tox_callback_user_status(m_tox, Tox::callback_user_status, nullptr);
    tox_callback_typing_change(m_tox, Tox::callback_typing_change, nullptr);
    tox_callback_read_receipt(m_tox, Tox::callback_read_receipt, nullptr);
    tox_callback_connection_status(
        m_tox, Tox::callback_connection_status, nullptr);

    m_db.close();
    // load state
    bool okay = false;
    if (statefile != "") {
        /* try to open the db */
        m_db.open(statefile);

        // take the last saved state
        auto state = m_db.toxcore_state_get(0);

        if (tox_load(m_tox, state.data(), state.size()) == -1) {
            throw Exception(LOADERROR);
        }

        for (auto boots : m_db.toxcore_bootstrap_get()) {
            if (boots.pub_key.size() == 32) {
                auto pub = from_hex(boots.pub_key);
                okay |= tox_bootstrap_from_address(m_tox,
                                                   boots.ip.c_str(),
                                                   boots.port, pub.data());
            }
        }
    }

    if (!okay) {
        // Fallback ..
        auto pub = from_hex(
            "0095FC11A624EEF1"
            "EED38B54A4BE3E7F"
            "F3527D367DC0ACD1"
            "0AC8329C99319513");
        auto host = "urotukok.net";
        if (!tox_bootstrap_from_address(m_tox, host, 33445, pub.data())) {
            throw Exception(BOOTERROR);
        }
    }
}

void Tox::save(const Glib::ustring& statefile) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }

    int length = (int)tox_size(m_tox);
    std::vector<unsigned char> state(length);
    tox_save(m_tox, (unsigned char*)state.data());
    m_db.toxcore_state_add(state);

    if (statefile != "") {
        m_db.move(statefile);
    }
}

int Tox::update_optimal_interval() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    return tox_do_interval(m_tox);
}

bool Tox::update(Tox::SEvent& ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
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
        throw Exception(UNITIALIZED);
    }
    std::vector<FriendNr> tmp(tox_count_friendlist(m_tox));
    tmp.resize(tox_get_friendlist(m_tox, tmp.data(), tmp.size()));
    return tmp;
}

Tox::FriendAddr Tox::get_address() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    FriendAddr tmp;
    tox_get_address(m_tox, tmp.data());
    return tmp;
}

Tox::FriendNr Tox::add_friend(Tox::FriendAddr addr,
                              const Glib::ustring& message) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    FriendNr res
        = tox_add_friend(m_tox,
                         addr.data(),
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.bytes());
    switch (res) {
        case TOX_FAERR_TOOLONG:
            throw Exception(MSGTOOLONG);
            break;
        case TOX_FAERR_NOMESSAGE:
            throw Exception(MSGEMPTY);
            break;
        case TOX_FAERR_OWNKEY:
            throw Exception(CANTADDYOURSELF);
            break;
        case TOX_FAERR_ALREADYSENT:
            throw Exception(ALREADYSENT);
            break;
        case TOX_FAERR_UNKNOWN:
            throw Exception(FAILED);
            break;
        case TOX_FAERR_BADCHECKSUM:
            throw Exception(BADCHECKSUM);
            break;
        case TOX_FAERR_SETNEWNOSPAM:
            throw Exception(NOSPAM);
            break;
        default:
            break;
    }
    return res;
}

Tox::FriendNr Tox::add_friend_norequest(FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    FriendNr res = tox_add_friend_norequest(m_tox, addr.data());
    if (res < 0) {
        throw Exception(FAILED);
    }
    return res;
}

Tox::FriendNr Tox::get_friend_number(Tox::FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    FriendNr res = tox_get_friend_number(m_tox, addr.data());
    if (res < 0) {
        throw Exception(FAILED);
    }
    return res;
}

void Tox::del_friend(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    if (tox_del_friend(m_tox, nr) != 0) {
        throw Exception(FAILED);
    }
}

Tox::ReceiptNr Tox::send_message(Tox::FriendNr nr,
                                 const Glib::ustring& message) {
    if (message.find("/me ") == 0) {
        return send_action(nr, message);
    }

    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    Tox::ReceiptNr res = tox_send_message(
        m_tox,
        nr,
        reinterpret_cast<const unsigned char*>(message.data()),
        message.bytes());
    if (res == 0) {
        throw Exception(FAILED);
    }

    auto addr = get_address(nr);
    m_db.toxcore_log_add(ToxLogSendEntity(to_hex(addr.data(), addr.size()),
                         ELogType::LOGMSG,
                         message,
                         res));

    return res;
}

Tox::ReceiptNr Tox::send_action(Tox::FriendNr nr, const Glib::ustring& action) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    Tox::ReceiptNr res
        = tox_send_action(m_tox,
                          nr,
                          reinterpret_cast<const unsigned char*>(action.data()),
                          action.bytes());
    if (res == 0) {
        throw Exception(FAILED);
    }

    auto addr = get_address(nr);
    m_db.toxcore_log_add(ToxLogSendEntity(to_hex(addr.data(), addr.size()),
                         ELogType::LOGACTION,
                         action,
                         res));

    return res;
}

void Tox::set_name(const Glib::ustring& name) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    if (tox_set_name(m_tox,
                     reinterpret_cast<const unsigned char*>(name.data()),
                     name.bytes()) != 0) {
        throw Exception(FAILED);
    }
}

Glib::ustring Tox::get_name() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    std::string name(/*MAX_NAME_LENGTH*/ 128, 0);
    int size = tox_get_self_name(m_tox, (unsigned char*)(name.data()));
    if (size < 0) {
        throw Exception(FAILED);
    }
    name.resize(size);
    return name;
}

Glib::ustring Tox::get_name(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    std::string name(/*MAX_NAME_LENGTH*/ 128, 0);
    int size = tox_get_name(m_tox, nr, (unsigned char*)(name.data()));
    if (size < 0) {
        throw Exception(FAILED);
    }
    name.resize(size);
    return name;
}

Glib::ustring Tox::get_name_or_address() {
    Glib::ustring tmp = get_name();
    if (tmp.empty()) {
        return to_hex(get_address().data(), 32);
    }
    return tmp;
}

Glib::ustring Tox::get_name_or_address(Tox::FriendNr nr) {
    Glib::ustring tmp = get_name(nr);
    if (tmp.empty()) {
        return to_hex(get_address(nr).data(), 32);
    }
    return tmp;
}

Glib::ustring Tox::get_status_message() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    int size = tox_get_self_status_message_size(m_tox);
    if (size < 0) {
        throw Exception(FAILED);
    }
    std::string name(size, 0);
    size = tox_get_self_status_message(
        m_tox, (unsigned char*)name.data(), name.size());
    if (size < 0) {
        throw Exception(FAILED);
    }
    name.resize(size);
    return name;
}

Glib::ustring Tox::get_status_message(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    int size = tox_get_status_message_size(m_tox, nr);
    if (size < 0) {
        throw Exception(FAILED);
    }
    std::string name(size, 0);
    size = tox_get_status_message(
        m_tox, nr, (unsigned char*)name.data(), name.size());
    if (size < 0) {
        throw Exception(FAILED);
    }
    name.resize(size);
    return name;
}

void Tox::set_status_message(Glib::ustring msg) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    if (tox_set_status_message(
            m_tox,
            reinterpret_cast<const unsigned char*>(msg.data()),
            msg.bytes()) != 0) {
        throw Exception(FAILED);
    }
}

bool Tox::is_connected() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    int status = tox_isconnected(m_tox);
    return status != 0;
}

Tox::EUSERSTATUS Tox::get_status() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    int status = tox_isconnected(m_tox);
    if (status == 0) {
        return EUSERSTATUS::OFFLINE;
    }

    status = tox_get_self_user_status(m_tox);
    if (status == EUSERSTATUS::INVALID) {
        throw Exception(FAILED);
    }

    return (EUSERSTATUS)status;
}

Tox::EUSERSTATUS Tox::get_status(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    int status = tox_get_friend_connection_status(m_tox, nr);
    if (status < 0) {
        throw Exception(UNITIALIZED);
    }
    if (status == 0) {
        return EUSERSTATUS::OFFLINE;
    }

    status = tox_get_user_status(m_tox, nr);
    if (status == EUSERSTATUS::INVALID) {
        throw Exception(FAILED);
    }

    return (EUSERSTATUS)status;
}

void Tox::set_status(Tox::EUSERSTATUS value) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    if (value == Tox::OFFLINE)
        value = Tox::AWAY;  // we can't set status to offline
    if (tox_set_user_status(m_tox, value) != 0) {
        throw Exception(FAILED);
    }
}

unsigned long long Tox::get_last_online(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    unsigned long long t = tox_get_last_online(m_tox, nr);
    if (t == ~0ull) {
        throw Exception(FAILED);
    }
    return t;
}

void Tox::send_typing(FriendNr nr, bool is_typing) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    if (tox_set_user_is_typing(m_tox, nr, is_typing) < 0) {
        throw Exception(FAILED);
    }
}

void Tox::inject_event(SEvent ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }

    if (ev.event == FRIENDMESSAGE) {
        // check if message includes "/me"
        if (ev.friend_message.data.find("/me ") == 0) {
            ev.event = FRIENDACTION;
            ev.friend_action.nr = ev.friend_message.nr;
            ev.friend_action.data = ev.friend_message.data;
        }
    } else if (ev.event == FRIENDACTION) {
        // check if message already includes "/me"
        if (ev.friend_action.data.find("/me ") != 0) {
            ev.friend_action.data = "/me " + ev.friend_action.data;
        }
    }

    events.push_back(ev);

    if (ev.event == READRECEIPT) {
        auto addr = get_address(ev.readreceipt.nr);
        m_db.toxcore_log_set_received(to_hex(addr.data(), addr.size()),
                                      ev.readreceipt.data);
    } else if (ev.event == FRIENDMESSAGE) {
        auto addr = get_address(ev.friend_message.nr);
        m_db.toxcore_log_add(ToxLogRecvEntity(to_hex(addr.data(), addr.size()),
                             ELogType::LOGMSG,
                             ev.friend_message.data));
    } else if (ev.event == FRIENDACTION) {
        auto addr = get_address(ev.friend_action.nr);
        m_db.toxcore_log_add(ToxLogRecvEntity(to_hex(addr.data(), addr.size()),
                             ELogType::LOGACTION,
                             ev.friend_action.data));
    }
}

std::vector<Tox::SLog> Tox::get_log(Tox::FriendNr nr, int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }

    std::vector<Tox::SLog> result;
    auto addr = get_address(nr);
    for (auto line : m_db.toxcore_log_get(to_hex(addr.data(),
                                                 addr.size()),
                                          offset,
                                          limit)) {
        Tox::SLog n;
        n.sendtime = line.sendtime;
        n.recvtime = line.recvtime;
        n.type = (ELogType)line.type;
        n.data = line.data;

        result.push_back(n);
    }

    std::reverse(result.begin(), result.end());

    return result;
}

void Tox::callback_friend_request(Tox*,
                                  const unsigned char* addr,
                                  const unsigned char* data,
                                  unsigned short len,
                                  void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDREQUEST;
    std::copy(addr,
              addr + tmp.friend_request.addr.size(),
              tmp.friend_request.addr.begin());
    tmp.friend_request.message
        = Glib::ustring(std::string((const char*)data, len));  // no shortcut ?
    Tox::instance().inject_event(tmp);
}

void Tox::callback_friend_message(Tox*,
                                  FriendNr nr,
                                  const unsigned char* data,
                                  unsigned short len,
                                  void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDMESSAGE;
    tmp.friend_message.nr = nr;
    tmp.friend_message.data
        = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_friend_action(Tox*,
                                 FriendNr nr,
                                 const unsigned char* data,
                                 unsigned short len,
                                 void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::FRIENDACTION;
    tmp.friend_action.nr = nr;
    tmp.friend_action.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_name_change(Tox*,
                               FriendNr nr,
                               const unsigned char* data,
                               unsigned short len,
                               void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::NAMECHANGE;
    tmp.name_change.nr = nr;
    tmp.name_change.data = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_status_message(Tox*,
                                  FriendNr nr,
                                  const unsigned char* data,
                                  unsigned short len,
                                  void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::STATUSMESSAGE;
    tmp.status_message.nr = nr;
    tmp.status_message.data
        = Glib::ustring(std::string((const char*)data, len));
    Tox::instance().inject_event(tmp);
}

void Tox::callback_user_status(Tox*, FriendNr nr, unsigned char data, void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::USERSTATUS;
    tmp.user_status.nr = nr;
    tmp.user_status.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_typing_change(Tox*, FriendNr nr, unsigned char data, void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::TYPINGCHANGE;
    tmp.typing_change.nr = nr;
    tmp.typing_change.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_read_receipt(Tox*, FriendNr nr, unsigned data, void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::READRECEIPT;
    tmp.readreceipt.nr = nr;
    tmp.readreceipt.data = data;
    Tox::instance().inject_event(tmp);
}

void Tox::callback_connection_status(Tox* m,
                                     FriendNr nr,
                                     unsigned char data,
                                     void*) {
    Tox::SEvent tmp;
    tmp.event = EEventType::USERSTATUS;
    tmp.user_status.nr = nr;
    if (data == 0) {
        tmp.user_status.data = EUSERSTATUS::OFFLINE;
    } else {
        // went online get user status
        tmp.user_status.data = tox_get_user_status(m, nr);
    }
    Tox::instance().inject_event(tmp);
}

Tox::FriendAddr Tox::get_address(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }
    FriendAddr tmp;
    if (tox_get_client_id(m_tox, nr, tmp.data()) != 0) {
        throw Exception(FAILED);
    }
    return tmp;
}

Glib::ustring Tox::to_hex(const unsigned char* data, size_t len) {
    std::string s;
    for (size_t i = 0; i < len; ++i) {
        static const char hex[] = "0123456789ABCDEF";
        s += hex[(data[i] >> 4) & 0xF];
        s += hex[data[i] & 0xF];
    }
    return s;
}

std::vector<unsigned char> Tox::from_hex(std::string data) {
    std::vector<unsigned char> tmp;
    tmp.reserve(tmp.size() / 2);
    for (size_t i = 0; i < data.size(); i += 2) {
        tmp.push_back(std::stoi(data.substr(i, 2), 0, 16));  // pretty stupid
    }
    return tmp;
}
