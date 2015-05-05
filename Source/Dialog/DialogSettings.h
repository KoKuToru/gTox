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
#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include "Helper/gToxBuilder.h"
#include "Helper/gToxObserver.h"

class DialogSettings : public Gtk::Window, public gToxObserver {
  private:
        gToxBuilder m_builder;
        bool m_in_window = false;

        Gtk::HeaderBar* m_headerbar = nullptr;
        Gtk::Box*       m_body = nullptr;

        Gtk::Box m_headerbar_box;
        Gtk::Box m_body_box;

        Glib::RefPtr<Gtk::SizeGroup> m_size_group;

  public:
        DialogSettings(gToxObservable* observable);
        ~DialogSettings();

        void show();
        void hide();
        void present();

        bool is_visible();
};

#endif
