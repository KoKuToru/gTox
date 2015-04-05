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
    TOX_ERR_OPTIONS_NEW nerror;
    auto options = std::shared_ptr<Tox_Options>(tox_options_new(&nerror),
                                                [](Tox_Options* p) {
                                                    tox_options_free(p);
                                                });
    if (nerror != TOX_ERR_OPTIONS_NEW_OK) {
        throw Exception(nerror);
    }

    options->ipv6_enabled = true;
    options->udp_enabled = true;

    m_db.close();
    // load state
    std::vector<unsigned char> state;
    bool okay = false;
    if (statefile != "") {
        /* try to open the db */
        m_db.open(statefile);

        // take the last saved state
        state = m_db.toxcore_state_get(0);
    }
    TOX_ERR_NEW error;
    m_tox = tox_new(options.get(), state.data(), state.size(), &error);

    if (error != TOX_ERR_NEW_OK) {
        throw Exception(error);
    }

    // install callbacks
    tox_callback_friend_request(m_tox, Tox::callback_friend_request, nullptr);
    tox_callback_friend_message(m_tox, Tox::callback_friend_message, nullptr);
    tox_callback_friend_name(m_tox, Tox::callback_name_change, nullptr);
    tox_callback_friend_status_message(m_tox, Tox::callback_status_message, nullptr);
    tox_callback_friend_status(m_tox, Tox::callback_user_status, nullptr);
    tox_callback_friend_typing(m_tox, Tox::callback_typing_change, nullptr);
    tox_callback_friend_read_receipt(m_tox, Tox::callback_read_receipt, nullptr);
    tox_callback_friend_connection_status(m_tox, Tox::callback_connection_status, nullptr);

    if (statefile != "") {
        for (auto boots : m_db.toxcore_bootstrap_get()) {
            if (boots.pub_key.size() == 32) {
                auto pub = from_hex(boots.pub_key);
                TOX_ERR_BOOTSTRAP error;
                okay |= tox_bootstrap(m_tox,
                                      boots.ip.c_str(),
                                      boots.port, pub.data(), &error);
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
        TOX_ERR_BOOTSTRAP error;
        if (!tox_bootstrap(m_tox, host, 33445, pub.data(), &error)) {
            throw Exception(error);
        }
    }
}

void Tox::save(const Glib::ustring& statefile) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    std::vector<unsigned char> state(tox_get_savedata_size(m_tox));
    tox_get_savedata(m_tox, (unsigned char*)state.data());
    m_db.toxcore_state_add(state);

    if (statefile != "") {
        m_db.move(statefile);
    }
}

int Tox::update_optimal_interval() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    return tox_iteration_interval(m_tox);
}

bool Tox::update(ToxEvent& ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    tox_iterate(m_tox);
    if (m_events.empty()) {
        return false;
    }
    ev = m_events.front();
    m_events.pop_front();
    return true;
}

std::vector<Tox::FriendNr> Tox::get_friendlist() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    std::vector<FriendNr> tmp(tox_self_get_friend_list_size(m_tox));
    tox_self_get_friend_list(m_tox, tmp.data());
    return tmp;
}

Tox::FriendAddr Tox::get_address() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    FriendAddr tmp;
    tox_self_get_address(m_tox, tmp.data());
    return tmp;
}

Tox::FriendNr Tox::add_friend(Tox::FriendAddr addr,
                              const Glib::ustring& message) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_ADD error;
    FriendNr res
        = tox_friend_add(m_tox,
                         addr.data(),
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.bytes(),
                         &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw Exception(error);
    }
    if (res == UINT32_MAX) {
        throw std::runtime_error("tox_friend_add unknow UINIT32_MAX error");
    }
    return res;
}

Tox::FriendNr Tox::add_friend_norequest(FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_ADD error;
    FriendNr res = tox_friend_add_norequest(m_tox, addr.data(), &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw Exception(error);
    }
    if (res == UINT32_MAX) {
        throw std::runtime_error("tox_friend_add_norequest unknow UINIT32_MAX error");
    }
    return res;
}

Tox::FriendNr Tox::get_friend_number(Tox::FriendAddr addr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_BY_PUBLIC_KEY error;
    FriendNr res = tox_friend_by_public_key(m_tox, addr.data(), &error);
    if (error != TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK) {
        throw Exception(error);
    }
    if (res == UINT32_MAX) {
        throw std::runtime_error("tox_friend_by_public_key unknow UINT32_MAX error");
    }
    return res;
}

