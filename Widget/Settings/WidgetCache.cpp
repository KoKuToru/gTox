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
#include "WidgetCache.h"
#include "Generated/icon.h"
#include <glibmm/i18n.h>

WidgetCache::WidgetCache()
    : Glib::ObjectBase("WidgetNetwork"),
      m_log("Log chat"),
      m_file("Cache recieved files"),
      m_clean_log("Clean up logs"),
      m_clean_file("Clean up recieved files") {
    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto grid = Gtk::manage(new Gtk::Grid);

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);

    grid->attach(m_log, 0, 0, 2, 1);
    grid->attach(m_file, 0, 1, 2, 1);
    grid->attach(m_clean_log, 0, 2, 1, 1);
    grid->attach(m_clean_file, 1, 2, 1, 1);

    pack_start(*grid, false, false);

    show_all();
}

WidgetCache::~WidgetCache() {
}
