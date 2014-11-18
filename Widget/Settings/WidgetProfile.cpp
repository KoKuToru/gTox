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
#include "WidgetProfile.h"
#include "Generated/icon.h"
#include <glibmm/i18n.h>

WidgetProfile::WidgetProfile() : Glib::ObjectBase("WidgetProfile") {
    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto grid = Gtk::manage(new Gtk::Grid);

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);

    grid->attach(*Gtk::manage(new Gtk::Label("Username", 0, 0.5)), 0, 0, 1, 1);
    grid->attach(m_username, 1, 0, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Status", 0, 0.5)), 0, 1, 1, 1);
    grid->attach(m_status, 1, 1, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Tox ID", 0, 0.5)), 0, 2, 1, 1);
    auto tox_id = Gtk::manage(new Gtk::Label("AABBCCDDEEFFF...", 0, 0.5));
    tox_id->set_selectable(true);
    grid->attach(*tox_id, 1, 2, 1, 1);

    pack_start(*grid, false, false);

    show_all();
}

WidgetProfile::~WidgetProfile() {
}
