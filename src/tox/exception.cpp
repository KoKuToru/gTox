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
#include "exception.h"
#include <map>
#include <glibmm/i18n.h>

using namespace toxmm2;

const char* exception::what() const noexcept {
    return m_what.c_str();
}

int exception::what_id() const noexcept {
    return m_what_id;
}

std::type_index exception::type() const noexcept{
    return m_type;
}

exception::exception(const std::string& what):
    m_type(typeid(std::string)),
    m_what(what),
    m_what_id(0) {
}

exception::exception(TOX_ERR_OPTIONS_NEW error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_OPTIONS_NEW_MALLOC, _("TOX_ERR_OPTIONS_NEW_MALLOC")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_OPTIONS_NEW unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_NEW error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_NEW_NULL, _("TOX_ERR_NEW_NULL")},
    {TOX_ERR_NEW_MALLOC, _("TOX_ERR_NEW_MALLOC")},
    {TOX_ERR_NEW_PORT_ALLOC, _("TOX_ERR_NEW_PORT_ALLOC")},
    {TOX_ERR_NEW_PROXY_BAD_TYPE, _("TOX_ERR_NEW_PROXY_BAD_HOST")},
    {TOX_ERR_NEW_PROXY_BAD_HOST, _("TOX_ERR_NEW_PROXY_BAD_HOST")},
    {TOX_ERR_NEW_PROXY_BAD_PORT, _("TOX_ERR_NEW_PROXY_BAD_PORT")},
    {TOX_ERR_NEW_PROXY_NOT_FOUND, _("TOX_ERR_NEW_PROXY_NOT_FOUND")},
    {TOX_ERR_NEW_LOAD_ENCRYPTED, _("TOX_ERR_NEW_LOAD_ENCRYPTED")},
    {TOX_ERR_NEW_LOAD_BAD_FORMAT, _("TOX_ERR_NEW_LOAD_BAD_FORMAT")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_NEW unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_BOOTSTRAP error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_BOOTSTRAP_NULL, _("TOX_ERR_BOOTSTRAP_NULL")},
    {TOX_ERR_BOOTSTRAP_BAD_HOST, _("TOX_ERR_BOOTSTRAP_BAD_HOST")},
    {TOX_ERR_BOOTSTRAP_BAD_PORT, _("TOX_ERR_BOOTSTRAP_BAD_PORT")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_BOOTSTRAP unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_ADD error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_ADD_NULL, _("TOX_ERR_FRIEND_ADD_NULL")},
    {TOX_ERR_FRIEND_ADD_TOO_LONG, _("TOX_ERR_FRIEND_ADD_TOO_LONG")},
    {TOX_ERR_FRIEND_ADD_NO_MESSAGE, _("TOX_ERR_FRIEND_ADD_NO_MESSAGE")},
    {TOX_ERR_FRIEND_ADD_OWN_KEY, _("TOX_ERR_FRIEND_ADD_OWN_KEY")},
    {TOX_ERR_FRIEND_ADD_ALREADY_SENT, _("TOX_ERR_FRIEND_ADD_ALREADY_SENT")},
    {TOX_ERR_FRIEND_ADD_BAD_CHECKSUM, _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM")},
    {TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM, _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM")},
    {TOX_ERR_FRIEND_ADD_MALLOC, _("TOX_ERR_FRIEND_ADD_MALLOC")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_ADD unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_BY_PUBLIC_KEY error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL, _("TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL")},
    {TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND, _("TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_BY_PUBLIC_KEY unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_DELETE error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND, _("TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_DELETE unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_SEND_MESSAGE error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_SEND_MESSAGE_NULL, _("TOX_ERR_FRIEND_SEND_MESSAGE_NULL")},
    {TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND, _("TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND")},
    {TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED, _("TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED")},
    {TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ, _("TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ")},
    {TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG, _("TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG")},
    {TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY, _("TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_SEND_MESSAGE unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_SET_INFO error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_SET_INFO_NULL, _("TOX_ERR_SET_INFO_NULL")},
    {TOX_ERR_SET_INFO_TOO_LONG, _("TOX_ERR_SET_INFO_TOO_LONG")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_SET_INFO unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_QUERY error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_QUERY_NULL, _("TOX_ERR_FRIEND_QUERY_NULL")},
    {TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND, _("TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_QUERY unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_SET_TYPING error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND, _("TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_SET_TYPING unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_GET_PUBLIC_KEY error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND, _("TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_SET_TYPING unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FRIEND_GET_LAST_ONLINE error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND, _("TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FRIEND_GET_LAST_ONLINE unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FILE_CONTROL error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FILE_CONTROL_FRIEND_NOT_FOUND, _("TOX_ERR_FILE_CONTROL_FRIEND_NOT_FOUND")},
    {TOX_ERR_FILE_CONTROL_FRIEND_NOT_CONNECTED, _("TOX_ERR_FILE_CONTROL_FRIEND_NOT_CONNECTED")},
    {TOX_ERR_FILE_CONTROL_NOT_FOUND, _("TOX_ERR_FILE_CONTROL_NOT_FOUND")},
    {TOX_ERR_FILE_CONTROL_NOT_PAUSED, _("TOX_ERR_FILE_CONTROL_NOT_PAUSED")},
    {TOX_ERR_FILE_CONTROL_DENIED, _("TOX_ERR_FILE_CONTROL_DENIED")},
    {TOX_ERR_FILE_CONTROL_ALREADY_PAUSED, _("TOX_ERR_FILE_CONTROL_ALREADY_PAUSED")},
    {TOX_ERR_FILE_CONTROL_SENDQ, _("TOX_ERR_FILE_CONTROL_SENDQ")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FILE_CONTROL unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FILE_SEEK error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FILE_SEEK_FRIEND_NOT_FOUND, _("TOX_ERR_FILE_SEEK_FRIEND_NOT_FOUND")},
    {TOX_ERR_FILE_SEEK_FRIEND_NOT_CONNECTED, _("TOX_ERR_FILE_SEEK_FRIEND_NOT_CONNECTED")},
    {TOX_ERR_FILE_SEEK_NOT_FOUND, _("TOX_ERR_FILE_SEEK_NOT_FOUND")},
    {TOX_ERR_FILE_SEEK_DENIED, _("TOX_ERR_FILE_SEEK_DENIED")},
    {TOX_ERR_FILE_SEEK_INVALID_POSITION, _("TOX_ERR_FILE_SEEK_INVALID_POSITION")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FILE_CONTROL unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FILE_GET error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FILE_GET_FRIEND_NOT_FOUND, _("TOX_ERR_FILE_GET_FRIEND_NOT_FOUND")},
    {TOX_ERR_FILE_GET_NOT_FOUND, _("TOX_ERR_FILE_GET_NOT_FOUND")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FILE_GET unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FILE_SEND error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FILE_SEND_NULL, _("TOX_ERR_FILE_SEND_NULL")},
    {TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND, _("TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND")},
    {TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED, _("TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED")},
    {TOX_ERR_FILE_SEND_NAME_TOO_LONG, _("TOX_ERR_FILE_SEND_NAME_TOO_LONG")},
    {TOX_ERR_FILE_SEND_TOO_MANY, _("TOX_ERR_FILE_SEND_TOO_MANY")}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FILE_SEND unknow error code " + std::to_string(error));
    }
}

exception::exception(TOX_ERR_FILE_SEND_CHUNK error):
    m_type(typeid(error)) {
    static std::map<decltype(error), std::string> enum2str =
    {{TOX_ERR_FILE_SEND_CHUNK_NULL, _("TOX_ERR_FILE_SEND_CHUNK_NULL")},
    {TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_FOUND, _("TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_FOUND")},
    {TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_CONNECTED, _("TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_CONNECTED")},
    {TOX_ERR_FILE_SEND_CHUNK_NOT_FOUND, _("TOX_ERR_FILE_SEND_CHUNK_NOT_FOUND")},
    {TOX_ERR_FILE_SEND_CHUNK_NOT_TRANSFERRING, _("TOX_ERR_FILE_SEND_CHUNK_NOT_TRANSFERRING")},
    {TOX_ERR_FILE_SEND_CHUNK_INVALID_LENGTH, "TOX_ERR_FILE_SEND_CHUNK_INVALID_LENGTH"},
    {TOX_ERR_FILE_SEND_CHUNK_SENDQ, "TOX_ERR_FILE_SEND_CHUNK_SENDQ"},
    {TOX_ERR_FILE_SEND_CHUNK_WRONG_POSITION, "TOX_ERR_FILE_SEND_CHUNK_WRONG_POSITION"}};
    auto iter = enum2str.find(error);
    if (iter != enum2str.end()) {
        m_what = iter->second;
        m_what_id = error;
    } else {
        throw std::runtime_error("TOX_ERR_FILE_SEND_CHUNK unknow error code " + std::to_string(error));
    }
}
