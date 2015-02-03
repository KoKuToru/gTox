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
#include "Generated/database.h"

void ToxDatabase::open(const std::string& path, bool init) {
    if (m_db) {
        throw "ERROR";
    }
    m_path = path;
    m_db = std::make_shared<SQLite::Database>(
        m_path, SQLITE_OPEN_READWRITE | (init ? SQLITE_OPEN_CREATE : 0));

    if (init) {
        m_db->exec(DATABASE::version_1);
    }

    // check for updates
    m_db->exec("SAVEPOINT try_load");
    try {

        auto version_str = config_get("version", "1");
        int version = std::atoi(version_str.c_str());

        std::string* upgrade_scripts[]
            = {&DATABASE::version_2, &DATABASE::version_3};

        if (version < 1 || version > (int)sizeof(upgrade_scripts)) {
            throw "ERROR";
        }

        int version_max
            = 1 + sizeof(upgrade_scripts) / sizeof(upgrade_scripts[0]);

        for (; version < version_max; ++version) {
            m_db->exec(*upgrade_scripts[version - 1]);
            config_set("version", std::to_string(version + 1));
        }

        // clean up toxcore
        try {
            SQLite::Statement storeq(*m_db,
                                     "DELETE FROM toxcore"
                                     " WHERE id < ?1");
            storeq.bind(1,
                        m_db->execAndGet(
                                  "SELECT max(id), date(savetime) FROM toxcore"
                                  " GROUP BY date(savetime)"
                                  " ORDER BY id DESC LIMIT 7,1").getInt());
            storeq.exec();
        } catch (...) {
            // nothing to remove
        }

        m_db->exec("RELEASE SAVEPOINT try_load");
    } catch (...) {
        // restore old state
        m_db->exec("ROLLBACK TO SAVEPOINT try_load");

        throw;
    }

    // increase runid
    config_set("rundid", config_get("runid", 0) + 1);
}

void ToxDatabase::close() {
    if (!m_db) {
        throw "ERROR";
    }
    m_db.reset();
}

void ToxDatabase::move(const std::string& path) {
    if (!m_db) {
        throw "ERROR";
    }
    // just to be save close database
    m_db.reset();
    // try to move file
    try {
        if (!Gio::File::create_for_path(m_path)
                 ->move(Gio::File::create_for_path(path))) {
            throw "ERROR";
        }
        m_path = path;
        // open new database
        m_db
            = std::make_shared<SQLite::Database>(m_path, SQLITE_OPEN_READWRITE);
    } catch (...) {
        // restore m_db state
        m_db
            = std::make_shared<SQLite::Database>(m_path, SQLITE_OPEN_READWRITE);
        throw;
    }
}

std::string ToxDatabase::config_get(const std::string& name,
                                    const std::string& value = "") {
    if (!m_db) {
        throw "ERROR";
    }

    auto selectq
        = query("SELECT value FROM config WHERE name=?1 LIMIT 1", name);

    if (selectq->executeStep()) {
        return selectq->getColumn(0).getText(value.c_str());
    }

    return value;
}

void ToxDatabase::config_set(const std::string& name,
                             const std::string& value) {
    if (!m_db) {
        throw "ERROR";
    }

    if (query("UPDATE config SET value=?2 WHERE name=?1", name, value)->exec()
        < 1) {
        query("INSERT INTO config(name, value) VALUES (?1, ?2)", name, value)
            ->exec();
    }
}

int ToxDatabase::config_get(const std::string& name, int value = 0) {
    if (!m_db) {
        throw "ERROR";
    }

    auto selectq
        = query("SELECT value FROM config WHERE name=?1 LIMIT 1", name);

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
        throw "ERROR";
    }

    if (query("UPDATE config SET value=?2 WHERE name=?1", name, value)->exec()
        < 1) {
        query("INSERT INTO config(name, value) VALUES (?1, ?2)", name, value)
            ->exec();
    }
}

void ToxDatabase::toxcore_state_cleanup() {
    if (!m_db) {
        throw "ERROR";
    }

    auto max_stmt = query(
        "SELECT max(id), date(savetime) FROM toxcore"
        " GROUP BY date(savetime) ORDER BY id DESC LIMIT 7,1");
    if (max_stmt->executeStep()) {
        int max_id = max_stmt->getColumn(0).getInt();
        query("DELETE FROM toxcore WHERE id < ?1", max_id)->exec();
    }
}

std::vector<unsigned char> ToxDatabase::toxcore_state_get(int nth) {
    if (!m_db) {
        throw "ERROR";
    }

    auto res = m_db->execAndGet(
        "SELECT state FROM toxcore ORDER BY id DESC"
        " LIMIT " + std::to_string(nth) + ", 1");

    std::vector<unsigned char> tmp(res.getBytes());
    unsigned char* raw = (unsigned char*)res.getBlob();
    std::copy(raw, raw + tmp.size(), tmp.begin());
    return tmp;
}

int ToxDatabase::toxcore_state_max_nth() {
    if (!m_db) {
        throw "ERROR";
    }

    return m_db->execAndGet("SELECT count(*) FROM toxcore").getInt();
}

void ToxDatabase::toxcore_state_add(const std::vector<unsigned char>& state) {
    if (!m_db) {
        throw "ERROR";
    }

    int runid = config_get("runid", 0);
    if (query(
            "UPDATE toxcore(savetime, state, runid)"
            " SET (CURRENT_TIMESTAMP, ?2, ?1) WHERE runid=?1",
            runid,
            state)->exec() < 1) {
        query(
            "INSERT INTO toxcore(savetime, state, runid)"
            " VALUES (CURRENT_TIMESTAMP, ?2, ?1",
            config_get("runid", 0),
            state)->exec();
    }
}
