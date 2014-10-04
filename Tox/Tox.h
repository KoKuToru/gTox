/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
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
#ifndef TOX_H
#include "../toxcore/toxcore/tox.h"
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <glibmm/ustring.h>
class Tox {
    private:
        static std::recursive_mutex m_mtx;
        static Tox* m_instance;
        Tox();

        Tox *m_tox;

    public:
        typedef int FriendNr;
        typedef int ReceiptNr;
        typedef std::array<unsigned char, TOX_FRIEND_ADDRESS_SIZE> FriendAddr;

        enum EFAERR {
            TOOLONG = TOX_FAERR_TOOLONG,
            NOMESSAGE = TOX_FAERR_NOMESSAGE,
            OWNKEY = TOX_FAERR_OWNKEY,
            ALREADYSENT = TOX_FAERR_ALREADYSENT,
            UNKNOWN = TOX_FAERR_UNKNOWN,
            BADCHECKSUM = TOX_FAERR_BADCHECKSUM,
            SETNEWNOSPAM = TOX_FAERR_SETNEWNOSPAM,
            NOMEM = TOX_FAERR_NOMEM
        };

        enum EUSERSTATUS {
            OFFLINE = -1,
            NONE = TOX_USERSTATUS_NONE,
            AWAY = TOX_USERSTATUS_AWAY,
            BUSY = TOX_USERSTATUS_BUSY,
            INVALID = TOX_USERSTATUS_INVALID
        };

        static Tox& instance();
        static void destory();

        void init(const Glib::ustring& statefile);
        void init() {
            init("");
        }

        ~Tox();

        std::vector<FriendNr> get_friendlist();
        FriendAddr get_address();
        FriendNr add_friend(FriendAddr addr, const Glib::ustring& message);
        FriendNr get_friend_number(FriendAddr address);
        void del_friend(FriendNr nr);

        ReceiptNr send_message(FriendNr nr, const Glib::ustring& message);
        ReceiptNr send_action(FriendNr nr, const Glib::ustring& action);
        void send_typing(FriendNr nr, bool is_typing);

        void set_name(const Glib::ustring& name);
        Glib::ustring get_name();
        Glib::ustring get_name(FriendNr nr);
        Glib::ustring get_status_message();
        Glib::ustring get_status_message(FriendNr nr);
        EUSERSTATUS get_status();
        EUSERSTATUS get_status(FriendNr nr);
        unsigned long long get_last_online(FriendNr nr);

        int update_optimal_interval();
        void update();
};
#endif
