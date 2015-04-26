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
#ifndef H_GTOX_CHILD
#define H_GTOX_CHILD
#include "Tox/Toxmm.h"
#include "gToxObservable.h"
class gToxObserver {
    private:
        gToxObservable* m_observable = nullptr;
        gToxObservable::Handler m_virtual_handler;

    protected:
        void set_observable(gToxObservable* observable);

    public:
        gToxObserver();
        gToxObserver(gToxObservable* observable);

        Toxmm& tox();
        gToxObservable* observable();

        //proxy functions:
        gToxObservable::Handler observer_add(gToxObservable::EFunc callback) {
            return observable()->observer_add(callback);
        }

        /**
         * @brief Call all installed callbacks with the event data
         * @param data
         */
        void observer_notify(const ToxEvent& data) {
            observable()->observer_notify(data);
        }

        virtual void observer_handle(const ToxEvent&) {
            //nothing
        }
};
#endif
