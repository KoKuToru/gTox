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
#ifndef TOXMM_EXCEPTION_H
#define TOXMM_EXCEPTION_H
#include <string>
#include <typeinfo>
#include <typeindex>
#include <tox/tox.h>

class ToxException: public std::exception {
  private:
        const std::type_index m_type;
        std::string m_what;
        int m_what_id;
    public:
        ToxException(const std::string& what);
        ToxException(TOX_ERR_OPTIONS_NEW error);
        ToxException(TOX_ERR_NEW error);
        ToxException(TOX_ERR_BOOTSTRAP error);
        ToxException(TOX_ERR_FRIEND_ADD error);
        ToxException(TOX_ERR_FRIEND_BY_PUBLIC_KEY error);
        ToxException(TOX_ERR_FRIEND_DELETE error);
        ToxException(TOX_ERR_FRIEND_SEND_MESSAGE error);
        ToxException(TOX_ERR_SET_INFO error);
        ToxException(TOX_ERR_FRIEND_QUERY error);
        ToxException(TOX_ERR_SET_TYPING error);
        ToxException(TOX_ERR_FRIEND_GET_PUBLIC_KEY error);
        ToxException(TOX_ERR_FRIEND_GET_LAST_ONLINE error);
        ToxException(TOX_ERR_FILE_CONTROL error);
        ToxException(TOX_ERR_FILE_SEEK error);

        virtual const char* what() const noexcept;
        virtual std::type_index type() const noexcept;
        virtual int what_id() const noexcept;
};


#endif
