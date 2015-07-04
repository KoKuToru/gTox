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
#ifndef H_GTOX_FILE_RECV2
#define H_GTOX_FILE_RECV2

#include "gToxFileManager.h"
#include <giomm.h>

class gToxFileRecv2: public gToxFileTransf {
        friend class gToxFileManager;
    public:
        using gToxFileTransf::gToxFileTransf;

        void resume();
        void pause();
        void cancel();

    private:
        Glib::RefPtr<Gio::FileOutputStream> m_stream;

    protected:
        bool is_recv();

        void deactivate();
        void activate();

        void recv_chunk(uint64_t position, const std::vector<uint8_t>& data, std::function<void()> callback);
        void send_chunk(uint64_t, size_t, std::function<void(const std::vector<uint8_t>&)>);
};

#endif
