/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#ifndef DIALOGERROR_H
#define DIALOGERROR_H

#include <gtkmm.h>
#include "Tox/Toxmm.h"
#include "Widget/WidgetChat.h"

// Single chat window
class DialogError : public Gtk::MessageDialog {
  private:
    std::string get_stacktrace();

  public:
    DialogError(bool fatal, std::string title, std::string message);
    ~DialogError();

    int run();
};

#endif
