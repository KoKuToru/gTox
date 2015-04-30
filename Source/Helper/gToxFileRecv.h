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
#ifndef H_GTOX_FILE_RECV
#define H_GTOX_FILE_RECV

#include "gToxObserver.h"

class gToxFileRecv: public gToxObserver {
    public:
        enum STATE {
            PAUSED,
            RECVING,
            STOPPED
        };

        gToxFileRecv(gToxObservable* observable,
                     Toxmm::EventFileRecv file);
        gToxFileRecv(const gToxFileRecv& o);
        void operator=(const gToxFileRecv& o);
        ~gToxFileRecv();

        void resume();
        void cancel();
        void pause();

        void get_progress(uint64_t& position, uint64_t& size);

        class EventFileProgress {
            public:
                gToxFileRecv* receiver;
                Toxmm::FriendNr nr;
                TOX_FILE_KIND file_kind;
                uint32_t file_number;
                uint64_t file_position;
                uint64_t file_size;
        };
    private:
        int m_fd = -1;
        Toxmm::EventFileRecv m_file;
        Glib::ustring m_path;
        uint64_t m_position = 0;
        STATE m_state = PAUSED;

    protected:
        void observer_handle(const ToxEvent&) override;
};

#endif
