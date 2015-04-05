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
#ifndef TOXMM_EVENT_H
#define TOXMM_EVENT_H
#include <string>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <vector>
#include "toxcore/toxcore/tox.h"

class ToxEvent {
  private:
        std::type_index m_type;
        std::shared_ptr<std::vector<char>> m_data;

    public:
        ToxEvent(): m_type(typeid(void)) {}
        template<typename T> ToxEvent(T data):
            m_type(typeid(T)),
            m_data(new std::vector<char>(sizeof(T)),
                   [](std::vector<char>* v) {
                       ((T*)v->data())->~T();
                       delete v;
                   }) {
            //init data:
            new (m_data->data()) T(data);
        }

        std::type_index type() const noexcept {
            return m_type;
        }
        template<typename T> const T get() const {
            if (type() != typeid(T)) {
                throw std::bad_cast();
            }
            return *(const T*)m_data->data();
        }
};


#endif
