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
#include <giomm.h>
#include "Helper/gToxFileManager.h"

Toxmm::Toxmm() {

}

void Toxmm::open(const Glib::ustring& profile_path, bool bootstrap, bool skip_profile) {
    std::vector<unsigned char> state;

    if (!skip_profile) {
        m_profile.open(profile_path);
        /* load the toxcore profile */
        if (!m_profile.can_read()) {
            throw std::runtime_error("Couldn't read toxcore profile");
        }

        state = m_profile.read();
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
    options->udp_enabled  = true;

    // load state
    if (state.empty()) {
        options->savedata_type   = TOX_SAVEDATA_TYPE_NONE;
    } else {
        options->savedata_type   = TOX_SAVEDATA_TYPE_TOX_SAVE;
        options->savedata_data   = state.data();
        options->savedata_length = state.size();
    }
    TOX_ERR_NEW error;
    m_tox = tox_new(options.get(), &error);

    if (error != TOX_ERR_NEW_OK) {
        throw Exception(error);
    }

    /* try to open the db */
    auto address = get_address();
    m_db.close();
    if (!skip_profile) {
        m_db.open(profile_path, to_hex(address.data(), TOX_PUBLIC_KEY_SIZE));
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
    tox_callback_file_chunk_request(m_tox, Toxmm::callback_file_chunk_request, this);
    tox_callback_file_recv_control(m_tox, Toxmm::callback_file_recv_control, this);

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

    m_filemanager.reset();
    if (!skip_profile && bootstrap) {
        m_filemanager = std::make_shared<gToxFileManager>(this);
        m_filemanager->init();
    }
}

Toxmm::~Toxmm() {
    m_filemanager.reset();
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

gToxFileManager& Toxmm::filemanager() {
    return *m_filemanager;
}

void Toxmm::save() {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    std::vector<unsigned char> state(tox_get_savedata_size(m_tox));
    tox_get_savedata(m_tox, (unsigned char*)state.data());
    m_profile.write(state);
    m_db.save();
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
    ToxLogEntity entity;
    entity.friendaddr = to_hex(addr.data(), addr.size());
    entity.type = EToxLogType::LOG_MESSAGE_SEND;
    entity.data = message;
    entity.receipt = res;
    m_db.toxcore_log_add(entity);

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
    ToxLogEntity entity;
    entity.friendaddr = to_hex(addr.data(), addr.size());
    entity.type = EToxLogType::LOG_ACTION_SEND;
    entity.data = action;
    entity.receipt = res;
    m_db.toxcore_log_add(entity);

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
    return fix_utf8(name.c_str(), name.size());
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

    return fix_utf8(name.c_str(), name.size());
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
    return fix_utf8(name.c_str(), name.size());
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
    return fix_utf8(name.c_str(), name.size());
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

uint64_t Toxmm::get_last_online(Toxmm::FriendNr nr) {
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

    if (ev.type() == typeid(EventReadReceipt)) {
        auto data = ev.get<EventReadReceipt>();
        auto addr = get_address(data.nr);
        m_db.toxcore_log_set_received(to_hex(addr.data(), addr.size()),
                                      data.receipt);
    } else if (ev.type() == typeid(EventFriendMessage)) {
        auto data = ev.get<EventFriendMessage>();
        auto addr = get_address(data.nr);
        ToxLogEntity entity;
        entity.type = EToxLogType::LOG_MESSAGE_RECV;
        entity.friendaddr = to_hex(addr.data(), addr.size());
        entity.data = data.message;
        m_db.toxcore_log_add(entity);
    } else if (ev.type() == typeid(EventFriendAction)) {
        auto data = ev.get<EventFriendAction>();
        auto addr = get_address(data.nr);
        ToxLogEntity entity;
        entity.type = EToxLogType::LOG_ACTION_RECV;
        entity.friendaddr = to_hex(addr.data(), addr.size());
        entity.data = data.message;
        m_db.toxcore_log_add(entity);
    } else if (ev.type() == typeid(EventFileRecv)) {
        auto data = ev.get<EventFileRecv>();
        if (data.kind == TOX_FILE_KIND::TOX_FILE_KIND_DATA) {
            auto addr = get_address(data.nr);
            auto path = database().config_get("SETTINGS_FILETRANSFER_SAVE_TO",
                                              Glib::get_user_special_dir(GUserDirectory::G_USER_DIRECTORY_DOWNLOAD));
            if (!Glib::file_test(path, Glib::FILE_TEST_IS_DIR)) {
                Gio::File::create_for_path(path)->make_directory_with_parents();
            }

            data.filename = Gio::File::create_for_path(data.filename)
                            ->get_basename();
            auto cpath = Glib::build_filename(path, data.filename);

            int index = 0;
            while (Glib::file_test(cpath, Glib::FILE_TEST_EXISTS)) {
                auto last_point = data.filename.find_last_of('.');
                if (last_point != std::string::npos) {
                    cpath = Glib::build_filename(path,
                                                 data.filename.substr(0, last_point) +
                                                 std::to_string(++index) +
                                                 data.filename.substr(last_point));
                } else {
                    cpath = Glib::build_filename(path,
                                                 data.filename +
                                                 std::to_string(++index));
                }
            }

            //TODO: CORNER CASE ! IN SAME EVENT LOOP SAME NAME WILL ALSO GENERATE SAME PATH !
            data.filepath = cpath;
            ev = ToxEvent(data);

            ToxLogEntity entity;
            entity.type = EToxLogType::LOG_FILE_RECV;
            entity.friendaddr = to_hex(addr.data(), addr.size());
            entity.data = data.filename;
            /*entity.filesize = data.file_size;
            entity.filenumber = data.file_number;
            entity.fileid = file_get_file_id(data.nr, data.file_number);*/
            //m_db.toxcore_log_add(entity);
        }
    }

    if (m_filemanager) {
        m_filemanager->observer_handle(ev);
    }

    m_events.push_back(ev);
}

std::vector<ToxLogEntity> Toxmm::get_log(Toxmm::FriendNr nr, int offset, int limit) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    auto addr = get_address(nr);
    return m_db.toxcore_log_get(to_hex(addr.data(),
                                       addr.size()),
                                offset,
                                limit);
}

void Toxmm::file_control(FriendNr nr, FileNr file_nr, TOX_FILE_CONTROL control) {
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

void Toxmm::file_seek(FriendNr nr, FileNr file_nr, uint64_t position) {
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

Toxmm::FileId Toxmm::file_get_file_id(FriendNr nr, FileNr file_nr) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    FileId id = {0};

    TOX_ERR_FILE_GET error;
    auto res = tox_file_get_file_id(m_tox, nr, file_nr, id.data(), &error);
    if (error != TOX_ERR_FILE_GET_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("file_get_field_id unknow FALSE error");
    }

    return id;
}

Toxmm::FileNr Toxmm::file_resume(FriendNr nr, FileId id) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    throw std::runtime_error("Toxmm::file_resume not implemented yet !");

    return 0;
}

Toxmm::FileNr Toxmm::file_send(FriendNr nr, TOX_FILE_KIND kind, Glib::ustring path) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS)) {
        throw std::runtime_error("FILE NOT FOUND");
    }
    auto name = Gio::File::create_for_path(path)->get_basename();
    auto info = Gio::File::create_for_path(path)->query_info();

    TOX_ERR_FILE_SEND error;
    auto res = tox_file_send(m_tox,
                             nr,
                             kind,
                             info->get_size(),
                             nullptr,
                             reinterpret_cast<const unsigned char*>(name.c_str()),
                             name.size(),
                             &error);
    if (error != TOX_ERR_FILE_SEND_OK) {
        throw Exception(error);
    }
    if (res == UINT32_MAX) {
        throw std::runtime_error("tox_file_send unknow UINT32_MAX error");
    }

    ToxLogEntity entity;
    entity.type = EToxLogType::LOG_FILE_SEND;
    auto addr = get_address(nr);
    entity.friendaddr = to_hex(addr.data(), addr.size());
    entity.data = path;
    /*entity.filesize = info->get_size();
    entity.filenumber = res;
    entity.fileid = file_get_file_id(nr, res);*/
    //m_db.toxcore_log_add(entity);

    return res;
}

