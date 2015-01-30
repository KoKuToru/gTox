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
#include <Tox/Tox.h>

WidgetCache::WidgetCache()
    : Glib::ObjectBase("WidgetNetwork"),
      m_log(),
      m_file(),
      m_clean_log("Clean up logs"),
      m_clean_file("Clean up recieved files") {

    m_log.set_active(Tox::instance().config_get("LOG_CHAT","1")=="1");
    m_file.set_active(Tox::instance().config_get("LOG_FILE","1")=="1");

    m_log.signal_state_set().connect_notify([](bool state) {
        Tox::instance().config_set("LOG_CHAT", std::to_string((int)state));
    });
    m_file.signal_state_set().connect_notify([](bool state) {
        Tox::instance().config_set("LOG_FILE", std::to_string((int)state));
    });

    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto grid = Gtk::manage(new Gtk::Grid);

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);

    grid->attach(*Gtk::manage(new Gtk::Label("Persist Chatlog", 0.0, 0.5)), 0, 0, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label("Persist File", 0.0, 0.5)), 0, 1, 1, 1);
    grid->attach(m_log, 1, 0, 1, 1);
    grid->attach(m_file, 1, 1, 1, 1);
    grid->attach(m_clean_log, 0, 2, 2, 1);
    grid->attach(m_clean_file, 0, 3, 2, 1);

    pack_start(*grid, false, false);

    show_all();
}

WidgetCache::~WidgetCache() {
}
