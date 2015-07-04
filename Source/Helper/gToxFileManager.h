/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#ifndef H_GTOX_FILE_MANAGER
#define H_GTOX_FILE_MANAGER

#include <vector>
#include "gToxObserver.h"
#include "Tox/Toxmm.h"
#include <functional>
#include <iostream>

struct gToxFileTransfEntity {
        size_t          id;
        bool            is_recv;
        std::string     friend_addr;
        Toxmm::FileNr   file_nr;
        Toxmm::FileId   file_id;
        TOX_FILE_KIND   file_kind;
        std::string     file_name;
        std::string     file_path;
        size_t          file_size;
        int             status;
};

class gToxFileManager;
class gToxFileTransf {
        friend class gToxFileManager;
    public:
        enum STATE {
            PAUSE,
            RESUME,
            CANCEL,
            FINISH
        };

    protected:
        std::weak_ptr<gToxFileManager> m_manager;

    //private:
    public:
        STATE           m_state;
        size_t          m_id;
        bool            m_active;
        Toxmm::FriendNr m_friend_nr;
        Toxmm::FileNr   m_file_nr;
        Toxmm::FileId   m_file_id;
        TOX_FILE_KIND   m_file_kind;
        std::string     m_file_name;
        std::string     m_file_path;
        size_t          m_file_position;
        size_t          m_file_size;

        virtual void deactivate() = 0;
        virtual void activate() = 0;

        virtual void recv_chunk(uint64_t position, const std::vector<uint8_t>& data, std::function<void()> callback) = 0;
        virtual void send_chunk(uint64_t position, size_t size, std::function<void(const std::vector<uint8_t>&)> callback) = 0;

    public:
        virtual bool is_recv() = 0;

        gToxFileTransf(std::weak_ptr<gToxFileManager> manager,
                       size_t id,
                       Toxmm::FriendNr friend_nr,
                       Toxmm::FileNr file_nr,
                       Toxmm::FileId file_id,
                       TOX_FILE_KIND file_kind,
                       std::string file_name,
                       std::string file_path,
                       size_t file_position,
                       size_t file_size,
                       int status):
            m_manager(manager),
            m_state(gToxFileTransf::STATE(status)),
            m_id(id),
            m_active(false),
            m_friend_nr(friend_nr),
            m_file_nr(file_nr),
            m_file_id(file_id),
            m_file_kind(file_kind),
            m_file_name(file_name),
            m_file_path(file_path),
            m_file_position(file_position),
            m_file_size(file_size) {
            std::clog << "Created instance " << (void*)this << std::endl;
        }
        ~gToxFileTransf() {
            std::clog << "Destroyed instance " << (void*)this << std::endl;
        }

        virtual void resume() = 0;
        virtual void pause() = 0;
        virtual void cancel() = 0;

        size_t id() {
            return m_id;
        }

        std::shared_ptr<gToxFileManager> manager() {
            return m_manager.lock();
        }

        const std::string& path() const {
            return m_file_path;
        }

        STATE state() const {
            return m_state;
        }

        bool active() const {
            return m_active;
        }

        Toxmm::FriendNr friend_nr() const {
            return m_friend_nr;
        }

        Toxmm::FileNr file_nr() const {
            return m_file_nr;
        }

        TOX_FILE_KIND file_kind() const {
            return m_file_kind;
        }

        const std::string& file_name() const {
            return m_file_name;
        }

        const std::string& file_path() const {
            return m_file_path;
        }

        size_t file_position() const {
        std::clog << "Position instance " << (void*)this << " " << m_file_position << std::endl;
            return m_file_position;
        }

        size_t file_size() const {
            return m_file_size;
        }
};

class gToxFileSend2: public gToxFileTransf {
        friend class gToxFileManager;
    public:
        using gToxFileTransf::gToxFileTransf;

    protected:
        bool is_recv() {
            return true;
        }
        void resume () {
        }
        void pause  () {
        }
        void cancel () {
        }
        void deactivate() {
        }
        void activate() {
        }
        void activate(Toxmm::FileNr nr) {
        }
        void recv_chunk(uint64_t , const std::vector<uint8_t>& , std::function<void()> ) {
        }
        void send_chunk(uint64_t , size_t , std::function<void(const std::vector<uint8_t>&)> ) {
        }
};


/**
 * @brief Manages all send/recv of files
 */
class Toxmm;
class gToxFileManager: public std::enable_shared_from_this<gToxFileManager> {
    private:
        Toxmm* m_tox;
        std::map<size_t, std::shared_ptr<gToxFileTransf>> m_file;

    public:
        gToxFileManager(Toxmm* tox);
        void init();
        ~gToxFileManager();

        std::shared_ptr<gToxFileTransf> find(size_t unique_file_id);
        std::vector<std::shared_ptr<gToxFileTransf>> find_by_friend_nr(Toxmm::FriendNr nr);

        void resume(gToxFileTransf* file);
        void pause(gToxFileTransf* file);
        void cancel(gToxFileTransf* file);

        void observer_handle(const ToxEvent&);

        /**
         * A new file recv or send
         */
        class EventNewFile {
            public:
                std::shared_ptr<gToxFileTransf> file;
        };

        /**
         * State and or progress changed
         */
        class EventFileUpdate {
            public:
                std::shared_ptr<gToxFileTransf> file;
        };
};

#endif
