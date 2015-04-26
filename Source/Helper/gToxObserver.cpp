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
#include "gToxObserver.h"
#include "gToxObservable.h"
#include <exception>

Toxmm& gToxObserver::tox() {
    if (!m_observable) {
        throw std::runtime_error("gToxChild m_tox_instance == nullptr");
    }
    return m_observable->tox();
}

void gToxObserver::set_observable(gToxObservable* observable) {
    m_observable = observable;
    //install myself
    m_virtual_handler = observer_add([this](const ToxEvent& e){
        observer_handle(e);
    });
}

gToxObserver::gToxObserver() {

}

gToxObserver::gToxObserver(gToxObservable* observable) {
    set_observable(observable);
}

gToxObservable* gToxObserver::observable() {
    return m_observable;
}
