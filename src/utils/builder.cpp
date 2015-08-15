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
#include "builder.h"

using namespace utils;

builder::builder(const Glib::RefPtr<Gtk::Builder> builder):
    m_builder(builder) {
}

Glib::RefPtr<Gtk::Builder> builder::operator->() {
    return m_builder;
}

GtkWidget* builder::get_cwidget(const Glib::ustring& name)
{
  GObject *cobject = gtk_builder_get_object (m_builder->gobj(), name.c_str());
  if(!cobject) {
      throw std::runtime_error(std::string() + "gToxBuilder - widget \"" + name + "\"not found");
  }

  if (!GTK_IS_WIDGET (cobject)) {
      throw std::runtime_error("gToxBuilder - not a widget");
      return 0;
  }

  return GTK_WIDGET(cobject);
}
