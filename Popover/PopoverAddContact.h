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
#ifndef POPOVERADDCONTACT_H
#define POPOVERADDCONTACT_H

#include <gtkmm.h>
class PopoverAddContact : public Gtk::Popover {
 private:
  Gtk::Entry m_addr;
  Gtk::TextView m_msg;

 public:
  PopoverAddContact(const Gtk::Widget &relative_to);
  ~PopoverAddContact();
};

#endif
