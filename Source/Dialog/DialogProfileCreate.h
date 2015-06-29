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
#include "Helper/gToxBuilder.h"
#include "Tox/Toxmm.h"

class DialogProfileCreate : public Gtk::Assistant {
  private:
    bool m_aborted;
    Glib::ustring m_path;

    Gtk::Entry  *m_username = nullptr;
    Gtk::Entry  *m_status = nullptr;
    Gtk::Switch *m_logging = nullptr;
    Gtk::Entry  *m_file_tox = nullptr;
    Gtk::Entry  *m_file_gtox = nullptr;

    void on_cancel();
    void on_close();
    void on_apply();

    Toxmm m_tox_instance;

  public:
    ~DialogProfileCreate();

    DialogProfileCreate(BaseObjectType* cobject, gToxBuilder builder,
                        const Glib::ustring& path);

    static gToxBuilderRef<DialogProfileCreate> create(const Glib::ustring& path);

    bool is_aborted();
    Glib::ustring get_path();
};

#endif
