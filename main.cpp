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
#include "toxcore/toxcore/tox.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

#include <gtkmm.h>
#include "Dialog/DialogContact.h"

#include "Tox/Tox.h"

int main(int argc, char *argv[]) {
    Gtk::Main kit(argc, argv);
    Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() = true;

    Glib::set_application_name("gTox");

    std::string config_path = Glib::build_filename(Glib::get_user_config_dir(), "gTox");
    if (!Glib::file_test(config_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(config_path)->make_directory();
    }

    Glib::Dir dir(config_path);
    std::vector<std::string> accounts(dir.begin(), dir.end());
    accounts.resize(std::distance(accounts.begin(), std::remove_if(accounts.begin(), accounts.end(), [](const std::string& name) {
        const std::string state_ext = ".state";
        return !(name.size() > state_ext.size() && name.substr(name.size()-state_ext.size(), state_ext.size()) == state_ext);
    })));

    if (accounts.empty()) {
        //start new account assistant
        //TODO
        Tox::instance().init();
        Tox::instance().save(Glib::build_filename(config_path, "demo.state"));
    } else if (accounts.size() > 1) {
        //start user select
        //TODO
        Tox::instance().init(accounts.front());
    } else {
        Tox::instance().init(accounts.front());

    }

    DialogContact dialog;
    kit.run(dialog);
    return 0;
}
