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
#include "DialogError.h"
#include <glibmm/i18n.h>
#include <glibmm/markup.h>

DialogError::DialogError(bool fatal,std::string title, std::string message):
    m_fatal(fatal),
    MessageDialog(title, false, fatal?Gtk::MESSAGE_ERROR:Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true) {
    set_secondary_text(((fatal)?_("ERROR_REPORT_TEXT_INTRO"):"")
                       + Glib::Markup::escape_text(message)
                       + ((fatal)?_("ERROR_REPORT_TEXT_OUTRO"):""), true);

    if (fatal) {
        add_button(_("ERROR_REPORT_BTN"), 213);
    }
}

DialogError::~DialogError() {
}

int DialogError::run() {
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
