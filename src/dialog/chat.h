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
#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include "tox/types.h"
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include <memory>

namespace widget {
    class chat_input;
}

namespace dialog {
    class main;
    class chat : public Gtk::Window {
        private:
            std::shared_ptr<toxmm2::contact> m_contact;

            Glib::RefPtr<main> m_main;
            utils::dispatcher m_dispatcher;
            utils::builder    m_builder;

            Gtk::HeaderBar* m_headerbar_attached;
            Gtk::HeaderBar* m_headerbar_detached;
            Gtk::Widget* m_body;

            Gtk::Button* m_btn_attach;
            Gtk::Button* m_btn_detach;
            Gtk::Button* m_btn_prev;
            Gtk::Button* m_btn_next;

            widget::chat_input* m_input;
            Gtk::Revealer* m_input_revealer;
            Gtk::Revealer* m_input_format_revealer;

            Glib::RefPtr<Glib::Binding> m_binding_name[2];
            Glib::RefPtr<Glib::Binding> m_binding_status[2];
            Glib::RefPtr<Glib::Binding> m_binding_online;
            Glib::RefPtr<Glib::Binding> m_binding_focus;

        public:
            chat(Glib::RefPtr<main> main, std::shared_ptr<toxmm2::contact> contact);
            ~chat();

            void activated();
    };
}

#endif
