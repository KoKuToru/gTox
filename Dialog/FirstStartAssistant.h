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

#include "Widget/Assistant/AccountWidget.h"
#include "Widget/Assistant/WelcomePage.h"
#include "Widget/Assistant/NewAccountWidget.h"

class FirstStartAssistant : public Gtk::Assistant {
private:
  WelcomePage welcome;
  AccountWidget account;
  NewAccountWidget create;
  Gtk::Box import;
  Gtk::Box finish;

  Glib::ustring path;

  bool aborted;

  int getPage(Gtk::Widget &widget);

  int on_next(int page);
  void on_cancel();
  void on_close();

public:
  FirstStartAssistant(Glib::ustring path);
  ~FirstStartAssistant();

  bool isAborted() { return aborted; }
  Glib::ustring getPath() { return path; }
};

#endif
