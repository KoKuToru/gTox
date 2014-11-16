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
#include "WidgetAbout.h"
#include "Generated/icon.h"
#include <glibmm/i18n.h>

WidgetAbout::WidgetAbout() : Glib::ObjectBase("WidgetAbout") {
    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;
    pack_start(*Gtk::manage(new Gtk::Image(ICON::load_icon(ICON::icon_128))));
    pack_start(*Gtk::manage([]() {
        auto tmp = new Gtk::Label;
        tmp->set_markup(_("ABOUT_SOFTWARE_NAME"));
        return tmp;
    }()));
    pack_start(*Gtk::manage([]() {
        auto tmp = new Gtk::Label;
        tmp->set_markup(_("ABOUT_SOFTWARE_VERSION"));
        return tmp;
    }()));
    pack_start(*Gtk::manage([]() {
        auto tmp = new Gtk::Label;
        tmp->set_margin_top(10);
        tmp->set_markup(_("ABOUT_SOFTWARE_HOMEPAGE"));
        return tmp;
    }()));
    pack_start(*Gtk::manage(new Gtk::Label(_("ABOUT_SOFTWARE_COPYRIGHT"))));
    pack_start(*Gtk::manage([]() {
        auto tmp = new Gtk::Label;
        tmp->set_margin_top(10);
        tmp->set_markup(_("ABOUT_SOFTWARE_WARRANTY"));
        return tmp;
    }()));
}

WidgetAbout::~WidgetAbout() {
}
