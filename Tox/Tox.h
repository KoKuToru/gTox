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
#ifndef TOXMM_H
#define TOXMM_H
#include "toxcore/toxcore/tox.h"
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <deque>
#include <glibmm/ustring.h>
#include "SQLiteCpp/Database.h"

/**
 * @brief Wraps the toxcore but also add features with a sqlitedb
 */
class Tox {
  private:
    /**
     * @brief Protects toxcore with a mutex
     *
     * Probably not very important
     */
    static std::recursive_mutex m_mtx;

    /**
     * @brief Instance of Tox
     */
    static Tox* m_instance;

    Tox();

    /**
     * @brief Instance of toxcore
     */
    Tox* m_tox;

    /**
     * @brief Database connection for the custom gTox save file
     */
    std::shared_ptr<SQLite::Database> m_db;

  public:
    typedef int FriendNr;
    typedef int ReceiptNr;
    typedef std::array<unsigned char, TOX_FRIEND_ADDRESS_SIZE> FriendAddr;

    enum EError {
        UNITIALIZED,      //!< Tox::init() never called
        LOADERROR,        //!< tox_load() failed
        BOOTERROR,        //!< tox_bootstrap_from_address() failed
        FILEERROR,        //!< unused
        FAILED,           //!< something went wrong
        MSGTOOLONG,       //!< Message is too long
        MSGEMPTY,         //!< Message is empty
        CANTADDYOURSELF,  //!< Not possible to add yourself as friend
        ALREADYSENT,      //!< Invite already sent
        BADCHECKSUM,      //!< Public address wrong
        NOSPAM,           //!< Public address NO_SPAM changed
        UNKNOWDBVERSION,  //!< gTox save file unknow version
        DBNOTOPEN         //!< Database is not open
    };

    struct Exception {
        const EError code;
        Exception(const EError code) : code(code) {
        }
    };

    enum EEventType {
        FRIENDREQUEST,
        FRIENDMESSAGE,
        FRIENDACTION,
        NAMECHANGE,
        STATUSMESSAGE,
        USERSTATUS,
        TYPINGCHANGE,
        READRECEIPT
    };

    struct SFriendRequest {
        FriendAddr addr;
        Glib::ustring message;
    };

    struct SFriendData {
        FriendNr nr;
        Glib::ustring data;
    };

    struct SFriendInt {
        FriendNr nr;
        int data;
    };

    struct SFriendBool {
        FriendNr nr;
        bool data;
    };

    struct SEvent {
        EEventType event;
        // not very optimal
        SFriendRequest friend_request;
        SFriendData friend_message;
        SFriendData friend_action;
        SFriendData name_change;
        SFriendData status_message;
        SFriendBool typing_change;
        SFriendInt user_status;
        SFriendInt readreceipt;
    };

    enum ELogType { LOGMSG = 1, LOGACTION = 2 };

    struct SLog {
        ELogType type;

        unsigned long long sendtime;
        unsigned long long recvtime;

        Glib::ustring data;
    };

    enum EUSERSTATUS {
        OFFLINE = -1,
        NONE = TOX_USERSTATUS_NONE,
        AWAY = TOX_USERSTATUS_AWAY,
        BUSY = TOX_USERSTATUS_BUSY,
        INVALID = TOX_USERSTATUS_INVALID
    };

    /**
     * @brief Get instance
     *
     * creates an instance if needed
     *
     * @return tox instance
     */
    static Tox& instance();

    /**
     * @brief Destorys the actual instance
     */
    static void destroy();

    /**
     * @brief Init/load toxcore
     *
     * If a something is already loaded
     * it gets killed with tox_kill().
     *
     * Runs tox_new() and installs callbacks.
     *
     * Loads custom gTox save-file.
     *
     * If necessary update gTox save-file.
     * Removes old toxcore savestates if older than 7 days.
     *
     * @throws Tox::Exception
     * @throws SQLite::Exception
     *
     * @param statefile Path to the save-file, can be empty
     */
    void init(const Glib::ustring& statefile);
    void init() {
        init("");
    }

    /**
     * @brief Saves the custom gTox save-file.
     *
     * @throws Tox::Exception
     * @throws SQLite::Exception
     *
     * @param statefile Path to the save-file, only works when never saved before !
     */
    void save(const Glib::ustring& statefile = "");

    /**
     * @brief runs tox_kill if needed
     */
    ~Tox();

    /**
     * @throws Tox::Exception
     *
     * @return Friendlist
     */
    std::vector<FriendNr> get_friendlist();

    /**
     * @brief Get own public address
     *
     * @throws Tox::Exception
     *
     * @return Public address of yourself
     */
    FriendAddr get_address();

    /**
     * @brief Get friends public address
     *
     * @throws Tox::Exception
     *
     * Public address which was used to accept the invite
     * or public address used to invite.
     *
     * @return Public address of friend
     */
    FriendAddr get_address(Tox::FriendNr nr);

    /**
     * @brief Invite friend
     *
     * @throws Tox::Exception
     *
     * @param addr Public address of friend
     * @param message Invite message
     *
     * @return ID of friend
     */
    FriendNr add_friend(FriendAddr addr, const Glib::ustring& message);

    /**
     * @brief Accept friend request
     *
     * @throws Tox::Exception
     *
     * @param addr Public address of friend
     *
     * @return ID of friend
     */
    FriendNr add_friend_norequest(FriendAddr addr);

    /**
     * @throws Tox::Exception
     *
     * @param address
     *
     * @return ID of friend
     */
    FriendNr get_friend_number(FriendAddr address);

