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
#include "gTox.h"
#include <glibmm/i18n.h>
#include <iostream>
#include <cstring>

#include "Dialog/DialogContact.h"
#include "Dialog/DialogProfile.h"
#include "Dialog/DialogProfileCreate.h"

Glib::RefPtr<gTox> gTox::m_instance;

gTox::gTox()
    : Gtk::Application("org.gtox",
                       Gio::ApplicationFlags(Gio::APPLICATION_HANDLES_OPEN | Gio::APPLICATION_HANDLES_COMMAND_LINE)),
      m_config_path(Glib::build_filename(Glib::get_user_config_dir(), "tox")),
      m_avatar_path(Glib::build_filename(m_config_path, "avatars")),
      m_config_global_path(Glib::build_filename(Glib::get_user_config_dir(), "gtox")) {

    Glib::set_application_name(_("APPLICATION_NAME"));

    if (!Glib::file_test(m_config_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_config_path)->make_directory();
    }

    if (!Glib::file_test(m_avatar_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_avatar_path)->make_directory();
    }

    if (!Glib::file_test(m_config_global_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_config_global_path)->make_directory();
    }

    m_config.open(Glib::build_filename(m_config_global_path, "config.sqlite"));

    Gtk::IconTheme::get_default()
            ->add_resource_path("/org/gtox/icon");

    Gtk::Settings::get_default()
            ->property_gtk_application_prefer_dark_theme() = database().config_get("SETTINGS_THEME_COLOR", 0) == 0;

    auto css = Gtk::CssProvider::create();
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(
                screen,
                css,
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    auto update_style = [css]() {
        bool dark = Gtk::Settings::get_default()
                    ->property_gtk_application_prefer_dark_theme();
        css->load_from_resource(dark?"/org/gtox/style/dark.css":"/org/gtox/style/light.css");
    };

    Gtk::Settings::get_default()
            ->property_gtk_application_prefer_dark_theme().signal_changed().connect(update_style);

    update_style();
}

Glib::RefPtr<gTox> gTox::create() {
    if (!m_instance) {
        m_instance = Glib::RefPtr<gTox>( new gTox() );
    }
    return m_instance;
}

Glib::RefPtr<gTox> gTox::instance() {
    return m_instance;
}

void gTox::on_activate() {

    Glib::Dir dir(m_config_path);
    std::vector<std::string> accounts(dir.begin(), dir.end());
    accounts.resize(std::distance(
        accounts.begin(),
        std::remove_if(
            accounts.begin(), accounts.end(), [](const std::string& name) {
                std::string state_ext = ".tox";
                bool f_tox = !(name.size() > state_ext.size()
                               && name.substr(name.size() - state_ext.size(),
                                        state_ext.size()) == state_ext);
                bool f_old_tox = (name != "tox_save");
                return f_tox && f_old_tox;
            })));

    //filter files with same name .tox
    //1. remove extension
    std::transform(accounts.begin(), accounts.end(), accounts.begin(), [](std::string a) {
        auto a_p = a.find_last_of(".");
        if (a_p != std::string::npos) {
            a.resize(a_p);
        }
        return a;
    });
    //2. sort
    std::sort(accounts.begin(), accounts.end());
    //3. remove duplicates
    accounts.erase(std::unique(accounts.begin(), accounts.end()), accounts.end());
    //4. make the full paths
    std::transform(accounts.begin(), accounts.end(), accounts.begin(), [this](const std::string& name) {
        return Glib::build_filename(m_config_path, name + ".tox");
    });

    // start profile select
    mark_busy();
    auto profile = DialogProfile::create(accounts);
    unmark_busy();

    add_window(*profile);
    profile->present();

    if (profile->get_path().empty()) {
        profile->signal_hide().connect_notify([this, profile](){
            remove_window(*profile);

            if (!profile->get_path().empty()) {
                open(Gio::File::create_for_path(profile->get_path()));
            } else if (!profile->is_aborted()) {
                mark_busy();
                auto assistant = DialogProfileCreate::create(m_config_path);
                unmark_busy();

                add_window(*assistant);

                assistant->present();

                assistant->signal_hide().connect_notify([this, assistant]() {
                    remove_window(*assistant);

                    //if (!assistant->is_aborted()) {
                        if (!assistant->get_path().empty()) {
                            open(Gio::File::create_for_path(assistant->get_path()));
                        } else {
                            activate();
                        }
                    //}

                    delete assistant;
                });
            }

            delete profile;
        });
        profile->show();
    } else {
        if (!profile->is_aborted()) {
            activate();
        }
        delete profile;
    }

    Gtk::Application::on_activate();
}

void gTox::on_open(const Gio::Application::type_vec_files& files,
                   const Glib::ustring& hint) {
    //open file !
    for (auto file : files) {
        mark_busy();
        auto tmp = DialogContact::create(file->get_path());
        unmark_busy();
        add_window(*tmp);

        tmp->show();
    }

    Gtk::Application::on_open(files, hint);
}

int gTox::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
    int argc = 0;
    auto argv = command_line ? command_line->get_arguments(argc) : nullptr;
    std::vector<std::string> arguments;
    for(int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    Gio::Application::type_vec_files files;
    std::vector<std::string> invites;
    for (auto value : arguments) {
        if (Glib::str_has_prefix(value, "tox://")) {
            invites.push_back(value.substr(strlen("tox://")));
            std::clog << "Invite: " << invites.back() << std::endl;
        } else if (Glib::str_has_prefix(value, "tox:")) {
            invites.push_back(value.substr(strlen("tox:")));
            std::clog << "Invite: " << invites.back() << std::endl;
        } else {
            files.push_back(Gio::File::create_for_path(value));
            std::clog << "Open: " << value << std::endl;
        }
    }

    if (!files.empty()) {
        open(files);
    }

    if (!invites.empty()) {
        //invite.. send signal too all ?
        for(auto w : get_windows()) {
            auto dialog = dynamic_cast<DialogContact*>(w);
            if (dialog) {
                /*
                 * TODO INVITE !
                 *
                dialog->
                */
            }
        }
    }

    if (invites.empty() && files.empty()) {
        activate();
    }

    return EXIT_SUCCESS;
}

ToxDatabase& gTox::database() {
    return m_config;
}
