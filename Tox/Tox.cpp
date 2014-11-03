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
    options.proxy_enabled = false;
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

    m_db.reset();
    // load state
    if (statefile != "") {
        /* try to open the db */
        m_db = std::make_shared<SQLite::Database>(statefile,
                                                  SQLITE_OPEN_READWRITE);

        m_db->exec("SAVEPOINT try_load");
        try {

            // ceck version of the db
            int version
                = m_db->execAndGet(
                            "SELECT value FROM config WHERE name='version'")
                      .getInt();

            while (version != 3 /*current version*/)
                switch (version) {
                    case 1:
                        m_db->exec(DATABASE::version_2);
                        version = 2;
                        break;
                    case 2:
                        m_db->exec(DATABASE::version_3);
                        version = 3;
                        break;
                    default:
                        throw Exception(UNKNOWDBVERSION);
                        break;
                }

            // increase runid by 1
            m_db->exec(
                "UPDATE config"
                " SET value = value + 1"
                " WHERE name='runid'");

            // clean up toxcore
            try {
                SQLite::Statement storeq(*m_db,
                                         "DELETE FROM toxcore"
                                         " WHERE id < ?1");
                storeq.bind(
                    1,
                    m_db->execAndGet(
                              "SELECT max(id), date(savetime) FROM toxcore "
                              "GROUP BY "
                              "date(savetime) ORDER BY id DESC LIMIT 7,1")
                        .getInt());
                storeq.exec();
            } catch (...) {
                // nothing to remove
            }

            // take the last saved state
            auto col = m_db->execAndGet(
                "SELECT state FROM toxcore"
                " ORDER BY id DESC LIMIT 1");
            auto state = (const unsigned char*)col.getBlob();
            size_t state_size = col.getBytes();

            if (tox_load(m_tox, state, state_size) == -1) {
                throw Exception(LOADERROR);
            }

            m_db->exec("RELEASE SAVEPOINT try_load");
        } catch (...) {

            // restore old state
            m_db->exec("ROLLBACK TO SAVEPOINT try_load");

            throw;
        }
    }

    SQLite::Statement stmt(*m_db,
                           "SELECT active, ip, port, pub_key"
                           " FROM bootstrap");
    bool okay = false;
    while (stmt.executeStep()) {
        int active = stmt.getColumn(0).getInt();
        if (active == 0) {
            continue;
        }

        std::string ip = stmt.getColumn(1).getText("");
        int port = stmt.getColumn(2).getInt();
        std::string pub_key = stmt.getColumn(3).getText("");

        if (pub_key.size() == 32) {
            auto pub = from_hex(pub_key);
            okay |= tox_bootstrap_from_address(
                m_tox, ip.c_str(), port, pub.data());
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

    if (!m_db) {
        // first time saving create database
        m_db = std::make_shared<SQLite::Database>(
            statefile, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        m_db->exec(DATABASE::version_1);
        m_db->exec(DATABASE::version_2);
    }

    // store state
    int length = (int)tox_size(m_tox);
    std::vector<unsigned char> state(length);
    tox_save(m_tox, (unsigned char*)state.data());

    m_db->exec("BEGIN TRANSACTION");
    try {
        SQLite::Statement removq(*m_db, "DELETE FROM toxcore WHERE runid=?1");
        SQLite::Statement storeq(*m_db,
                                 "INSERT INTO toxcore"
                                 " (savetime, state, runid)"
                                 " VALUES (CURRENT_TIMESTAMP, ?1, ?2)");

        auto runid = m_db->execAndGet(
                               "SELECT value FROM config"
                               " WHERE name='runid' LIMIT 1").getInt();

        removq.bind(1, runid);

        storeq.bind(1, state.data(), state.size());
        storeq.bind(2, runid);

        removq.exec();
        storeq.exec();
    } catch (...) {
        m_db->exec("ROLLBACK TRANSACTION");
        throw;
    }

    m_db->exec("COMMIT TRANSACTION");
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

    if (m_db) {
        SQLite::Statement storeq(
            *m_db,
            "INSERT INTO log(friendaddr, sendtime, type, message, receipt)"
            " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)");

        storeq.bind(1, get_address(nr).data(), TOX_CLIENT_ID_SIZE);
        storeq.bind(2, ELogType::LOGMSG);
        storeq.bind(3, message.data(), message.bytes());
        storeq.bind(4, res);

        storeq.exec();
    }

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

    if (m_db) {
        SQLite::Statement storeq(
            *m_db,
            "INSERT INTO log(friendaddr, sendtime, type, message, receipt)"
            " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)");

        storeq.bind(1, get_address(nr).data(), TOX_CLIENT_ID_SIZE);
        storeq.bind(2, ELogType::LOGACTION);
        storeq.bind(3, action.data(), action.bytes());
        storeq.bind(4, res);

        storeq.exec();
    }

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

    if (m_db) {
        if (ev.event == READRECEIPT) {
            SQLite::Statement updateq(
                *m_db,
                "UPDATE log SET recvtime=CURRENT_TIMESTAMP "
                "WHERE friendaddr=?1 AND receipt=?2");

            updateq.bind(
                1, get_address(ev.readreceipt.nr).data(), TOX_CLIENT_ID_SIZE);
            updateq.bind(2, ev.readreceipt.data);

            updateq.exec();
        } else if (ev.event == FRIENDMESSAGE) {
            SQLite::Statement storeq(
                *m_db,
                "INSERT INTO log(friendaddr, recvtime, type, message)"
                " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3)");

            storeq.bind(1,
                        get_address(ev.friend_message.nr).data(),
                        TOX_CLIENT_ID_SIZE);
            storeq.bind(2, ELogType::LOGMSG);
            storeq.bind(3,
                        ev.friend_message.data.data(),
                        ev.friend_message.data.bytes());

            storeq.exec();
        } else if (ev.event == FRIENDACTION) {
            SQLite::Statement storeq(
                *m_db,
                "INSERT INTO log(friendaddr, recvtime, type, message)"
                " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3)");

            storeq.bind(
                1, get_address(ev.friend_action.nr).data(), TOX_CLIENT_ID_SIZE);
            storeq.bind(2, ELogType::LOGACTION);
            storeq.bind(
                3, ev.friend_action.data.data(), ev.friend_action.data.bytes());

            storeq.exec();
        }
    }
}

std::vector<Tox::SLog> Tox::get_log(Tox::FriendNr nr, int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lg(m_mtx);
    if (m_tox == nullptr) {
        throw Exception(UNITIALIZED);
    }

    std::vector<Tox::SLog> result;

    if (m_db) {
        SQLite::Statement loadq(
            *m_db,
            "SELECT"
            " strftime('%s', sendtime),"
            " strftime('%s', recvtime),"
            " type, message FROM log"
            " WHERE friendaddr = ?1 ORDER BY id DESC LIMIT ?2, ?3");

        loadq.bind(1, get_address(nr).data(), TOX_CLIENT_ID_SIZE);
        loadq.bind(2, offset);
        loadq.bind(3, limit);

        while (loadq.executeStep()) {
            Tox::SLog n;
            n.sendtime = loadq.getColumn(0).getInt64();
            n.recvtime = loadq.getColumn(1).getInt64();
            n.type = (ELogType)loadq.getColumn(2).getInt();
            auto data = loadq.getColumn(3);
            auto data_ptr = (const char*)data.getBlob();
            n.data = Glib::ustring(data_ptr, data_ptr + data.getBytes());

            result.push_back(n);
        }
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
