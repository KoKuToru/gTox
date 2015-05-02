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
#ifndef WIDGETPOPOVERSETTINGS_H
#define WIDGETPOPOVERSETTINGS_H

#include <gtkmm.h>
#include "Dialog/DialogSettings.h"
#include "Widget/Settings/WidgetProfile.h"
#include "Helper/gToxObserver.h"
#include "Helper/gToxBuilder.h"
#include "Widget/WidgetAvatar.h"

class PopoverSettings : public Gtk::Popover, public gToxObserver {
    private:
        gToxBuilder m_builder;
        //DialogSettings m_settings;

        Gtk::Entry* m_username;
        Gtk::Entry* m_status;
        WidgetAvatar* m_avatar;

        Gtk::Stack* m_stack;

    public:
        PopoverSettings(gToxObservable* observable,
                        const Gtk::Widget& relative_to);
        ~PopoverSettings();

        void set_visible(bool visible = true);
};

#endif
