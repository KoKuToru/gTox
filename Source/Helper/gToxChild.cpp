/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "gToxChild.h"
#include "gToxInstance.h"
#include <exception>

Toxmm& gToxChild::tox() {
    if (!m_tox_instance) {
        throw std::runtime_error("gToxChild m_tox_instance == nullptr");
    }
    return m_tox_instance->tox();
}

void gToxChild::set_instance(gToxInstance* tox_instance) {
    m_tox_instance = tox_instance;
}

gToxChild::gToxChild() {

}

gToxChild::gToxChild(gToxInstance* tox_instance) {
    set_instance(tox_instance);
}

gToxInstance* gToxChild::instance() {
    return m_tox_instance;
}
