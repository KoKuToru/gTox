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
#ifndef TOXDATABASE_H
#define TOXDATABASE_H

#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include <memory>
#include <glibmm/ustring.h>
#include <vector>
#include <array>

class ToxBootstrapEntity {
    public:
        bool active;
        std::string ip;
        int port;
        std::string pub_key;
};

class ToxLogBaseEntity {
    public:
        ToxLogBaseEntity() = default;
        ToxLogBaseEntity(std::string friendaddr,
                         int type,
                         std::string data):
            friendaddr(friendaddr),
            type(type),
            data(data) {}

        std::string friendaddr;
        int type;
        std::string data;
};

class ToxLogRecvEntity:
        public ToxLogBaseEntity {
    public:
        using ToxLogBaseEntity::ToxLogBaseEntity;
};

class ToxLogSendEntity:
        public ToxLogBaseEntity {
    public:
        ToxLogSendEntity() = default;
        ToxLogSendEntity(std::string friendaddr,
                         int type,
                         std::string data,
                         int receipt): ToxLogBaseEntity(friendaddr, type, data), receipt(receipt) {}
        int receipt;
};

class ToxLogEntity:
        public ToxLogSendEntity {
    public:
        ToxLogEntity() = default;
        unsigned long long recvtime;
        unsigned long long sendtime;
};


class ToxDatabase {
  private:
    std::string m_path_db;
    std::string m_path_state;
    std::shared_ptr<SQLite::Database> m_db;

    void bind(SQLite::Statement& stmt, int i, const bool& value) {
        stmt.bind(i, value);
    }
    void bind(SQLite::Statement& stmt, int i, const int& value) {
        stmt.bind(i, value);
    }
    void bind(SQLite::Statement& stmt, int i, const sqlite3_int64& value) {
        stmt.bind(i, value);
    }
    void bind(SQLite::Statement& stmt, int i, const double& value) {
        stmt.bind(i, value);
    }
    void bind(SQLite::Statement& stmt, int i, const std::string& value) {
        stmt.bind(i, (void*)value.data(), value.size());
    }
    void bind(SQLite::Statement& stmt, int i, const Glib::ustring& value) {
        stmt.bind(i, value.raw());
    }
    void bind(SQLite::Statement& stmt, int i, const char* value) {
        stmt.bind(i, value);
    }
    void bind(SQLite::Statement& stmt,
              int i,
              const void* value,
              const int size) {
        stmt.bind(i, value, size);
    }
    template <typename T>
    void bind(SQLite::Statement& stmt, int i, const std::vector<T>& value) {
        bind(stmt, i, (const void*)value.data(), value.size() * sizeof(T));
    }
    template <typename T, size_t S>
    void bin(SQLite::Statement& stmt, int i, const std::array<T, S>& value) {
        bind(stmt, i, (const void*)value.data(), value.size() * sizeof(T));
    }

    struct function_base {
        virtual ~function_base() {
        }
        virtual void operator()(int i) = 0;
    };

    template <typename T>
    struct function : function_base {
        T m_f;
        function(const T& f) : m_f(f) {
        }
        void operator()(int i) {
            m_f(i);
        }
        ~function() {
        }
    };

    template <typename T>
    std::unique_ptr<function_base> function_create(const T& t) {
        return std::unique_ptr<function_base>(new function<T>(t));
    }

  public:
    ToxDatabase() {
    }
    ToxDatabase(const ToxDatabase& o) = delete;
    ~ToxDatabase() {
    }
    void operator=(const ToxDatabase& o) = delete;

    template <typename... T>
    std::shared_ptr<SQLite::Statement> query(std::string query, const T&... t) {
        auto stmt = std::make_shared<SQLite::Statement>(*m_db, query);
        std::unique_ptr<function_base> funcs[] = {(function_create(
            std::bind(static_cast<void (
                          ToxDatabase::*)(SQLite::Statement&, int, const T&)>(
                          &ToxDatabase::bind),
                      this,
                      std::ref(*stmt),
                      std::placeholders::_1,
                      std::cref(t))))...};
        for (size_t i = 0; i < sizeof...(t); ++i) {
            funcs[i]->operator()(i + 1);
        }
        return stmt;
    }

    void open(const std::string& path, bool init = false);
    void close();
    void move(const std::string& path);

    /**
     * @brief Gets a config parameter
     *
     * @throws SQLite::Exception
     *
     * @param name of the paramter
     * @param value the default value
     *
     * @return value of the config parameter
     */
    std::string config_get(const std::string& name, const std::string& value);
    /**
     * @brief Sets a config parameter
     *
     * @throws SQLite::Exception
     *
     * @param name  of the paramter
     * @param value of the config parameter
     */
    void config_set(const std::string& name, const std::string& value);

    /**
     * @brief Gets a config parameter
     *
     * @throws Tox::Exception
     *
     * @param name of the paramter
     * @param value the default value
     *
     * @return value of the config parameter
     */
    int config_get(const std::string& name, int value);
    /**
     * @brief Sets a config parameter
     *
     * @throws Tox::Exception
     *
     * @param name  of the paramter
     * @param value of the config parameter
     */
    void config_set(const std::string& name, int value);

    /**
     * @brief Removes old toxcore states
     *
     * @throws SQLite::Exception
     *
     */
    void toxcore_state_cleanup();
    /**
     * @brief Get a toxcore state
     *
     * @throws SQLite::Exception
     *
     * @param nth 0 = newest, n = oldest
     * @return toxcore state
     */
    std::vector<unsigned char> toxcore_state_get(int nth);
    /**
     * @brief Get the count of saved toxcore states
     *
     * @throws SQLite::Exception
     *
     * @return how many nth toxcore states
     */
    int toxcore_state_max_nth();
    /**
     * @brief Add a new toxcore state, will overwrite the old one if the same
     *
     * @throws SQLite::Exception
     *
     * session
     * @param state toxcore state
     */
    void toxcore_state_add(const std::vector<unsigned char>& state);

    /**
     * @brief Gets the bootstrap settings
     *
     * @throws SQLite::Exception
     *
     * @param active_only
     * @return list of bootstraps
     */
    std::vector<ToxBootstrapEntity> toxcore_bootstrap_get(bool active_only = true);
    /**
     * @brief Adds a new log line
     *
     * @throws SQLite::Exception
     *
     * @param entity
     */
    void toxcore_log_add(ToxLogSendEntity entity);
    void toxcore_log_add(ToxLogRecvEntity entity);
    void toxcore_log_set_received(std::string friendaddr, int receipt_id);
    /**
     * @brief Get chat log
     *
     * @throws SQLite::Exception
     *
     * @param nr ID of friend
     * @param offset
     * @param limit
     *
     * @return Chat log
     */
    std::vector<ToxLogEntity> toxcore_log_get(std::string friendaddr, int offset, int limit);
};

#endif
