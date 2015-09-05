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

bool translation_working() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    const char* TRANSLATION_CHECK = "TRANSLATION_CHECKER";
    return !(_(TRANSLATION_CHECK) == TRANSLATION_CHECK);
}

bool find_translation_domain() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    // Translation search locations in order of preference
    std::vector<std::string> locations {"./i18n",
        bindtextdomain("gtox", nullptr), // default location
        "/usr/local/share/locale"};

    for (auto l : locations) {
        bindtextdomain("gtox", l.c_str());
        if (translation_working())
            return true;
    }

    return false;
}

bool setup_translation() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    // Translations returns in UTF-8
    bind_textdomain_codeset("gtox", "UTF-8");
    textdomain("gtox");
#if defined _WIN32 || defined __CYGWIN__
    return true;
#else
    return find_translation_domain();
#endif
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

    setup_translation();

    print_copyright();

    Glib::RefPtr<gTox> application = gTox::create();

    const int status = application->run(argc, argv);
    return status;
}
