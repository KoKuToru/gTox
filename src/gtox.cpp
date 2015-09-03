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
#include "gtox.h"
#include <glibmm/i18n.h>
#include <iostream>
#include <cstring>

#include "dialog/error.h"
#include "dialog/profile_selection.h"
#include "dialog/profile_create.h"
#include "dialog/main.h"

#include "utils/debug.h"

#include "config.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

Glib::RefPtr<gTox> gTox::m_instance;

gTox::gTox()
    : Gtk::Application("org.gtox",
                       Gio::ApplicationFlags(Gio::APPLICATION_HANDLES_OPEN | Gio::APPLICATION_HANDLES_COMMAND_LINE)),
      m_config_path(Glib::build_filename(Glib::get_user_config_dir(), "tox")),
      m_avatar_path(Glib::build_filename(m_config_path, "avatars")),
      m_config_global_path(Glib::build_filename(Glib::get_user_config_dir(), "gtox")) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    Glib::set_application_name(_("gTox"));

    if (!Glib::file_test(m_config_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_config_path)->make_directory_with_parents();
    }

    if (!Glib::file_test(m_avatar_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_avatar_path)->make_directory_with_parents();
    }

    if (!Glib::file_test(m_config_global_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(m_config_global_path)->make_directory_with_parents();
    }

    Gtk::IconTheme::get_default()
            ->add_resource_path("/org/gtox/icon");

    m_theme_binding = Glib::Binding::bind_property(config::global().property_theme_color(),
                                                   Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme(),
                                                   Glib::BINDING_DEFAULT | Glib::BINDING_BIDIRECTIONAL | Glib::BINDING_SYNC_CREATE,
                                                   [](const int& value_in, bool& value_out) {
                                                       utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in, value_out });
                                                       value_out = value_in == 0;
                                                       return true;
                                                   },
                                                   [](const bool& value_in, int& value_out) {
                                                       utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in, value_out });
                                                       value_out = value_in ? 0 : 1;
                                                       return true;
                                                   });

    auto css = Gtk::CssProvider::create();
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(
                screen,
                css,
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    auto update_style = [css]() {
        utils::debug::scope_log a(DBG_LVL_2("gtox"), {});
        bool dark = Gtk::Settings::get_default()
                    ->property_gtk_application_prefer_dark_theme();
        css->load_from_resource(dark?"/org/gtox/style/dark.css":"/org/gtox/style/light.css");
    };

    Gtk::Settings::get_default()
            ->property_gtk_application_prefer_dark_theme().signal_changed().connect(update_style);

    update_style();
}

Glib::RefPtr<gTox> gTox::create() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    if (!m_instance) {
        m_instance = Glib::RefPtr<gTox>( new gTox() );
    }
    return m_instance;
}

Glib::RefPtr<gTox> gTox::instance() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_instance;
}

void gTox::on_activate() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    Glib::Dir dir(m_config_path);
    std::vector<std::string> accounts(dir.begin(), dir.end());
    accounts.resize(std::distance(
        accounts.begin(),
        std::remove_if(
            accounts.begin(), accounts.end(), [](const std::string& name) {
                utils::debug::scope_log log(DBG_LVL_3("gtox"), { name });
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
        utils::debug::scope_log log(DBG_LVL_3("gtox"), { a });
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
        utils::debug::scope_log log(DBG_LVL_3("gtox"), { name });
        return Glib::build_filename(m_config_path, name + ".tox");
    });

    // start profile select
    mark_busy();
    auto profile = dialog::profile_selection::create(accounts);
    unmark_busy();

    add_window(*profile);
    profile->present();

    auto profile_ptr = profile.raw();
    if (profile->get_path().empty()) {
        profile->signal_hide().connect_notify([this, profile_ptr](){
            utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
            if (!profile_ptr->get_path().empty()) {
                open(Gio::File::create_for_path(profile_ptr->get_path()));
            } else if (!profile_ptr->is_aborted()) {
                mark_busy();
                auto assistant = dialog::profile_create::create(m_config_path);
                unmark_busy();

                add_window(*assistant);

                assistant->present();

                auto assistant_ptr = assistant.raw();
                assistant->signal_hide().connect_notify([this, assistant_ptr]() {
                    utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
                    Glib::ustring path = assistant_ptr->get_path();
                    remove_window(*assistant_ptr);

                    if (!path.empty()) {
                        open(Gio::File::create_for_path(path));
                    } else {
                        activate();
                    }
                }, true);
            }
            remove_window(*profile_ptr);
        }, true);
        profile->show();
    } else {
        if (!profile->is_aborted()) {
            activate();
        }
    }

    Gtk::Application::on_activate();
}

void gTox::on_open(const Gio::Application::type_vec_files& files,
                   const Glib::ustring& hint) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    //open file !
    for (auto file : files) {
        mark_busy();
        auto tmp = dialog::main::create(file->get_path());
        unmark_busy();
        add_window(*tmp);

        tmp->show();
    }

    Gtk::Application::on_open(files, hint);
}

int gTox::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
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
        /*for(auto w : get_windows()) {
            auto dialog = dynamic_cast<DialogContact*>(w);
            if (dialog) {
                //TODO: invites
            }
        }*/
    }

    if (invites.empty() && files.empty()) {
        activate();
    }

    return EXIT_SUCCESS;
}