void Tox::del_friend(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_DELETE error;
    auto res = tox_friend_delete(m_tox, nr, &error);
    if (error != TOX_ERR_FRIEND_DELETE_OK) {
        throw Exception(error);
    }

    if (!res) {
        throw std::runtime_error("tox_friend_delete unknow FALSE error");
    }
}

Tox::ReceiptNr Tox::send_message(Tox::FriendNr nr,
                                 const Glib::ustring& message) {
    if (message.find("/me ") == 0) {
        return send_action(nr, message);
    }

    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    Tox::ReceiptNr res = tox_friend_send_message(
        m_tox,
        nr,
        TOX_MESSAGE_TYPE::TOX_MESSAGE_TYPE_NORMAL,
        reinterpret_cast<const unsigned char*>(message.data()),
        message.bytes(),
        &error);

    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw Exception(error);
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
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    Tox::ReceiptNr res = tox_friend_send_message(
        m_tox,
        nr,
        TOX_MESSAGE_TYPE::TOX_MESSAGE_TYPE_ACTION,
        reinterpret_cast<const unsigned char*>(action.data()),
        action.bytes(),
        &error);

    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        throw Exception(error);
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
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_SET_INFO error;
    auto res = tox_self_set_name(m_tox,
                                 reinterpret_cast<const unsigned char*>(name.data()),
                                 name.bytes(),
                                 &error);

    if (error != TOX_ERR_SET_INFO_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_self_set_name unknow FALSE error");
    }
}

Glib::ustring Tox::get_name() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    std::string name(tox_self_get_name_size(m_tox), 0);
    tox_self_get_name(m_tox, (unsigned char*)(name.data()));
    return name;
}

Glib::ustring Tox::get_name(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FRIEND_QUERY error;
    auto size = tox_friend_get_name_size(m_tox, nr, &error);

    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }
    if (size == SIZE_MAX) {
        throw std::runtime_error("tox_friend_get_name_size unknow SIZE_MAX error");
    }

    std::string name(size, 0);
    auto res = tox_friend_get_name(m_tox, nr, (unsigned char*)(name.data()), &error);

    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_friend_get_name unknow FALSE error");
    }

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
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    auto size = tox_self_get_status_message_size(m_tox);
    std::string name(size, 0);
    tox_self_get_status_message(m_tox, (unsigned char*)name.data());
    return name;
}

Glib::ustring Tox::get_status_message(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_QUERY error;
    auto size = tox_friend_get_status_message_size(m_tox, nr, &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }
    std::string name(size, 0);
    tox_friend_get_status_message(m_tox, nr, (unsigned char*)name.data(), &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }
    return name;
}

void Tox::set_status_message(Glib::ustring msg) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_SET_INFO error;
    auto res = tox_self_set_status_message(m_tox,
                                           reinterpret_cast<const unsigned char*>(msg.data()),
                                           msg.bytes(),
                                           &error);
    if (error != TOX_ERR_SET_INFO_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_self_set_status_message unknow FALSE error");
    }
}

bool Tox::is_connected() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    return tox_self_get_connection_status(m_tox) != TOX_CONNECTION_NONE;
}

Tox::EUSERSTATUS Tox::get_status() {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    auto con = tox_self_get_connection_status(m_tox);
    if (con == TOX_CONNECTION_NONE) {
        return EUSERSTATUS::OFFLINE;
    }

    return (EUSERSTATUS)tox_self_get_status(m_tox);
}

Tox::EUSERSTATUS Tox::get_status(FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_QUERY error;
    auto con = tox_friend_get_connection_status(m_tox, nr, &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }
    if (con == TOX_CONNECTION_NONE) {
        return EUSERSTATUS::OFFLINE;
    }

    auto status = (EUSERSTATUS)tox_friend_get_status(m_tox, nr, &error);
    if (error != TOX_ERR_FRIEND_QUERY_OK) {
        throw Exception(error);
    }

    return status;
}

void Tox::set_status(Tox::EUSERSTATUS value) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    if (value == Tox::OFFLINE)
        value = Tox::AWAY;  // we can't set status to offline
    tox_self_set_status(m_tox, (TOX_USER_STATUS)value);
}

unsigned long long Tox::get_last_online(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_FRIEND_GET_LAST_ONLINE error;
    auto res = tox_friend_get_last_online(m_tox, nr, &error);
    if (error != TOX_ERR_FRIEND_GET_LAST_ONLINE_OK) {
        throw Exception(error);
    }
    return res;
}

