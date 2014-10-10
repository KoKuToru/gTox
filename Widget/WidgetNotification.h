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
#ifndef WIDGETNOTIFICATION_H
#define WIDGETNOTIFICATION_H

#include <gtkmm.h>
class WidgetNotification: public Gtk::Box {
    private:
        Gtk::Label  m_title;
        Gtk::Label  m_message;

        Gtk::Grid   m_layout;

        Gtk::Button m_remove;
        Gtk::Button m_button;

        struct Noti {
            Glib::ustring titel;
            Glib::ustring message;
            Glib::ustring button;
            std::function<void(void)> callback;
        };

        std::vector<Noti> m_messages;

        std::function<void(void)> m_callback;

    public:
        WidgetNotification();
        ~WidgetNotification();

        void add_notification(Glib::ustring titel, Glib::ustring message, Glib::ustring button = "", std::function<void(void)> callback = [](){});

    protected:
        void on_button_clicked();
};

#endif
