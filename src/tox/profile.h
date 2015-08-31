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
#ifndef TOXMM2_PROFILE_H
#define TOXMM2_PROFILE_H

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <glibmm.h>
#include <giomm.h>
#include <set>

namespace toxmm {

    //TODO: make this portable
    class profile {
        private:
            std::set<Glib::ustring>& create_used_files();

            bool m_can_write;
            bool m_can_read;
            Glib::RefPtr<Gio::File>            m_file;
            Glib::RefPtr<Gio::FileInputStream> m_stream;
            std::set<Glib::ustring>& m_used_files;

        public:
            profile();
            profile(const profile& o) = delete;
            ~profile();

            void operator=(const profile& o) = delete;

            void open(const Glib::ustring& path);
            void close();

            bool can_write();
            bool can_read();

            /**
            * @brief if new_path exists it will be overwritten with this profile
            * @param new_path
            */
            void move(const Glib::ustring& new_path);

            void write(const std::vector<unsigned char>& data);
            std::vector<unsigned char> read();
    };
}


#endif
