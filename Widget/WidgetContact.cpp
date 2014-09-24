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
#include "WidgetContact.h"

WidgetContact::WidgetContact() {
    //test data
    for(int i = 0; i < 20; ++i) {
        items.emplace_back(new WidgetContactListItem);
        m_list.add(*(items.back().get()));
    }
    //this->add(m_list);
    this->add(m_list);//, true, true);
}

WidgetContact::~WidgetContact() {

}
