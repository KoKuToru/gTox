/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#ifndef TOXMM2_STORAGE_H
#define TOXMM2_STORAGE_H

#include <glibmm.h>

namespace toxmm2 {

    /**
     * @brief toxmm will use this class to save data it will need to
     * permanently store somewhere
     */
    class storage {
        public:
            /**
             * @brief saves the data somehow
             *
             * @param unique key for the data
             * @param data
             */
            virtual void save(const std::initializer_list<std::string>& key, const std::vector<uint8_t>& data) = 0;
            /**
             * @brief loads the data somehow
             *
             * If not data is found for the given key,
             * data.size() should be set to zero.
             *
             * @param unique key for the data
             * @param data
             */
            virtual void load(const std::initializer_list<std::string>& key, std::vector<uint8_t>& data) = 0;
    };
}


#endif
