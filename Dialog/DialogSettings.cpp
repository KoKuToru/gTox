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
  this->set_default_geometry(600, 600);
  this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

  auto hbox = Gtk::manage(new Gtk::HBox());
  auto sidebar = Gtk::manage(new Gtk::Box());
  auto seperator = Gtk::manage(new Gtk::VSeparator());
  auto box = Gtk::manage(new Gtk::Box());
  auto listbox = Gtk::manage(new Gtk::ListBox());
  auto scroller_1 = Gtk::manage(new Gtk::ScrolledWindow());
  auto scroller_2 = Gtk::manage(new Gtk::ScrolledWindow());

  add(*hbox);
  hbox->add(*sidebar);
  hbox->add(*seperator);
  hbox->add(*box);
  box->add(m_stack);
  sidebar->add(*scroller_1);
  scroller_1->add(*listbox);
  box->add(*scroller_2);

  scroller_2->add(*Gtk::manage(new Gtk::Label("Test")));

  scroller_1->set_hexpand(false);
  seperator->set_hexpand(false);
  scroller_1->set_halign(Gtk::ALIGN_START);
  seperator->set_halign(Gtk::ALIGN_START);

  listbox->add(*Gtk::manage(new Gtk::Label("Test")));
}

DialogSettings::~DialogSettings() {}

void DialogSettings::show() { show_all(); }
