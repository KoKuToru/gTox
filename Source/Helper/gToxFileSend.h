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
#ifndef H_GTOX_FILE_SEND
#define H_GTOX_FILE_SEND

#include "gToxObserver.h"
#include <giomm.h>

class gToxFileSend: public gToxObserver {
    public:
        enum STATE {
            INITIAL,
            PAUSED,
            SENDING,
            STOPPED
        };

        gToxFileSend(gToxObservable* observable,
                     Toxmm::FriendNr nr,
                     TOX_FILE_KIND kind,
                     Glib::ustring path);
        gToxFileSend(gToxObservable* observable,
                     Toxmm::FriendNr nr,
                     Glib::ustring path,
                     Toxmm::FileId id,
                     uint64_t filesize);

        gToxFileSend(const gToxFileSend& o) = delete;
        void operator=(const gToxFileSend& o) = delete;
        ~gToxFileSend();

        void resume();
        void cancel();
        void pause();

        void get_progress(uint64_t& position, uint64_t& size);
        Glib::ustring get_path();

        void emit_progress();

        class EventFileProgress {
            public:
                gToxFileSend* sender;
                Toxmm::FriendNr nr;
                std::string file_path;
                long long file_number;
                uint64_t file_position;
                uint64_t file_size;
        };
    private:
        Glib::RefPtr<Gio::File> m_file;
        Glib::RefPtr<Gio::FileInputStream> m_stream;
        Toxmm::FriendNr m_friend_nr;
        Toxmm::FileNr m_nr;
        Toxmm::FileId m_id;
        Glib::ustring m_path;
        TOX_FILE_KIND m_kind;
        uint64_t m_position = 0;
        uint64_t m_size = 0;
        STATE m_state = INITIAL;

    protected:
        void observer_handle(const ToxEvent&) override;
};

#endif
