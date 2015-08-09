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
#ifndef WIDGET_CHAT_BUBBLE_H
#define WIDGET_CHAT_BUBBLE_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include "tox/types.h"

namespace widget {
    class avatar;
    class chat_bubble: public Gtk::Revealer {
        private:
            utils::dispatcher m_dispatcher;

            avatar*     m_avatar;
            Gtk::Box*   m_row_box;
            Gtk::Label* m_username;
            Gtk::Frame* m_frame;

            Glib::RefPtr<Glib::Binding> m_binding_name;

            void init(utils::builder builder);

        public:
            chat_bubble(BaseObjectType* cobject,
                        utils::builder builder,
                        Glib::PropertyProxy_ReadOnly<Glib::ustring> name,
                        Glib::PropertyProxy_ReadOnly<toxmm2::contactAddrPublic> addr,
                        Glib::DateTime time);
            ~chat_bubble();

            void add_row(Gtk::Widget& widget);

            static utils::builder::ref<chat_bubble> create(std::shared_ptr<toxmm2::core> core, Glib::DateTime time);
            static utils::builder::ref<chat_bubble> create(std::shared_ptr<toxmm2::contact> contact, Glib::DateTime time);
    };
}

#endif
