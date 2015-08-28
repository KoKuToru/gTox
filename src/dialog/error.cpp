/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#include "error.h"
#include <glibmm/i18n.h>
#include <glibmm/markup.h>

using namespace dialog;

error::error(Gtk::Window& parent,bool fatal,std::string title, std::string message):
    MessageDialog(parent, title, false, fatal?Gtk::MESSAGE_ERROR:Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true) {
    set_secondary_text(((fatal)?_("gTox can't continue.\n"
                                  "The following problem occurred:\n"
                                  "\n"
                                  "<i><b>"):"")
                       + Glib::Markup::escape_text(message)
                       + ((fatal)?(Glib::Markup::escape_text("\n\n")
                                   + _("</b></i>\n"
                                       "\n"
                                       "Please make sure that you are using the newest gTox version.\n"
                                       "If you do use the newest, please report the error to us.")):""), true);

    if (fatal) {
        add_button(_("Report"), 213);
    }
}

error::error(bool fatal,std::string title, std::string message):
    MessageDialog(title, false, fatal?Gtk::MESSAGE_ERROR:Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true) {
    set_secondary_text(((fatal)?_("gTox can't continue.\n"
                                  "The following problem occurred:\n"
                                  "\n"
                                  "<i><b>"):"")
                       + Glib::Markup::escape_text(message)
                       + ((fatal)?(Glib::Markup::escape_text("\n\n")
                                   + _("</b></i>\n"
                                       "\n"
                                       "Please make sure that you are using the newest gTox version.\n"
                                       "If you do use the newest, please report the error to us.")):""), true);

    if (fatal) {
        add_button(_("Report"), 213);
    }
}

error::~error() {
}

int error::run() {
    switch(MessageDialog::run()) {
        case 213:
            if (!Gio::AppInfo::launch_default_for_uri("https://github.com/KoKuToru/gTox/issues/new")) {
                //TODO !
            }
            //continue to run:
            run();
            break;
    }

    return 0;
}
