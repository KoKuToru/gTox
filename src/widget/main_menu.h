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
#ifndef WIDGETPOPOVERSETTINGS_H
#define WIDGETPOPOVERSETTINGS_H

#include <gtkmm.h>
#include "tox/core.h"
#include "widget/avatar.h"

namespace dialog {
    class main;
    class settings;
}

namespace widget {
    class main_menu : public Gtk::Popover {
        private:
            Glib::RefPtr<dialog::main> m_main;
            Glib::RefPtr<dialog::settings> m_settings;

            struct {
                    Gtk::Entry* username;
                    Gtk::Entry* status;
                    widget::avatar* avatar;
            } m_profile;

            struct {
                    Gtk::Entry* tox_id;
                    Gtk::TextView* message;
            } m_add_contact;

            Gtk::Stack* m_stack;
            Gtk::Button* m_settings_btn;

        public:
            main_menu(Glib::RefPtr<dialog::main> m_main);
            ~main_menu();

            void set_visible(bool visible = true);

        protected:
            void add_contact();
    };
}
#endif
