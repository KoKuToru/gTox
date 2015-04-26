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

class gToxInstance;
class gToxChild {
    private:
        gToxInstance* m_tox_instance = nullptr;

    protected:
        void set_instance(gToxInstance* tox_instance);

    public:
        gToxChild();
        gToxChild(gToxInstance* tox_instance);

        Toxmm& tox();
        gToxInstance* instance();
};
#endif