    /**
     * @brief Remove friend from friendlist
     *
     * @throws Tox::Exception
     *
     * Removes friend from the friendlist,
     * can be quickly readded with add_friend_norequest().
     * But only if NO_SPAM value of public address didn't change.
     *
     * @param nr ID of friend
     */
    void del_friend(FriendNr nr);

    /**
     * @brief send message to somebody
     *
     * @throws Tox::Exception
     * @throws SQLite::Exception
     *
     * @param nr ID of friend
     * @param message
     *
     * @return Receipt ID of the sent message
     */
    ReceiptNr send_message(FriendNr nr, const Glib::ustring& message);

    /**
     * @brief send message to somebody
     *
     * Prefixes message with "/me "
     * Clients should render "actions" different than "message"
     *
     * @throws Tox::Exception
     * @throws SQLite::Exception
     *
     * @param nr ID of friend
     * @param message
     *
     * @return Receipt ID of the sent message
     */
    ReceiptNr send_action(FriendNr nr, const Glib::ustring& action);

    /**
     * @brief send typing message
     *
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @param is_typing true or false
     */
    void send_typing(FriendNr nr, bool is_typing);

    /**
     * @brief Changes the display name of yourself
     *
     * @throws Tox::Exception
     *
     * @param name new display name
     */
    void set_name(const Glib::ustring& name);

    /**
     * @throws Tox::Exception
     *
     * @return Name of yourself
     */
    Glib::ustring get_name();

    /**
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @return Name of friend
     */
    Glib::ustring get_name(FriendNr nr);

    /**
     * @throws Tox::Exception
     *
     * @return Name of yourself or the address of yourself as hex-string
     */
    Glib::ustring get_name_or_address();

    /**
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @return Name of friend or the address of friend as hex-string
     */
    Glib::ustring get_name_or_address(FriendNr nr);

    /**
     * @throws Tox::Exception
     *
     * @return Status message of yourself
     */
    Glib::ustring get_status_message();

    /**
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @return Status message of friend
     */
    Glib::ustring get_status_message(FriendNr nr);

    /**
     * @brief Changes your status message
     *
     * @throws Tox::Exception
     *
     * @param msg new message
     */
    void set_status_message(Glib::ustring msg);

    /**
     * @throws Tox::Exception
     *
     * @return Online/Busy/Away/Offline status of yourself
     */
    EUSERSTATUS get_status();

    /**
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @return Online/Busy/Away/Offline status of friend
     */
    EUSERSTATUS get_status(FriendNr nr);

    /**
     * @brief Changes your status
     *
     * @param value new status value Online/Busy/Away
     */
    void set_status(EUSERSTATUS value);

    /**
     * @return True when connected to DHT
     */
    bool is_connected();

    /**
     * @throws Tox::Exception
     *
     * @param nr ID of friend
     *
     * @return unixtimestamp of last friend seen online
     */
    unsigned long long get_last_online(FriendNr nr);

    /**
     * @throws Tox::Exception
     *
     * @return Optimal interval for update() in ms
     */
    int update_optimal_interval();

    /**
     * @brief Event loop
     *
     * @throws Tox::Exception
     *
     * @param ev Reference to a event-object
     *
     * @return True when ev was filled with a event
     */
    bool update(SEvent& ev);

    /**
     * @brief Helper function, convert byte array to hex-string
     *
     * @throws Tox::Exception
     *
     * @param data unsigned char array
     * @param len length of data
     *
     * @return hex-string
     */
    static Glib::ustring to_hex(const unsigned char* data, size_t len);

    /**
     * @brief Helper function, convert hex-string to byte array
     *
     * @throws Tox::Exception
     *
     * @param data hex-string
     *
     * @return unsigned char vector
     */
    static std::vector<unsigned char> from_hex(std::string data);

    /**
     * @brief Get chat log
     *
     * @throws SQLite::Exception
     *
     * @param nr ID of friend
     * @param offset
     * @param limit
     *
     * @return Chat log
     */
    std::vector<SLog> get_log(FriendNr nr, int offset = 0, int limit = 100);

    /**
     * @brief Gets a config parameter
     *
     * @throws Tox::Exception
     *
     * @param name of the paramter
     * @param value the default value
     *
     * @return value of the config parameter
     */
    std::string config_get(const std::string& name, const std::string& value);
    /**
     * @brief Sets a config parameter
     *
     * @throws Tox::Exception
     *
     * @param name  of the paramter
     * @param value of the config parameter
     */
    void config_set(const std::string& name, const std::string& value);

    protected:
    std::deque<SEvent> events;

    static void callback_friend_request(Tox*,
                                        const unsigned char* addr,
                                        const unsigned char* data,
                                        unsigned short len,
                                        void*);
    static void callback_friend_message(Tox*,
                                        FriendNr nr,
                                        const unsigned char* data,
                                        unsigned short len,
                                        void*);
    static void callback_friend_action(Tox*,
                                       FriendNr nr,
                                       const unsigned char* data,
                                       unsigned short len,
                                       void*);
    static void callback_name_change(Tox*,
                                     FriendNr nr,
                                     const unsigned char* data,
                                     unsigned short len,
                                     void*);
    static void callback_status_message(Tox*,
                                        FriendNr nr,
                                        const unsigned char* data,
                                        unsigned short len,
                                        void*);
    static void callback_user_status(Tox*,
                                     FriendNr nr,
                                     unsigned char data,
                                     void*);
    static void callback_typing_change(Tox*,
                                       FriendNr nr,
                                       unsigned char data,
                                       void*);
    static void callback_read_receipt(Tox*, FriendNr nr, unsigned data, void*);
    static void callback_connection_status(Tox*,
                                           FriendNr nr,
                                           unsigned char data,
                                           void*);

    void inject_event(SEvent ev);
};
#endif
