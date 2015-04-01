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
#include <execinfo.h>
#include <cxxabi.h>

DialogError::DialogError(bool fatal,std::string title, std::string message):
    MessageDialog(title, false, fatal?Gtk::MESSAGE_ERROR:Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true) {
    set_secondary_text(((fatal)?_("ERROR_REPORT_TEXT_INTRO"):"")
                       + Glib::Markup::escape_text(message)
                       + ((fatal)?(Glib::Markup::escape_text("\n\n" + get_stacktrace())
                                   + _("ERROR_REPORT_TEXT_OUTRO")):""), true);

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

std::string DialogError::get_stacktrace() {
    return "";

    std::stringstream ss;

    void * array[50];
    int size = backtrace(array, 50);

    char ** messages = backtrace_symbols(array, size);

    // skip first stack frame (points here)
    for (int i = 1; i < size && messages != NULL; ++i)
    {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // find parantheses and +address offset surrounding mangled name
        for (char *p = messages[i]; *p; ++p)
        {
            if (*p == '(')
            {
                mangled_name = p;
            }
            else if (*p == '+')
            {
                offset_begin = p;
            }
            else if (*p == ')')
            {
                offset_end = p;
                break;
            }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin)
        {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int status;
            char * real_name = mangled_name;
            try {
                real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
            } catch (...) {
                //ignore
            }

            // if demangling is successful, output the demangled function name
            if (status == 0)
            {
                ss << "[bt]: (" << i << ") " << messages[i] << " : "
                   << real_name << "+" << offset_begin << offset_end
                   << std::endl;

            }
            // otherwise, output the mangled function name
            else
            {
                ss << "[bt]: (" << i << ") " << messages[i] << " : "
                   << mangled_name << "+" << offset_begin << offset_end
                   << std::endl;
            }

            if (mangled_name != real_name) {
                free(real_name);
            }
        }
        // otherwise, print the whole line
        else
        {
            ss << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }
    }
    ss << std::endl;

    free(messages);

    return ss.str();
}
