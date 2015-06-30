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
#include <tox/tox.h>
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <deque>
#include <glibmm/ustring.h>
#include "SQLiteCpp/Database.h"
#include "ToxDatabase.h"
#include "ToxException.h"
#include "ToxEvent.h"
#include "ToxProfile.h"

/**
 * @brief Wraps the toxcore but also add features with a sqlitedb
 */
class Toxmm {
  private:
    /**
     * @brief Instance of toxcore
     */
    Tox* m_tox = nullptr;

    /**
     * @brief Database connection for the custom gTox save file
     */
    ToxDatabase m_db;

    /**
      * @brief Save/Load profile helper
      */
    ToxProfile m_profile;

  public:
    typedef uint32_t FriendNr;
    typedef uint32_t ReceiptNr;
    typedef uint32_t FileNr;
    typedef std::array<uint8_t, TOX_ADDRESS_SIZE> FriendAddr;
    typedef std::array<uint8_t, TOX_FILE_ID_LENGTH> FileId;
    typedef std::array<uint8_t, TOX_HASH_LENGTH> Hash;

    typedef ToxException Exception;

    enum EUSERSTATUS {
        OFFLINE = -1,
        NONE = TOX_USER_STATUS::TOX_USER_STATUS_NONE,
        AWAY = TOX_USER_STATUS::TOX_USER_STATUS_AWAY,
        BUSY = TOX_USER_STATUS::TOX_USER_STATUS_BUSY
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
        uint32_t data;
    };

    struct SFriendBool {
        FriendNr nr;
        bool is_typing;
    };

    struct SCustom {
        FriendNr nr;
        std::string cmd;
        std::string data;
    };

    class EventFriendRequest {
        public:
            FriendAddr addr;
            Glib::ustring message;
    };

    class EventFriendMessage {
        public:
            FriendNr nr;
            Glib::ustring message;
    };

    class EventFriendAction {
        public:
            FriendNr nr;
            Glib::ustring message;
    };

    class EventName {
        public:
            FriendNr nr;
            Glib::ustring name;
    };

    class EventStatusMessage {
        public:
            FriendNr nr;
            Glib::ustring status_message;
    };

    class EventTyping {
        public:
            FriendNr nr;
            bool is_typing;
    };

    class EventUserStatus {
        public:
            FriendNr nr;
            EUSERSTATUS status;
    };

    class EventReadReceipt {
        public:
            FriendNr nr;
            uint32_t receipt;
    };

    class EventFileRecv {
        public:
            FriendNr nr;
            long long file_number;
            TOX_FILE_KIND kind;
            uint64_t file_size;
            Glib::ustring filename;
    };

    class EventFileRecvChunk {
        public:
            FriendNr nr;
            uint32_t file_number;
            uint64_t file_position;
            std::vector<uint8_t> file_data;
    };

    ToxDatabase& database();
    ToxProfile& profile();

    Toxmm();

    void open(const Glib::ustring& statefile, bool bootstrap = true, bool skip_profile = false);
    void save();

    /**
     * @brief runs tox_kill if needed
     */
    ~Toxmm();

    /**
     * @throws Toxmm::Exception
     *
     * @return Friendlist
     */
    std::vector<FriendNr> get_friendlist();

    /**
     * @brief Get own public address
     *
     * @throws Toxmm::Exception
     *
     * @return Public address of yourself
     */
    FriendAddr get_address();

    /**
     * @brief Get friends public address
     *
     * @throws Toxmm::Exception
     *
     * Public address which was used to accept the invite
     * or public address used to invite.
     *
     * @return Public address of friend
     */
    FriendAddr get_address(Toxmm::FriendNr nr);

    /**
     * @brief Invite friend
     *
     * @throws Toxmm::Exception
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
     * @throws Toxmm::Exception
     *
     * @param addr Public address of friend
     *
     * @return ID of friend
     */
    FriendNr add_friend_norequest(FriendAddr addr);

    /**
     * @throws Toxmm::Exception
     *
     * @param address
     *
     * @return ID of friend
     */
    FriendNr get_friend_number(FriendAddr address);

    /**
     * @brief Remove friend from friendlist
     *
     * @throws Toxmm::Exception
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
     * @throws Toxmm::Exception
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
     * @throws Toxmm::Exception
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
     * @throws Toxmm::Exception
     *
     * @param nr ID of friend
     *
     * @param is_typing true or false
     */
    void send_typing(FriendNr nr, bool is_typing);

    /**
     * @brief Changes the display name of yourself
     *
     * @throws Toxmm::Exception
     *
     * @param name new display name
     */
    void set_name(const Glib::ustring& name);

    /**
     * @throws Toxmm::Exception
     *
     * @return Name of yourself
     */
    Glib::ustring get_name();

    /**
     * @throws Toxmm::Exception
     *
     * @param nr ID of friend
     *
     * @return Name of friend
     */
    Glib::ustring get_name(FriendNr nr);

    /**
     * @throws Toxmm::Exception
     *
     * @return Name of yourself or the address of yourself as hex-string
     */
    Glib::ustring get_name_or_address();

    /**
     * @throws Toxmm::Exception
     *
     * @param nr ID of friend
     *
     * @return Name of friend or the address of friend as hex-string
     */
    Glib::ustring get_name_or_address(FriendNr nr);

    /**
     * @throws Toxmm::Exception
     *
     * @return Status message of yourself
     */
    Glib::ustring get_status_message();

    /**
     * @throws Toxmm::Exception
     *
     * @param nr ID of friend
     *
     * @return Status message of friend
     */
    Glib::ustring get_status_message(FriendNr nr);

