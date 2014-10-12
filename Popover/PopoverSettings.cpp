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
#include "PopoverSettings.h"
#include "Generated/icon.h"
#include "Dialog/DialogContact.h"

PopoverSettings::PopoverSettings(const Widget& relative_to): Gtk::Popover(relative_to) {
    add_label("Settings");
}

PopoverSettings::~PopoverSettings() {

}

void PopoverSettings::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    //update data
    if (!visible) {
        return;
    }

}
