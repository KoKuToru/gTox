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
#ifndef DIALOGCONTACT_H
#define DIALOGCONTACT_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/storage.h"
#include "tox/types.h"
#include "widget/main_menu.h"
#include "config.h"
#include "storage.h"
#include "utils/debug.h"

namespace dialog {
    // contact list with pinned chat
    class main : public Gtk::Window, public utils::debug::track_obj<main> {
        private:
            std::shared_ptr<toxmm::core> m_toxcore;

            //Glib::RefPtr<Gtk::StatusIcon> m_status_icon;

            Gtk::HeaderBar* m_headerbar;
            Gtk::MenuButton* m_btn_status;
            Gtk::Stack* m_stack_header;
            Gtk::Stack* m_stack;
            Gtk::ListBox* m_list_contact;
            Gtk::ListBox* m_list_contact_active;
            Gtk::ListBox* m_list_notify;
            Gtk::ScrolledWindow* m_list_contact_scroll;
            Gtk::Revealer* m_request_revealer;
            Gtk::Button*   m_request_btn;

            Gtk::Image* m_status_icon;

            sigc::connection m_update_interval;

            std::string m_config_path;

            Glib::RefPtr<Glib::Binding> m_binding_title;
            Glib::RefPtr<Glib::Binding> m_binding_subtitle;
            Glib::RefPtr<Glib::Binding> m_binding_position;
            Glib::RefPtr<Glib::Binding> m_binding_contact_active;
            Glib::RefPtr<Glib::Binding> m_binding_download_path;

            Gtk::Menu m_popup_menu;

            std::shared_ptr<widget::main_menu> m_menu;

            Glib::RefPtr<Gio::SimpleActionGroup> m_action;

            std::vector<std::pair<Gtk::Widget*, Gtk::Widget*>> m_stack_data;

            std::shared_ptr<class config> m_config;

            std::vector<std::pair<toxmm::contactAddrPublic, Glib::ustring>> m_requests;

        public:
            main(BaseObjectType* cobject,
                 utils::builder builder,
                 const Glib::ustring& file);
            ~main();

            std::shared_ptr<toxmm::core>& tox();

            static utils::builder::ref<main> create(const Glib::ustring& file);

            void chat_add   (Gtk::Widget& headerbar, Gtk::Widget& body, Gtk::Button& prev, Gtk::Button& next);
            void chat_add   (Gtk::Widget& headerbar, Gtk::Widget& body);
            void chat_remove(Gtk::Widget& headerbar, Gtk::Widget& body);
            void chat_show  (Gtk::Widget& headerbar, Gtk::Widget& body, Gtk::Button& prev, Gtk::Button& next);

            void exit();

            std::shared_ptr<class config>& config();

        protected:
            void load_contacts();

            std::shared_ptr<utils::storage> m_storage;
    };
}
#endif
