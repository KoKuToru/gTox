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
        Gio::File::create_for_path(npath)->make_directory();
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
               &DATABASE::version_9};

        if (version < 1 || version > (int)sizeof(upgrade_scripts)) {
            throw std::runtime_error("Database not open !");
        }

        int version_max
            = 1 + sizeof(upgrade_scripts) / sizeof(upgrade_scripts[0]);

        for (; version < version_max; ++version) {
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
    config_set("runid", config_get("runid", 0) + 1);

    //attach in memory database
    m_db->exec("ATTACH DATABASE ':memory:' AS mem");
    m_db->exec("CREATE TABLE mem.log AS SELECT * FROM log WHERE 0");
}

void ToxDatabase::close() {
    m_db.reset();
}

void ToxDatabase::save() {
    m_db.reset();
    m_db = std::make_shared<SQLite::Database>(
        m_path_db, SQLITE_OPEN_READWRITE);
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

void ToxDatabase::toxcore_log_add(ToxLogSendEntity entity) {
    entity.friendaddr.resize(64);
    std::string table = config_get("LOG_CHAT", 1) ? "log" : "mem.log";

    query("INSERT INTO " + table + "(friendaddr, sendtime, type, message, receipt)"
                                   " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3, ?4)",
          entity.friendaddr,
          entity.type,
          entity.data,
          entity.receipt)->exec();
}

void ToxDatabase::toxcore_log_add(ToxLogRecvEntity entity) {
    entity.friendaddr.resize(64);
    std::string table = config_get("LOG_CHAT", 1) ? "log" : "mem.log";

    query("INSERT INTO " + table + "(friendaddr, recvtime, type, message)"
          " VALUES (?1, CURRENT_TIMESTAMP, ?2, ?3)",
          entity.friendaddr,
          entity.type,
          entity.data)->exec();
}

void ToxDatabase::toxcore_log_set_received(std::string friendaddr, int receipt_id) {
    friendaddr.resize(64);
    for(std::string table : {"log", "mem.log"}) {
        query("UPDATE " + table + " SET recvtime=CURRENT_TIMESTAMP"
              " WHERE friendaddr=?1 AND receipt=?2",
              friendaddr,
              receipt_id)->exec();
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
                      " strftime('%s', sendtime),"
                      " strftime('%s', recvtime),"
                      " type, message"
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
        tmp.type = stmt->getColumn(2).getInt();
        auto data = stmt->getColumn(3);
        auto data_ptr = (const char*)data.getBlob();
        tmp.data = std::string(data_ptr, data_ptr + data.getBytes());
        res.push_back(tmp);
    }
    //memory db ! todo !
    return res;
}
