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
#include "gToxInstance.h"
#include "gToxChild.h"

Toxmm& gToxInstance::tox() {
    return m_tox;
}

gToxInstance::CallbackHandler gToxInstance::add_observer(EFunc callback) {
    return {this, callback};
}

void gToxInstance::install(EFunc* func) {
    m_callback_list.push_back(func);
}

void gToxInstance::uninstall(EFunc* func) {
    m_callback_list.erase(std::find(m_callback_list.begin(),
                                    m_callback_list.end(),
                                    func));
}