    /**
     * @brief Changes your status message
     *
     * @throws Toxmm::Exception
     *
     * @param msg new message
     */
    void set_status_message(Glib::ustring msg);

    /**
     * @throws Toxmm::Exception
     *
     * @return Online/Busy/Away/Offline status of yourself
     */
    EUSERSTATUS get_status();

    /**
     * @throws Toxmm::Exception
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
     * @throws Toxmm::Exception
     *
     * @param nr ID of friend
     *
     * @return unixtimestamp of last friend seen online
     */
    uint64_t get_last_online(FriendNr nr);

    /**
     * @throws Toxmm::Exception
     *
     * @return Optimal interval for update() in ms
     */
    int update_optimal_interval();

    /**
     * @brief Event loop
     *
     * @throws Toxmm::Exception
     *
     * @param ev Reference to a event-object
     *
     * @return True when ev was filled with a event
     */
    bool update(ToxEvent& ev);

    /**
     * @brief Helper function, convert byte array to hex-string
     *
     * @throws Toxmm::Exception
     *
     * @param data uint8_t array
     * @param len length of data
     *
     * @return hex-string
     */
    static Glib::ustring to_hex(const uint8_t* data, size_t len);

    /**
     * @brief Helper function, convert hex-string to byte array
     *
     * @throws Toxmm::Exception
     *
     * @param data hex-string
     *
     * @return uint8_t vector
     */
    static std::vector<uint8_t> from_hex(std::string data);

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
    std::vector<ToxLogEntity> get_log(FriendNr nr, int offset = 0, int limit = 100);

    /**
     * @brief Sends a file control command to a friend for a given file transfer.
     *
     * @param nr The friend number of the friend the file is being transferred to or received from.
     * @param file_nr The friend-specific identifier for the file transfer.
     * @param control The control command to send.
     *
     * @throws Toxmm::Exception
     */
    void file_control(FriendNr nr, FileNr file_nr, TOX_FILE_CONTROL control);

    /**
     * @brief Sends a file seek control command to a friend for a given file transfer.
     * This function can only be called to resume a file transfer right before
     * TOX_FILE_CONTROL_RESUME is sent.
     *
     * @param nr The friend number of the friend the file is being
     *   received from.
     * @param file_nr The friend-specific identifier for the file transfer.
     * @param postion The position that the file should be seeked to.
     */
    void file_seek(FriendNr nr, FileNr file_nr, uint64_t position);

    /**
     * @brief Gets the file id associated to the file transfer.
     * @param nr The friend number of the friend the file is being
     *   transferred to or received from.
     * @param file_nr The friend-specific identifier for the file transfer.
     */
    FileId file_get_file_id(FriendNr nr, FileNr file_nr);

    /**
     * @brief file_send
     * @param nr The friend number of the friend the file send request
     *   should be sent to.
     * @param kind The meaning of the file to be sent.
     * @param path The full path of the file
     * @return
     */
    FileNr file_send(FriendNr nr, TOX_FILE_KIND kind, Glib::ustring path);
    /**
     * @param id  A file identifier of length TOX_FILE_ID_LENGTH that can be used to
     *   uniquely identify file transfers across core restarts. It can then be obtained by using file_get_file_id().
     * */
    FileNr file_resume(FriendNr nr, FileId id);

    /**
     * Generates a cryptographic hash of the given data.
     *
     * This function may be used by clients for any purpose, but is provided
     * primarily for validating cached avatars. This use is highly recommended to
     * avoid unnecessary avatar updates.
     *
     * @param data Data to be hashed
     * @return hash of the given data
     */
    Hash hash(const std::vector<uint8_t>& data);

    /**
     * Replaces illegal UTF-8 Characters with U+FFFD replacement character
     *
     * @param input
     * @param size
     * @return valid utf-8 string
     */
    static Glib::ustring fix_utf8(const char* input, int size);

  protected:
    std::deque<ToxEvent> m_events;

    static void callback_friend_request(Tox*,
                                        const uint8_t* addr,
                                        const uint8_t* data,
                                        size_t len,
                                        void*);
    static void callback_friend_message(Tox*,
                                        FriendNr nr,
                                        TOX_MESSAGE_TYPE type,
                                        const uint8_t* data,
                                        size_t len,
                                        void*);
    static void callback_friend_action(Tox*,
                                       FriendNr nr,
                                       const uint8_t* data,
                                       size_t len,
                                       void*);
    static void callback_name_change(Tox*,
                                     FriendNr nr,
                                     const uint8_t* data,
                                     size_t len,
                                     void*);
    static void callback_status_message(Tox*,
                                        FriendNr nr,
                                        const uint8_t* data,
                                        size_t len,
                                        void*);
    static void callback_user_status(Tox*,
                                     FriendNr nr,
                                     TOX_USER_STATUS data,
                                     void*);
    static void callback_typing_change(Tox*,
                                       FriendNr nr,
                                       bool data,
                                       void*);
    static void callback_read_receipt(Tox*, FriendNr nr, uint32_t data, void*);
    static void callback_connection_status(Tox*,
                                           FriendNr nr,
                                           TOX_CONNECTION data,
                                           void*);

    static void callback_file_recv(Tox*,
                                   FriendNr nr,
                                   uint32_t file_number,
                                   uint32_t kind,
                                   uint64_t file_size,
                                   const uint8_t* filename,
                                   size_t filename_length,
                                   void*);
    static void callback_file_recv_chunk(Tox*,
                                         FriendNr nr,
                                         uint32_t file_number,
                                         uint64_t position,
                                         const uint8_t* data,
                                         size_t data_length,
                                         void*);

    void inject_event(ToxEvent ev);
};
#endif
