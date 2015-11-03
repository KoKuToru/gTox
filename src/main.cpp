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
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

#include <gtkmm.h>
#include "dialog/error.h"
#include <glibmm/i18n.h>
#include <glibmm/exception.h>
#include <gstreamermm/init.h>
#include "tox/exception.h"
#include "utils/debug.h"
#include "gtox.h"

void print_copyright() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
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

bool find_translation_domain(std::string path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    // Translation search locations in order of preference
    std::vector<std::string> locations {
        Glib::build_filename(path, "i18n"),
        Glib::build_filename(path, "..", "share", "locale")
    };

    for (auto l : locations) {
        //check for gtox.mo
        auto mo = Glib::build_filename(l, "en", "LC_MESSAGES", "gtox.mo");
        if (Glib::file_test(mo, Glib::FILE_TEST_EXISTS)) {
            bindtextdomain("gtox", l.c_str());
            return true;
        }
    }

    return false;
}

bool setup_translation(std::string path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    // Translations returns in UTF-8
    bind_textdomain_codeset("gtox", "UTF-8");
    textdomain("gtox");
    return find_translation_domain(path);
}

void terminate_handler() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    std::exception_ptr exptr = std::current_exception();
    try {
        std::rethrow_exception(exptr);
    } catch (const Glib::Exception &ex) {
        dialog::error(true, "Fatal Unexpected Glib Exception", ex.what()).run();
    } catch (const toxmm::exception &ex) {
        dialog::error(true, "Fatal Unexpected Tox Exception", ex.what()).run();
    } catch (const std::exception &ex) {
        dialog::error(true, "Fatal Unexpected Exception", ex.what()).run();
    } catch (const std::string &ex) {
        dialog::error(true, "Fatal Unexpected String Exception", ex).run();
    } catch (...) {
        dialog::error(true, "Fatal Unexpected Exception", "unknow exception !")
            .run();
    }
    abort();
}
#include <assert.h>
int main(int argc, char* argv[]) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                  argc,
                                  utils::debug::parameter(argv, argc),
                              });

    std::set_terminate(terminate_handler);
    Glib::add_exception_handler(sigc::ptr_fun(&terminate_handler));

    Gtk::Main kit(argc, argv);
    Gst::init(argc, argv);

    setup_translation(
                Glib::path_get_dirname(
                    Glib::find_program_in_path(argv[0])));

    print_copyright();

    bool non_unique = false;
    if (argc > 1) {
        non_unique = std::any_of(argv, argv + argc, [](auto x) {
            return std::string("-non-unique") == x;
        });
    }
    Glib::RefPtr<gTox> application = gTox::create(non_unique);

    const int status = application->run(argc, argv);
    return status;
}
