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

#ifndef FIRSTSTARTASSISTANT_H
#define FIRSTSTARTASSISTANT_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/debug.h"

namespace dialog {
    class profile_create : public Gtk::Assistant, public utils::debug::track_obj<profile_create> {
        private:
            bool m_aborted;
            Glib::ustring m_path;

            std::vector<uint8_t> m_last_toxcore;

            Gtk::Entry  *m_username = nullptr;
            Gtk::Entry  *m_status = nullptr;
            Gtk::Entry  *m_file_tox = nullptr;

            void on_cancel();
            void on_close();
            void on_apply();

        public:
            ~profile_create();

            profile_create(BaseObjectType* cobject, utils::builder builder,
                                const Glib::ustring& path);

            static utils::builder::ref<profile_create> create(const Glib::ustring& path);

            bool is_aborted();
            Glib::ustring get_path();
    };
}
#endif