void Toxmm::file_send_chunk(FriendNr nr, FileNr file_nr, uint64_t position, const std::vector<uint8_t>& data) {
    if (m_tox == nullptr) {
        throw std::runtime_error("TOX_UNITIALIZED");
    }

    TOX_ERR_FILE_SEND_CHUNK error;
    auto res = tox_file_send_chunk(m_tox,
                                   nr,
                                   file_nr,
                                   position,
                                   data.data(),
                                   data.size(),
                                   &error);
    if (error != TOX_ERR_FILE_SEND_CHUNK_OK) {
        throw Exception(error);
    }
    if (!res) {
        throw std::runtime_error("tox_file_send_chunk unknow FALSE error");
    }
}

Toxmm::Hash Toxmm::hash(const std::vector<uint8_t>& data) {
    Hash res = {0};
    tox_hash(res.data(), data.data(), data.size());
    return res;
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
    event.message = fix_utf8((const char*)data, len);
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
                                              fix_utf8((const char*)data, len)
                                          }));
}

void Toxmm::callback_friend_action(Tox*,
                                 FriendNr nr,
                                 const unsigned char* data,
                                 size_t len,
                                 void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventFriendAction{
                                              nr,
                                              fix_utf8((const char*)data, len)
                                          }));
}

void Toxmm::callback_name_change(Tox*,
                               FriendNr nr,
                               const unsigned char* data,
                               size_t len,
                               void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventName{
                                              nr,
                                              fix_utf8((const char*)data, len)
                                          }));
}

void Toxmm::callback_status_message(Tox*,
                                  FriendNr nr,
                                  const unsigned char* data,
                                  size_t len,
                                  void* _this) {
    ((Toxmm*)_this)->inject_event(ToxEvent(EventStatusMessage{
                                              nr,
                                              fix_utf8((const char*)data, len)
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
                               FileNr file_number,
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
                                     FileNr file_number,
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

void Toxmm::callback_file_recv_control(Tox*,
                                       FriendNr nr,
                                       FileNr file_number,
                                       TOX_FILE_CONTROL control,
                                       void* _this) {
    ((Toxmm*)_this)->inject_event(
                ToxEvent(EventFileControl{
                             nr,
                             file_number,
                             control
                         }));
}

void Toxmm::callback_file_chunk_request(Tox*,
                                        FriendNr nr,
                                        FileNr file_number,
                                        uint64_t position,
                                        size_t size,
                                        void* _this) {
    ((Toxmm*)_this)->inject_event(
                ToxEvent(EventFileSendChunkRequest{
                             nr,
                             file_number,
                             position,
                             size
                         }));
}

Glib::ustring Toxmm::fix_utf8(const char* input, int size) {
    static const Glib::ustring uFFFD(1, gunichar(0xFFFD));
    std::string fixed;
    fixed.reserve(size);
    const gchar* str = input;
    const gchar* last_valid;
    while(!g_utf8_validate(str, std::distance(str, input + size), &last_valid)) {
        fixed.append(str, last_valid);
        fixed.append(uFFFD.raw().begin(), uFFFD.raw().end());
        str = last_valid + 1;
    }
    fixed.append(str, last_valid);
    return fixed;
}
