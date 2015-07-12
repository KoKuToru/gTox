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

#include "utils/builder.h"
#include "config.h"

namespace dialog {
    class main;
    class settings : public Gtk::Window {
        private:
            utils::builder m_builder;
            Glib::RefPtr<main> m_main;

            Gtk::HeaderBar* m_headerbar_attached;
            Gtk::HeaderBar* m_headerbar_detached;
            Gtk::Widget* m_body;

            Gtk::Button* m_btn_detach;
            Gtk::Button* m_btn_attach;
            Gtk::Button* m_btn_prev;
            Gtk::Button* m_btn_next;
            Gtk::Button* m_btn_close_attached;
            Gtk::Button* m_btn_close_detached;

            Gtk::Switch* m_tray_visible;
            Gtk::Switch* m_tray_on_start;
            Gtk::Switch* m_tray_on_close;

            Gtk::Switch*   m_c_n_on_message;
            Gtk::Switch*   m_c_n_with_audio;
            Gtk::ComboBox* m_c_auto_away;
            Gtk::Switch*   m_c_send_typing;
            Gtk::Switch*   m_c_logging;

            Gtk::FileChooserButton* m_ft_save_to;
            Gtk::Switch*            m_ft_auto_accept;
            Gtk::Switch*            m_ft_display_inline;

            Gtk::Switch* m_cl_use_compact;
            Gtk::Switch* m_cl_display_active;

            Gtk::Switch*   m_connection_udp;
            Gtk::Switch*   m_connection_tcp;
            Gtk::ComboBox* m_proxy_type;
            Gtk::Entry*    m_proxy_host;
            Gtk::Entry*    m_proxy_port;

            Gtk::ComboBox* m_t_color;

            Gtk::Switch* m_p_remember;

            Gtk::ComboBox* m_video_device;

            std::vector<Glib::RefPtr<Glib::Binding>> m_bindings;

        public:
            settings(Glib::RefPtr<main> main);
            ~settings();

            void activated();
    };
}
#endif
