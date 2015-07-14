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
#include "tox/types.h"
#include "widget/main_menu.h"
#include "config.h"
#include "storage.h"

namespace dialog {
    // contact list with pinned chat
    class main : public Gtk::Window {
        private:
            std::shared_ptr<toxmm2::core> m_toxcore;

            //Glib::RefPtr<Gtk::StatusIcon> m_status_icon;

            Gtk::HeaderBar* m_headerbar;
            Gtk::MenuButton* m_btn_status;
            Gtk::Stack* m_stack_header;
            Gtk::Stack* m_stack;
            Gtk::ListBox* m_list_contact;
            Gtk::ListBox* m_list_contact_active;
            Gtk::ListBox* m_list_notify;
            Gtk::ScrolledWindow* m_list_contact_scroll;

            Gtk::Image* m_status_icon;

            sigc::connection m_update_interval;

            std::string m_config_path;

            Glib::RefPtr<Glib::Binding> m_binding_title;
            Glib::RefPtr<Glib::Binding> m_binding_subtitle;
            Glib::RefPtr<Glib::Binding> m_binding_position;
            Glib::RefPtr<Glib::Binding> m_binding_contact_active;

            Gtk::Menu m_popup_menu;

            Glib::RefPtr<widget::main_menu> m_menu;

            Glib::RefPtr<Gio::SimpleActionGroup> m_action;

            std::vector<std::pair<Gtk::Widget*, Gtk::Widget*>> m_stack_data;

            std::shared_ptr<class config> m_config;

        public:
            main(BaseObjectType* cobject,
                 utils::builder builder,
                 const Glib::ustring& file);
            ~main();

            std::shared_ptr<toxmm2::core>& tox();

            static utils::builder::ref<main> create(const Glib::ustring& file);

            void chat_add   (Gtk::Widget& headerbar, Gtk::Widget& body, Gtk::Button& prev, Gtk::Button& next);
            void chat_remove(Gtk::Widget& headerbar, Gtk::Widget& body);
            void chat_show  (Gtk::Widget& headerbar, Gtk::Widget& body, Gtk::Button& prev, Gtk::Button& next);

            void exit();

            std::shared_ptr<class config>& config();

        protected:
            void load_contacts();

            class storage : public toxmm2::storage {
                    friend class main;
                private:
                    std::string m_prefix;

                public:
                    storage();

                protected:
                    void set_prefix_key(const std::string& prefix) override;
                    void load(const std::initializer_list<std::string>& key, std::vector<uint8_t>& data) override;
                    void save(const std::initializer_list<std::string>& key, const std::vector<uint8_t>& data) override;

                    std::string get_path_for_key(const std::initializer_list<std::string>& key);
            };

            std::shared_ptr<storage> m_storage;
    };
}
#endif
