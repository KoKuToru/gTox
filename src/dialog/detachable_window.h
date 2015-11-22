/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#ifndef DIALOGDETACHABLEWINDOW_H
#define DIALOGDETACHABLEWINDOW_H

#include <gtkmm.h>
#include "utils/debug.h"
#include "utils/builder.h"

namespace dialog {

    class main;

    class detachable_window
            : public Gtk::Window,
              public utils::debug::track_obj<detachable_window> {
        public:
            //props
            auto property_has_focus()          -> Glib::PropertyProxy<bool>;
            auto property_is_attached()        -> Glib::PropertyProxy<bool>;
            auto property_headerbar_title()    -> Glib::PropertyProxy<Glib::ustring>;
            auto property_headerbar_subtitle() -> Glib::PropertyProxy<Glib::ustring>;
            auto property_body()               -> Glib::PropertyProxy<Gtk::Widget*>;
            auto property_headerbar()          -> Glib::PropertyProxy_ReadOnly<Gtk::HeaderBar*>;

            using type_slot_detachable_add = sigc::slot<void, detachable_window*>;
            using type_slot_detachable_del = sigc::slot<void, detachable_window*>;
            using type_signal_close        = sigc::signal<void>;

            type_signal_close signal_close();

            detachable_window(type_slot_detachable_add main_add,
                              type_slot_detachable_del main_del);
            virtual ~detachable_window();

            void present();

        private:
            type_slot_detachable_add m_attach;

            Gtk::HeaderBar* m_headerbar_attached;
            Gtk::HeaderBar* m_headerbar_detached;
            Gtk::Button* m_btn_attach;
            Gtk::Button* m_btn_detach;
            Gtk::Button* m_btn_close_attached;
            Gtk::Button* m_btn_close_detached;

            std::vector<Glib::RefPtr<Glib::Binding>> m_bindings;

            Glib::Property<bool>            m_prop_has_focus;
            Glib::Property<bool>            m_prop_is_attached;
            Glib::Property<Glib::ustring>   m_prop_headerbar_title;
            Glib::Property<Glib::ustring>   m_prop_headerbar_subtitle;
            Glib::Property<Gtk::Widget*>    m_prop_body;
            Glib::Property<Gtk::HeaderBar*> m_prop_headerbar;

            type_signal_close m_signal_close;

            sigc::connection m_con_map;
            sigc::connection m_con_unmap;
            sigc::connection m_con_active;
    };
}

#endif
