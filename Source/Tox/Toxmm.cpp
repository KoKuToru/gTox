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
#include "Toxmm.h"
#include <fstream>
#include <string>
#include <algorithm>
#include "Generated/database.h"

Toxmm::Toxmm() {

}

void Toxmm::open(const Glib::ustring& profile_path, bool bootstrap) {
    m_profile.open(profile_path);
    /* load the toxcore profile */
    if (!m_profile.can_read()) {
        throw std::runtime_error("Couldn't read toxcore profile");
    }
    std::vector<unsigned char> state = m_profile.read();
    /* try to open the db */
    m_db.open(profile_path);

    TOX_ERR_OPTIONS_NEW nerror;
    auto options = std::shared_ptr<Tox_Options>(tox_options_new(&nerror),
                                                [](Tox_Options* p) {
                                                    tox_options_free(p);
                                                });
    if (nerror != TOX_ERR_OPTIONS_NEW_OK) {
        throw Exception(nerror);
    }

    options->ipv6_enabled = true;
    options->udp_enabled  = true;

    // load state
    TOX_ERR_NEW error;
    m_tox = tox_new(options.get(), state.data(), state.size(), &error);

    if (error != TOX_ERR_NEW_OK) {
        throw Exception(error);
    }

    // install callbacks
    tox_callback_friend_request(m_tox, Toxmm::callback_friend_request, this);
    tox_callback_friend_message(m_tox, Toxmm::callback_friend_message, this);
    tox_callback_friend_name(m_tox, Toxmm::callback_name_change, this);
    tox_callback_friend_status_message(m_tox, Toxmm::callback_status_message, this);
    tox_callback_friend_status(m_tox, Toxmm::callback_user_status, this);
    tox_callback_friend_typing(m_tox, Toxmm::callback_typing_change, this);
    tox_callback_friend_read_receipt(m_tox, Toxmm::callback_read_receipt, this);
    tox_callback_friend_connection_status(m_tox, Toxmm::callback_connection_status, this);
    tox_callback_file_recv(m_tox, Toxmm::callback_file_recv, this);
    tox_callback_file_recv_chunk(m_tox, Toxmm::callback_file_recv_chunk, this);

    if (bootstrap) {
        bool okay = true;

        if (profile_path != "") {
            for (auto boots : m_db.toxcore_bootstrap_get()) {
                if (boots.pub_key.size()%2 == 0) {
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
}

Toxmm::~Toxmm() {
    if (m_tox != nullptr) {
        tox_kill(m_tox);
        m_tox = nullptr;
    }
}

ToxDatabase& Toxmm::database() {
    return m_db;
}

ToxProfile& Toxmm::profile() {
    return m_profile;
}

void Toxmm::save() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    std::vector<unsigned char> state(tox_get_savedata_size(m_tox));
    tox_get_savedata(m_tox, (unsigned char*)state.data());
    m_profile.write(state);
}

int Toxmm::update_optimal_interval() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    return tox_iteration_interval(m_tox);
}

bool Toxmm::update(ToxEvent& ev) {
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

std::vector<Toxmm::FriendNr> Toxmm::get_friendlist() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    std::vector<FriendNr> tmp(tox_self_get_friend_list_size(m_tox));
    tox_self_get_friend_list(m_tox, tmp.data());
    return tmp;
}

Toxmm::FriendAddr Toxmm::get_address() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    FriendAddr tmp;
    tox_self_get_address(m_tox, tmp.data());
    return tmp;
}

Toxmm::FriendNr Toxmm::add_friend(Toxmm::FriendAddr addr,
                              const Glib::ustring& message) {
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

Toxmm::FriendNr Toxmm::add_friend_norequest(FriendAddr addr) {
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

Toxmm::FriendNr Toxmm::get_friend_number(Toxmm::FriendAddr addr) {
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

void Toxmm::del_friend(Toxmm::FriendNr nr) {
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

Toxmm::ReceiptNr Toxmm::send_message(Toxmm::FriendNr nr,
                                 const Glib::ustring& message) {
    if (message.find("/me ") == 0) {
        return send_action(nr, message);
    }

    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    Toxmm::ReceiptNr res = tox_friend_send_message(
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

Toxmm::ReceiptNr Toxmm::send_action(Toxmm::FriendNr nr, const Glib::ustring& action) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    Toxmm::ReceiptNr res = tox_friend_send_message(
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

void Toxmm::set_name(const Glib::ustring& name) {
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

Glib::ustring Toxmm::get_name() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    std::string name(tox_self_get_name_size(m_tox), 0);
    tox_self_get_name(m_tox, (unsigned char*)(name.data()));
    return name;
}

Glib::ustring Toxmm::get_name(Toxmm::FriendNr nr) {
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

Glib::ustring Toxmm::get_name_or_address() {
    Glib::ustring tmp = get_name();
    if (tmp.empty()) {
        return to_hex(get_address().data(), 32);
    }
    return tmp;
}

Glib::ustring Toxmm::get_name_or_address(Toxmm::FriendNr nr) {
    Glib::ustring tmp = get_name(nr);
    if (tmp.empty()) {
        return to_hex(get_address(nr).data(), 32);
    }
    return tmp;
}

Glib::ustring Toxmm::get_status_message() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    auto size = tox_self_get_status_message_size(m_tox);
    std::string name(size, 0);
    tox_self_get_status_message(m_tox, (unsigned char*)name.data());
    return name;
}

Glib::ustring Toxmm::get_status_message(FriendNr nr) {
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

void Toxmm::set_status_message(Glib::ustring msg) {
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

bool Toxmm::is_connected() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    return tox_self_get_connection_status(m_tox) != TOX_CONNECTION_NONE;
}

Toxmm::EUSERSTATUS Toxmm::get_status() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    auto con = tox_self_get_connection_status(m_tox);
    if (con == TOX_CONNECTION_NONE) {
        return EUSERSTATUS::OFFLINE;
    }

    return (EUSERSTATUS)tox_self_get_status(m_tox);
}

Toxmm::EUSERSTATUS Toxmm::get_status(FriendNr nr) {
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

void Toxmm::set_status(Toxmm::EUSERSTATUS value) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }
    if (value == Toxmm::OFFLINE)
        value = Toxmm::AWAY;  // we can't set status to offline
    tox_self_set_status(m_tox, (TOX_USER_STATUS)value);
}

unsigned long long Toxmm::get_last_online(Toxmm::FriendNr nr) {
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

void Toxmm::send_typing(FriendNr nr, bool is_typing) {
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

void Toxmm::inject_event(ToxEvent ev) {
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

std::vector<Toxmm::SLog> Toxmm::get_log(Toxmm::FriendNr nr, int offset, int limit) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    std::vector<Toxmm::SLog> result;
    auto addr = get_address(nr);
    for (auto line : m_db.toxcore_log_get(to_hex(addr.data(),
                                                 addr.size()),
                                          offset,
                                          limit)) {
        Toxmm::SLog n;
        n.sendtime = line.sendtime;
        n.recvtime = line.recvtime;
        n.type = (ELogType)line.type;
        n.data = line.data;

        result.push_back(n);
    }

    std::reverse(result.begin(), result.end());

    return result;
}

void Toxmm::file_control(FriendNr nr, uint32_t file_nr, TOX_FILE_CONTROL control) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FILE_CONTROL error;
    auto res = tox_file_control(m_tox, nr, file_nr, control, &error);
    if (error != TOX_ERR_FILE_CONTROL_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_file_control unknow FALSE error");
    }
}

void Toxmm::file_seek(FriendNr nr, uint32_t file_nr, uint64_t position) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FILE_SEEK error;
    auto res = tox_file_seek(m_tox, nr, file_nr, position, &error);
    if (error != TOX_ERR_FILE_SEEK_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_file_seek unknow FALSE error");
    }
}

void Toxmm::callback_friend_request(Tox*,
                                  const unsigned char* addr,
                                  const unsigned char* data,
                                  size_t len,
                                  void* _this) {
    EventFriendRequest event;
    std::copy(addr,
              addr + event.addr.size(),
              event.addr.begin());
    event.message = std::string((const char*)data, len);
    ((Toxmm*)_this)->inject_event(ToxEvent(event));
}

void Toxmm::callback_friend_message(Tox*,
                                  FriendNr nr,
                                  TOX_MESSAGE_TYPE type,
                                  const unsigned char* data,
                                  size_t len,
                                  void* _this) {
    if (type == TOX_MESSAGE_TYPE::TOX_MESSAGE_TYPE_ACTION) {
        callback_friend_action(nullptr, nr, data, len, nullptr);
        return;
    }
    ((Toxmm*)_this)->inject_event(ToxEvent(EventFriendMessage{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Toxmm::callback_friend_action(Tox*,
                                 FriendNr nr,
                                 const unsigned char* data,
                                 size_t len,
                                 void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventFriendAction{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Toxmm::callback_name_change(Tox*,
                               FriendNr nr,
                               const unsigned char* data,
                               size_t len,
                               void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventName{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Toxmm::callback_status_message(Tox*,
                                  FriendNr nr,
                                  const unsigned char* data,
                                  size_t len,
                                  void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventStatusMessage{
                                              nr,
                                              std::string((const char*)data, len)
                                          }));
}

void Toxmm::callback_user_status(Tox*, FriendNr nr, TOX_USER_STATUS, void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventUserStatus{
                                              nr,
                                              ((Toxmm*)_this)->get_status(nr)
                                          }));
}

void Toxmm::callback_typing_change(Tox*, FriendNr nr, bool data, void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventTyping{
                                              nr,
                                              data
                                          }));
}

void Toxmm::callback_read_receipt(Tox*, FriendNr nr, unsigned data, void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventReadReceipt{
                                              nr,
                                              data
                                          }));
}

void Toxmm::callback_connection_status(Tox*,
                                     FriendNr nr,
                                     TOX_CONNECTION,
                                     void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventUserStatus{
                                              nr,
                                              ((Toxmm*)_this)->get_status(nr)
                                          }));
}

Toxmm::FriendAddr Toxmm::get_address(Toxmm::FriendNr nr) {
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

Glib::ustring Toxmm::to_hex(const unsigned char* data, size_t len) {
    std::string s;
    for (size_t i = 0; i < len; ++i) {
        static const char hex[] = "0123456789ABCDEF";
        s += hex[(data[i] >> 4) & 0xF];
        s += hex[data[i] & 0xF];
    }
    return s;
}

std::vector<unsigned char> Toxmm::from_hex(std::string data) {
    std::vector<unsigned char> tmp;
    tmp.reserve(tmp.size() / 2);
    for (size_t i = 0; i < data.size(); i += 2) {
        tmp.push_back(std::stoi(data.substr(i, 2), 0, 16));  // pretty stupid
    }
    return tmp;
}

void Toxmm::callback_file_recv(Tox*,
                               FriendNr nr,
                               uint32_t file_number,
                               uint32_t kind,
                               uint64_t file_size,
                               const unsigned char* filename,
                               size_t filename_length,
                               void* _this) {
    ((Toxmm*)_this)->inject_event(
                ToxEvent(EventFileRecv{
                             nr,
                             file_number,
                             TOX_FILE_KIND(kind),
                             file_size,
                             std::string((const char*)filename, filename_length)
                         }));
}

void Toxmm::callback_file_recv_chunk(Tox*,
                                     Toxmm::FriendNr nr,
                                     uint32_t file_number,
                                     uint64_t position,
                                     const unsigned char* data,
                                     size_t data_length,
                                     void* _this) {
    ((Toxmm*)_this)->inject_event(
                ToxEvent(EventFileRecvChunk{
                             nr,
                             file_number,
                             position,
                             std::vector<unsigned char>(data, data+data_length)
                         }));
}
