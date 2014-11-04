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

#include "DialogSettings.h"
#include <glibmm/i18n.h>
#include "Debug/DialogCss.h"

DialogSettings::DialogSettings() {
    set_title(_("SETTINGS_TITLE"));
    set_default_geometry(800, 600);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    auto hbox = Gtk::manage(new Gtk::HBox());
    auto sidebar = Gtk::manage(new Gtk::Box());
    auto seperator = Gtk::manage(new Gtk::VSeparator());
    auto box = Gtk::manage(new Gtk::Box());
    auto listbox = Gtk::manage(new Gtk::ListBox());
    auto scroller_1 = Gtk::manage(new Gtk::ScrolledWindow());
    auto scroller_2 = Gtk::manage(new Gtk::ScrolledWindow());

    add(*hbox);
    hbox->pack_start(*sidebar, false, false);
    hbox->pack_start(*seperator, false, false);
    hbox->add(*box);
    sidebar->add(*scroller_1);
    scroller_1->add(*listbox);
    box->pack_start(*scroller_2, true, true);
    scroller_2->add(m_stack);

    m_stack.add(*Gtk::manage(new Gtk::Label("Test")), "");

    listbox->get_style_context()->add_class("settings");

    listbox->add(*Gtk::manage(new Gtk::Label("AAAAAAAAAAAAAAA", 0, 0.5)));
    listbox->add(*Gtk::manage(new Gtk::Label("Test2", 0, 0.5)));
    listbox->add(*Gtk::manage(new Gtk::Label("Test3", 0, 0.5)));

    // only scroll vertically
    scroller_1->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scroller_2->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
}

DialogSettings::~DialogSettings() {
}

void DialogSettings::show() {
    show_all();
}
