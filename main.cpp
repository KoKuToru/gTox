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
#include "Dialog/FirstStartAssistant.h"
#include "Dialog/DialogError.h"
#include <libnotifymm.h>
#include <glibmm/i18n.h>

#include "Tox/Tox.h"

void print_copyright() {
    std::clog
        << "gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git"
        << std::endl << std::endl << "Copyright (C) 2014  Luca Béla Palkovics"
        << std::endl << "Copyright (C) 2014  Maurice Mohlek" << std::endl
        << std::endl << "This program is free software: you can redistribute "
                        "it and/or modify" << std::endl
        << "it under the terms of the GNU General Public License as published "
           "by" << std::endl
        << "the Free Software Foundation, either version 3 of the License, or"
        << std::endl << "(at your option) any later version." << std::endl
        << std::endl
        << "This program is distributed in the hope that it will be useful,"
        << std::endl
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
        << std::endl
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
        << std::endl << "GNU General Public License for more details."
        << std::endl << std::endl
        << "You should have received a copy of the GNU General Public License"
        << std::endl << "along with this program.  If not, see "
                        "<http://www.gnu.org/licenses/>" << std::endl
        << std::endl;
}

static const char* TRANSLATION_CHECK = "OKAY";

bool try_setup_translation(const char* lng) {
    // https://www.gnu.org/software/gettext/manual/html_node/gettext-grok.html

    if (lng != nullptr) {
        /* Change language.  */
        setenv("LANGUAGE", lng, 1);

        /* Make change known.  */
        {
            extern int _nl_msg_cat_cntr;
            ++_nl_msg_cat_cntr;
        }
    }

    bind_textdomain_codeset("gTox", "UTF-8");
    textdomain("gTox");

    static std::string original_locale = bindtextdomain("gTox", nullptr);

    // use ./Locale if possible
    if (Glib::file_test("./Locale", Glib::FILE_TEST_IS_DIR)) {
        bindtextdomain("gTox", "./Locale");
    }
    // try if default is loading
    if (gettext(TRANSLATION_CHECK) == TRANSLATION_CHECK) {
        // change to /usr/local/share/locale ..
        bindtextdomain("gTox", "/usr/local/share/locale");
        // try again
        if (gettext(TRANSLATION_CHECK) == TRANSLATION_CHECK) {
            // back to original
            bindtextdomain("gTox", original_locale.c_str());
            return false;
        }
    }

    return true;
}

void terminate_handler() {
    std::exception_ptr exptr = std::current_exception();
    try {
        std::rethrow_exception(exptr);
    } catch (SQLite::Exception &ex) {
        DialogError(true, "Fatal Unexpected Sqlite Exception", ex.what()).run();
    } catch (std::string &ex) {
        DialogError(true, "Fatal Unexpected String Exception", ex).run();
    } catch (Tox::Exception &ex) {
        DialogError(true, "Fatal Unexpected Tox Exception", "Code: " + std::to_string(ex.code)).run();
    } catch (std::exception &ex) {
        DialogError(true, "Fatal Unexpected Exception", ex.what()).run();
    } catch (...) {
        DialogError(true, "Fatal Unexpected Exception", "unknow exception !").run();
    }

}

int main(int argc, char* argv[]) {
    std::set_terminate(terminate_handler);

    Gtk::Main kit(argc, argv);

    Gtk::Settings::get_default()
            ->property_gtk_application_prefer_dark_theme() = true;
    Glib::set_application_name("gTox");

    if (!try_setup_translation(nullptr) && !try_setup_translation("en")) {
        DialogError(false, "Fatal Error", "Couldn't find translation files").run();
        return -1;
    }

    print_copyright();

    Notify::init("gTox");

    std::string config_path
        = Glib::build_filename(Glib::get_user_config_dir(), "gTox");
    if (!Glib::file_test(config_path, Glib::FILE_TEST_IS_DIR)) {
        Gio::File::create_for_path(config_path)->make_directory();
    }

    Glib::Dir dir(config_path);
    std::vector<std::string> accounts(dir.begin(), dir.end());
    accounts.resize(std::distance(
        accounts.begin(),
        std::remove_if(
            accounts.begin(), accounts.end(), [](const std::string& name) {
                const std::string state_ext = ".state";
                return !(name.size() > state_ext.size()
                         && name.substr(name.size() - state_ext.size(),
                                        state_ext.size()) == state_ext);
            })));

    if (accounts.empty()) {
        // start new account assistant
        FirstStartAssistant assistant(config_path);
        kit.run(assistant);

        if (assistant.isAborted()) {
            return 0;
        }

        config_path = assistant.getPath();
    } else if (accounts.size() > 1) {
        // start user select
        // TODO
        config_path = Glib::build_filename(config_path, accounts.front());
        Tox::instance().init(config_path);
    } else {
        config_path = Glib::build_filename(config_path, accounts.front());
        Tox::instance().init(config_path);
    }

    DialogContact::init(config_path);
    kit.run(DialogContact::instance());
    DialogContact::destroy();
    return 0;
}
