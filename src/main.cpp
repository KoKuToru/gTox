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
#include <libnotifymm.h>
#include <glibmm/i18n.h>
#include <glibmm/exception.h>
#include <gstreamermm/init.h>
#include "tox/exception.h"

#include "gTox.h"

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

static const char* TRANSLATION_CHECK = "TRANSLATION_CHECKER";

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

    _("TRANSLATION_CHECKER");

    return true;
}

void terminate_handler() {
    std::exception_ptr exptr = std::current_exception();
    try {
        std::rethrow_exception(exptr);
    } catch (const Glib::Exception &ex) {
        dialog::error(true, "Fatal Unexpected Glib Exception", ex.what()).run();
    } catch (const toxmm2::exception &ex) {
        dialog::error(true, "Fatal Unexpected Tox Exception", ex.what()).run();
    } catch (const std::exception &ex) {
        dialog::error(true, "Fatal Unexpected Exception", ex.what()).run();
    } catch (const std::string &ex) {
        dialog::error(true, "Fatal Unexpected String Exception", ex).run();
    } catch (...) {
        dialog::error(true, "Fatal Unexpected Exception", "unknow exception !").run();
    }
    abort();
}

int main(int argc, char* argv[]) {
    std::set_terminate(terminate_handler);
    Glib::add_exception_handler(sigc::ptr_fun(&terminate_handler));

    Gtk::Main kit(argc, argv);
    Gst::init(argc, argv);
    Notify::init("gTox");

    if (!try_setup_translation(nullptr) && !try_setup_translation("en")) {
        dialog::error(false, "Fatal Error", "Couldn't find translation files").run();
        return -1;
    }

    print_copyright();

    Glib::RefPtr<gTox> application = gTox::create();

    const int status = application->run(argc, argv);
    return status;
}
