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
#include <stdio.h>

WidgetCache::WidgetCache()
    : Glib::ObjectBase("WidgetNetwork"),
      m_log(),
      m_file(),
      m_clean_log("Clean up logs"),
      m_clean_file("Clean up recieved files") {

    m_log.set_active(Tox::instance().database().config_get("LOG_CHAT", 1));
    m_file.set_active(Tox::instance().database().config_get("LOG_FILE", 1));
    m_log.set_halign(Gtk::Align::ALIGN_END);
    m_file.set_halign(Gtk::Align::ALIGN_END);

#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 14)
    m_log.signal_state_set().connect_notify([](bool state) {
        Tox::instance().database().config_set("LOG_CHAT", state);
    });
    m_file.signal_state_set().connect_notify([](bool state) {
        Tox::instance().database().config_set("LOG_FILE", state);
    });
#else
#warning "Without GTK 3.14+ signal_state_set() on Gtk::Switches wont work"
#endif

    m_clean_log.signal_clicked().connect([this](){
        Gtk::Window& parent = dynamic_cast<Gtk::Window&>(*this->get_toplevel());
        Gtk::MessageDialog msg(parent,
                               _("SETTINGS_CACHE_CLEAN_LOG_TITLE"),
                               false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_OK_CANCEL,
                               true);
        msg.set_secondary_text(_("SETTINGS_CACHE_CLEAN_LOG"));
        if (msg.run() == Gtk::RESPONSE_YES) {
            msg.hide();
            Gtk::MessageDialog msg2(parent,
                                    _("SETTINGS_CACHE_CLEAN_LOG_SUCCESS_TITLE"),
                                    false,
                                    Gtk::MESSAGE_INFO,
                                    Gtk::BUTTONS_OK);
            msg2.set_secondary_text(
                        Glib::ustring::compose(
                            _("SETTINGS_CACHE_CLEAN_LOG_SUCCESS"),
                            Tox::instance().database().toxcore_log_cleanup()));
            msg2.run();
        }
    });

    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto grid = Gtk::manage(new Gtk::Grid);

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);

    grid->attach(
        *Gtk::manage(new Gtk::Label("Persist Chatlog", 0.0, 0.5)), 0, 0, 1, 1);
    /*grid->attach(
        *Gtk::manage(new Gtk::Label("Persist File", 0.0, 0.5)), 0, 1, 1, 1);*/
    grid->attach(m_log, 1, 0, 1, 1);
    // grid->attach(m_file, 1, 1, 1, 1);
    grid->attach(m_clean_log, 0, 2, 2, 1);
    grid->attach(m_clean_file, 0, 3, 2, 1);

    pack_start(*grid, false, false);

    show_all();
}

WidgetCache::~WidgetCache() {
}
