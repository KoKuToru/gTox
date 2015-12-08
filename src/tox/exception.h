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
#ifndef TOXMM2_EXCEPTION_H
#define TOXMM2_EXCEPTION_H
#include <string>
#include <typeinfo>
#include <typeindex>
#include <tox/tox.h>
#include <tox/toxav.h>

namespace toxmm {
    class exception: public std::exception {
        private:
            const std::type_index m_type;
            std::string m_what;
            int m_what_id;
        public:
            exception(const std::string& what);
            exception(TOX_ERR_OPTIONS_NEW error);
            exception(TOX_ERR_NEW error);
            exception(TOX_ERR_BOOTSTRAP error);
            exception(TOX_ERR_FRIEND_ADD error);
            exception(TOX_ERR_FRIEND_BY_PUBLIC_KEY error);
            exception(TOX_ERR_FRIEND_DELETE error);
            exception(TOX_ERR_FRIEND_SEND_MESSAGE error);
            exception(TOX_ERR_SET_INFO error);
            exception(TOX_ERR_FRIEND_QUERY error);
            exception(TOX_ERR_SET_TYPING error);
            exception(TOX_ERR_FRIEND_GET_PUBLIC_KEY error);
            exception(TOX_ERR_FRIEND_GET_LAST_ONLINE error);
            exception(TOX_ERR_FILE_CONTROL error);
            exception(TOX_ERR_FILE_SEEK error);
            exception(TOX_ERR_FILE_GET error);
            exception(TOX_ERR_FILE_SEND error);
            exception(TOX_ERR_FILE_SEND_CHUNK error);

            exception(TOXAV_ERR_NEW error);
            exception(TOXAV_ERR_CALL error);
            exception(TOXAV_ERR_ANSWER error);
            exception(TOXAV_ERR_CALL_CONTROL error);
            exception(TOXAV_ERR_BIT_RATE_SET error);
            exception(TOXAV_ERR_SEND_FRAME error);

            virtual const char* what() const noexcept;
            virtual std::type_index type() const noexcept;
            virtual int what_id() const noexcept;
    };
}
#endif