void Tox::send_typing(FriendNr nr, bool is_typing) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    TOX_ERR_SET_TYPING error;
    auto res = tox_self_set_typing(m_tox, nr, is_typing, &error);
    if (error != TOX_ERR_SET_TYPING_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_self_set_typing unknow FALSE error");
    }
}

void Tox::inject_event(ToxEvent ev) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    //FIX /me PROBLEM
    if (ev.type() == typeid(EventFriendMessage)) {
        // check if message includes "/me"
        auto data = ev.get<EventFriendMessage>();
        if (data.message.find("/me ") == 0) {
            ev = ToxEvent(EventFriendAction{data.nr, data.message});
        }
    } else if (ev.type() == typeid(EventFriendAction)) {
        // check if message already includes "/me"
        auto data = ev.get<EventFriendAction>();
        if (data.message.find("/me ") != 0) {
            data.message = "/me " + data.message;
        }
    }

    m_events.push_back(ev);

    if (ev.type() == typeid(EventReadReceipt)) {
        auto data = ev.get<EventReadReceipt>();
        auto addr = get_address(data.nr);
        m_db.toxcore_log_set_received(to_hex(addr.data(), addr.size()),
                                      data.receipt);
    } else if (ev.type() == typeid(EventFriendMessage)) {
        auto data = ev.get<EventFriendMessage>();
        auto addr = get_address(data.nr);
        m_db.toxcore_log_add(ToxLogRecvEntity(to_hex(addr.data(), addr.size()),
                             ELogType::LOGMSG,
                             data.message));
    } else if (ev.type() == typeid(EventFriendAction)) {
        auto data = ev.get<EventFriendAction>();
        auto addr = get_address(data.nr);
        m_db.toxcore_log_add(ToxLogRecvEntity(to_hex(addr.data(), addr.size()),
                             ELogType::LOGACTION,
                             data.message));
    }
}

std::vector<Tox::SLog> Tox::get_log(Tox::FriendNr nr, int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
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
                                  size_t len,
                                  void*) {
    EventFriendRequest event;
    std::copy(addr,
              addr + event.addr.size(),
              event.addr.begin());
    event.message = std::string((const char*)data, len);
    Tox::instance().inject_event(ToxEvent(event));
}

void Tox::callback_friend_message(Tox*,
                                  FriendNr nr,
                                  TOX_MESSAGE_TYPE type,
                                  const unsigned char* data,
                                  size_t len,
                                  void*) {
    if (type == TOX_MESSAGE_TYPE::TOX_MESSAGE_TYPE_ACTION) {
        callback_friend_action(nullptr, nr, data, len, nullptr);
        return;
    }
    Tox::instance().inject_event(ToxEvent(EventFriendMessage{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Tox::callback_friend_action(Tox*,
                                 FriendNr nr,
                                 const unsigned char* data,
                                 size_t len,
                                 void*) {
    Tox::instance().inject_event(ToxEvent(EventFriendAction{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Tox::callback_name_change(Tox*,
                               FriendNr nr,
                               const unsigned char* data,
                               size_t len,
                               void*) {
    Tox::instance().inject_event(ToxEvent(EventName{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Tox::callback_status_message(Tox*,
                                  FriendNr nr,
                                  const unsigned char* data,
                                  size_t len,
                                  void*) {
    Tox::instance().inject_event(ToxEvent(EventStatusMessage{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Tox::callback_user_status(Tox*, FriendNr nr, TOX_USER_STATUS, void*) {
    Tox::instance().inject_event(ToxEvent(EventUserStatus{
                                              nr,
                                              Tox::instance().get_status(nr)
                                          }));
}

void Tox::callback_typing_change(Tox*, FriendNr nr, bool data, void*) {
    Tox::instance().inject_event(ToxEvent(EventTyping{
                                              nr,
                                              data
                                          }));
}

void Tox::callback_read_receipt(Tox*, FriendNr nr, unsigned data, void*) {
    Tox::instance().inject_event(ToxEvent(EventReadReceipt{
                                              nr,
                                              data
                                          }));
}

void Tox::callback_connection_status(Tox*,
                                     FriendNr nr,
                                     TOX_CONNECTION,
                                     void*) {
    Tox::instance().inject_event(ToxEvent(EventUserStatus{
                                              nr,
                                              Tox::instance().get_status(nr)
                                          }));
}

Tox::FriendAddr Tox::get_address(Tox::FriendNr nr) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    FriendAddr tmp;
    TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
    auto res = tox_friend_get_public_key(m_tox, nr, tmp.data(), &error);
    if (error != TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_friend_get_public_key unknow FALSE error");
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
