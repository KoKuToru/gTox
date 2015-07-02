/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "ToxDatabase.h"
#include <giomm/file.h>
#include <glibmm/fileutils.h>
#include "Generated/database.h"
#include <tox/tox.h>
#include <fstream>
#include <exception>
#include <glibmm.h>
#include <glib.h>
#include "Helper/gToxFileManager.h"
#include <iostream>

void ToxDatabase::open(const std::string& path, const std::string& address, bool init) {
    if (m_db) {
        throw std::runtime_error("Database already open");
    }

    //TODO: 28.07.2015 START Remove after some weeks
    //old path:
    m_path_db    = path;
    std::string state_ext = ".tox";
    bool f_tox = (m_path_db.size() > state_ext.size()
                  && m_path_db.substr(m_path_db.size() - state_ext.size(),
                                      state_ext.size()) == state_ext);
    if (f_tox) {
        m_path_db = m_path_db.substr(0, m_path_db.size() - 4);
    }
    m_path_db += ".gtox";
    //TODO: 28.07.2015 END Remove after some weeks

    //New path:
    auto npath = Glib::build_filename(Glib::path_get_dirname(path), "gtox");
    if (!Glib::file_test(npath, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(npath)->make_directory_with_parents();
    }
    npath = Glib::build_filename(npath, address + ".sqlite");
    if (address.empty()) {
        npath = path;
    }

    //TODO: 28.07.2015 START Remove after some weeks
    //move old path to new path
    if (Glib::file_test(m_path_db, Glib::FILE_TEST_IS_REGULAR)) {
        Gio::File::create_for_path(m_path_db)
                            ->move(Gio::File::create_for_path(npath));
    }
    //TODO: 28.07.2015 END Remove after some weeks
    m_path_db = npath;

    if (!Glib::file_test(m_path_db, Glib::FILE_TEST_IS_REGULAR)) {
        //force init
        init = true;
    }

    m_db = std::make_shared<SQLite::Database>(
        m_path_db, SQLITE_OPEN_READWRITE | (init ? SQLITE_OPEN_CREATE : 0));

    if (init) {
        m_db->exec(DATABASE::version_1);
    }

    // check for updates
    m_db->exec("SAVEPOINT try_load");
    try {

        auto version = config_get("version", 0);

        std::string* upgrade_scripts[]
            = {&DATABASE::version_2,
               &DATABASE::version_3,
               &DATABASE::version_4,
               &DATABASE::version_5,
               &DATABASE::version_6,
               &DATABASE::version_7,
               &DATABASE::version_8,
               &DATABASE::version_9,
               &DATABASE::version_10,
               &DATABASE::version_11,
               &DATABASE::version_12};

        if (version < 1 || version > (int)sizeof(upgrade_scripts)) {
            throw std::runtime_error("Database not open !");
        }

        int version_max
            = 1 + sizeof(upgrade_scripts) / sizeof(upgrade_scripts[0]);

        for (; version < version_max; ++version) {
            std::clog << "Upgrad to version " << (version + 1) << std::endl;
            m_db->exec(*upgrade_scripts[version - 1]);
            config_set("version", version + 1);
        }

        m_db->exec("RELEASE SAVEPOINT try_load");
    } catch (...) {
        // restore old state
        m_db->exec("ROLLBACK TO SAVEPOINT try_load");

        throw;
    }

    // increase runid
    m_runid = config_get("runid", 0) + 1;
    config_set("runid", m_runid);

    //attach in memory database
    m_db->exec("ATTACH DATABASE ':memory:' AS mem");
    m_db->exec("CREATE TABLE mem.log AS SELECT * FROM log WHERE 0");
    m_db->exec("CREATE TABLE mem.file AS SELECT * FROM file WHERE 0");
}

void ToxDatabase::close() {
    m_db.reset();
}

void ToxDatabase::save() {
    //does nothing
}

std::string ToxDatabase::config_get(const std::string& name,
                                    const std::string& value = "") {
    if (!m_db) {
        throw std::runtime_error("Database not open !");
    }

    auto selectq
        = query("SELECT value FROM config WHERE cast(name as text)=cast(?1 as text) LIMIT 1", name);

    if (selectq->executeStep()) {
        return selectq->getColumn(0).getText(value.c_str());
    }

    return value;
}

void ToxDatabase::config_set(const std::string& name,
                             const std::string& value) {
    if (!m_db) {
        throw std::runtime_error("Database not open !");
    }

    if (query("UPDATE config SET value=?2 WHERE cast(name as text)=cast(?1 as text)", name, value)->exec()
        < 1) {
        query("INSERT INTO config(name, value) VALUES (?1, ?2)", name, value)
            ->exec();
    }
}

int ToxDatabase::config_get(const std::string& name, int value = 0) {
    if (!m_db) {
        throw std::runtime_error("Database not open !");
    }

    auto selectq
        = query("SELECT value FROM config WHERE cast(name as text)=cast(?1 as text) LIMIT 1", name);

    if (selectq->executeStep()) {
        if (selectq->getColumn(0).isNull()) {
            return value;
        }
        return selectq->getColumn(0).getInt();
    }

    return value;
}

void ToxDatabase::config_set(const std::string& name, int value) {
    if (!m_db) {
        throw std::runtime_error("Database not open !");
    }

    if (query("UPDATE config SET value=?2 WHERE cast(name as text)=cast(?1 as text)", name, value)->exec()
        < 1) {
        query("INSERT INTO config(name, value) VALUES (?1, ?2)", name, value)
            ->exec();
    }
}

std::vector<ToxBootstrapEntity> ToxDatabase::toxcore_bootstrap_get(bool active_only) {
    std::vector<ToxBootstrapEntity> res;
    auto stmt = query("SELECT active, ip, port, pub_key"
                      " FROM bootstrap"
                      " WHERE ?1 != 0 OR active != 0", active_only);
    while(stmt->executeStep()) {
        res.push_back({stmt->getColumn(0).getInt() != 0,
                       stmt->getColumn(1).getText(""),
                       stmt->getColumn(2).getInt(),
                       stmt->getColumn(3).getText("")});
    }
    return res;
}

void ToxDatabase::toxcore_log_add(ToxLogEntity entity) {
    entity.friendaddr.resize(64);
    std::string table = config_get("LOG_CHAT", 1) ? "log" : "mem.log";

    switch (entity.type) {
        case LOG_MESSAGE_SEND:
        case LOG_ACTION_SEND:
            query("INSERT INTO " + table + "(friendaddr, sendtime, type, data, receipt)"
                  " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)",
                  entity.friendaddr,
                  entity.type,
                  entity.data,
                  entity.receipt)->exec();
            break;
        case LOG_MESSAGE_RECV:
        case LOG_ACTION_RECV:
            query("INSERT INTO " + table + "(friendaddr, recvtime, type, data)"
                  " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3)",
                  entity.friendaddr,
                  entity.type,
                  entity.data)->exec();
            break;
        case LOG_FILE_SEND:
            query("INSERT INTO " + table + "(friendaddr, sendtime, type, data, fileid)"
                  " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)",
                  entity.friendaddr,
                  entity.type,
                  entity.data,
                  entity.unqiue_file_id)->exec();
            break;
        case LOG_FILE_RECV:
            query("INSERT INTO " + table + "(friendaddr, recvtime, type, data, fileid)"
                  " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)",
                  entity.friendaddr,
                  entity.type,
                  entity.data,
                  entity.unqiue_file_id)->exec();
            break;
    }
}

void ToxDatabase::toxcore_log_set_received(std::string friendaddr, int receipt_id) {
    friendaddr.resize(64);
    for(std::string table : {"log", "mem.log"}) {
        query("UPDATE " + table + " SET recvtime=CURRENT_TIMESTAMP, status=1"
              " WHERE friendaddr=?1 AND receipt=?2",
              friendaddr,
              receipt_id)->exec();
    }
}

void ToxDatabase::toxcore_log_set_file_complete(std::string friendaddr, uint32_t filenumber, std::array<uint8_t, TOX_FILE_ID_LENGTH> fileid) {
    friendaddr.resize(64);
    for(std::string table : {"log", "mem.log"}) {
        query("UPDATE " + table + " SET status=1"
              " WHERE friendaddr=?1 AND filenumber=?2 AND fileid=?3",
              friendaddr,
              filenumber,
              fileid)->exec();
    }
}

void ToxDatabase::toxcore_log_set_file_aborted(std::string friendaddr, uint32_t filenumber, std::array<uint8_t, TOX_FILE_ID_LENGTH> fileid) {
    friendaddr.resize(64);
    for(std::string table : {"log", "mem.log"}) {
        query("UPDATE " + table + " SET status=2"
              " WHERE friendaddr=?1 AND filenumber=?2 AND fileid=?3",
              friendaddr,
              filenumber,
              fileid)->exec();
    }
}


size_t ToxDatabase::toxcore_log_cleanup(){
    size_t res = 0;
    for(std::string table : {"log", "mem.log"}){
        res += query("DELETE FROM " + table)->exec();
    }
    return res;
}

std::vector<ToxLogEntity> ToxDatabase::toxcore_log_get(std::string friendaddr, int offset, int limit) {
    friendaddr.resize(64);
    std::vector<ToxLogEntity> res;
    auto stmt = query("SELECT"
                      " strftime('%s', sendtime)," //0
                      " strftime('%s', recvtime)," //1
                      " type, "                    //2
                      " data, "                    //3
                      " receipt, "                 //4
                      " fileid, "                  //5
                      " status  "                  //6
                      " FROM (SELECT * FROM log UNION ALL SELECT * FROM mem.log)"
                      " WHERE cast(friendaddr as text) = cast(?1 as text)"
                      " ORDER BY ifNull(sendtime, recvtime) DESC, id DESC LIMIT ?2, ?3",
                      friendaddr,
                      offset,
                      limit);
    while (stmt->executeStep()) {
        ToxLogEntity tmp;
        tmp.sendtime = stmt->getColumn(0).getInt64();
        tmp.recvtime = stmt->getColumn(1).getInt64();
        tmp.type = (EToxLogType)stmt->getColumn(2).getInt();
        auto data = stmt->getColumn(3);
        auto data_ptr = (const char*)data.getBlob();
        tmp.data = std::string(data_ptr, data_ptr + data.getBytes());
        tmp.receipt = stmt->getColumn(4).isNull()?-1:stmt->getColumn(4).getInt64();
        tmp.unqiue_file_id = stmt->getColumn(5).getInt64();
        tmp.status = EToxLogStatus(stmt->getColumn(6).getInt());
        res.push_back(tmp);
    }
    std::reverse(res.begin(), res.end());
    return res;
}

int ToxDatabase::get_runid() {
    return m_runid;
}

std::vector<gToxFileTransfEntity> ToxDatabase::gtoxfiletransf_get() {
    std::vector<gToxFileTransfEntity> res;
    auto stmt = query("SELECT"
                      " id, "
                      " is_recv, "
                      " friend_addr, "
                      " file_nr, "
                      " file_id, "
                      " file_kind, "
                      " file_path,"
                      " file_size, "
                      " status "
                      "FROM (SELECT * FROM file UNION ALL SELECT * FROM mem.file)");
    while (stmt->executeStep()) {
        gToxFileTransfEntity tmp;
        tmp.id = stmt->getColumn(0).getInt();
        tmp.is_recv = stmt->getColumn(1).getInt() == 1;
        {
            auto data = stmt->getColumn(2);
            auto data_ptr = (const char*)data.getBlob();
            tmp.friend_addr = std::string(data_ptr, data_ptr + data.getBytes());
        }
        tmp.file_nr = stmt->getColumn(3).getInt64();
        {
            auto data = stmt->getColumn(4);
            auto data_ptr = (const uint8_t*)data.getBlob();
            std::copy(data_ptr, data_ptr + data.getBytes(), tmp.file_id.begin());
        }
        tmp.file_kind = TOX_FILE_KIND(stmt->getColumn(5).getInt());
        {
            auto data = stmt->getColumn(6);
            auto data_ptr = (const char*)data.getBlob();
            tmp.file_path = std::string(data_ptr, data_ptr + data.getBytes());
        }
        tmp.file_size = stmt->getColumn(7).getInt64();
        tmp.status = stmt->getColumn(8).getInt();
        res.push_back(tmp);
    }
    return res;
}

void ToxDatabase::gtoxfiletransf_insert(gToxFileTransfEntity data) {
    data.friend_addr.resize(64);
    std::string table = config_get("LOG_CHAT", 1) ? "file" : "mem.file";
    query("INSERT INTO " + table + " ("
          " id, "
          " is_recv, "
          " friend_addr, "
          " file_nr, "
          " file_id, "
          " file_kind, "
          " file_path, "
          " file_size, "
          " status) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)",
                      data.id,
                      data.is_recv,
                      data.friend_addr,
                      data.file_nr,
                      data.file_id,
                      int(data.file_kind),
                      data.file_path,
                      data.file_size,
                      data.status)->exec();
}

void ToxDatabase::gtoxfiletransf_update(gToxFileTransfEntity data) {

}
