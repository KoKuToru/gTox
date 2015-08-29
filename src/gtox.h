/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#ifndef GTOX_APPLICATION
#define GTOX_APPLICATION
#include <gtkmm.h>

class gTox : public Gtk::Application {
    public:
        gTox();
        static Glib::RefPtr<gTox> create();
        static Glib::RefPtr<gTox> instance();

        const std::string m_config_path;
        const std::string m_avatar_path;
        const std::string m_config_global_path;

        Glib::RefPtr<Glib::Binding> m_theme_binding;

    protected:
        void on_activate() override;
        void on_open(const Gio::Application::type_vec_files& files,
                     const Glib::ustring& hint) override;
        int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

    private:
        static Glib::RefPtr<gTox> m_instance;
};

#endif
