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
#ifndef WIDGETCONTACTLISTITEM_H
#define WIDGETCONTACTLISTITEM_H

#include <gtkmm.h>
#include "widget/avatar.h"
#include "tox/types.h"
#include "utils/builder.h"
#include "utils/dispatcher.h"

namespace dialog {
    class main;
    class chat;
}

namespace widget {
    class contact : public Gtk::ListBoxRow {
        private:
            dialog::main& m_main;
            Glib::RefPtr<dialog::chat> m_chat;

            utils::dispatcher m_dispatcher;

            std::shared_ptr<toxmm::contact> m_contact;

            widget::avatar* m_avatar;
            widget::avatar* m_avatar_mini;

            Gtk::Label* m_name;
            Gtk::Label* m_status_msg;
            Gtk::Image* m_status_icon;
            Gtk::Spinner* m_spin;

            Gtk::Label* m_name_mini;
            Gtk::Label* m_status_msg_mini;
            Gtk::Image* m_status_icon_mini;
            Gtk::Spinner* m_spin_mini;

            Gtk::Widget* m_contact_list_grid_mini;
            Gtk::Widget* m_contact_list_grid;

            Gtk::Revealer* m_revealer;

            Glib::RefPtr<Glib::Binding> m_bindings[7];

            bool m_for_active_chats;

        public:
            contact(BaseObjectType* cobject,
                    utils::builder builder,
                    dialog::main& main,
                    std::shared_ptr<toxmm::contact> contact,
                    bool for_active_chats=false);
            virtual ~contact();

            static utils::builder::ref<contact> create(dialog::main& main,
                                                       std::shared_ptr<toxmm::contact> contact,
                                                       bool for_active_chats=false);

            //! for sort
            int compare(contact* other);

            std::shared_ptr<toxmm::contact> get_contact();
            void activated();

        protected:
            void on_show();
            void on_hide();
    };
}
#endif
