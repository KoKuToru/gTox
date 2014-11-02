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
  static std::recursive_mutex m_mtx;
  static Tox *m_instance;
  Tox();

  Tox *m_tox;
  std::shared_ptr<SQLite::Database> m_db;

 public:
  typedef int FriendNr;
  typedef int ReceiptNr;
  typedef std::array<unsigned char, TOX_FRIEND_ADDRESS_SIZE> FriendAddr;

  enum EError {
    UNITIALIZED,
    LOADERROR,
    BOOTERROR,
    FILEERROR,
    FAILED,
    MSGTOOLONG,
    MSGEMPTY,
    CANTADDYOURSELF,
    ALREADYSENT,
    BADCHECKSUM,
    NOSPAM,
    UNKNOWDBVERSION
  };

  struct Exception {
    const EError code;
    Exception(const EError code) : code(code) {}
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

  enum ELogType {
    LOGMSG = 1,
    LOGACTION = 2
  };

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

  static Tox &instance();
  static void destroy();

  void init(const Glib::ustring &statefile);
  void init() { init(""); }

  void save(const Glib::ustring &statefile);

  ~Tox();

  std::vector<FriendNr> get_friendlist();
  FriendAddr get_address();
  FriendNr add_friend(FriendAddr addr, const Glib::ustring &message);
  FriendNr add_friend_norequest(FriendAddr addr);
  FriendNr get_friend_number(FriendAddr address);
  void del_friend(FriendNr nr);

  ReceiptNr send_message(FriendNr nr, const Glib::ustring &message);
  ReceiptNr send_action(FriendNr nr, const Glib::ustring &action);
  void send_typing(FriendNr nr, bool is_typing);

  void set_name(const Glib::ustring &name);
  Glib::ustring get_name();
  Glib::ustring get_name(FriendNr nr);
  Glib::ustring get_name_or_address();
  Glib::ustring get_name_or_address(FriendNr nr);
  Glib::ustring get_status_message();
  void set_status_message(Glib::ustring msg);
  Glib::ustring get_status_message(FriendNr nr);
  EUSERSTATUS get_status();
  EUSERSTATUS get_status(FriendNr nr);
  void set_status(EUSERSTATUS value);
  bool is_connected();
  unsigned long long get_last_online(FriendNr nr);

  int update_optimal_interval();
  bool update(SEvent &ev);

  Tox::FriendAddr get_address(Tox::FriendNr nr);

  static Glib::ustring to_hex(const unsigned char *data, size_t len);
  static std::vector<unsigned char> from_hex(std::string data);

  std::vector<SLog> get_log(FriendNr nr, int offset = 0, int limit = 100);

 protected:
  std::deque<SEvent> events;

  static void callback_friend_request(Tox *,
                                      const unsigned char *addr,
                                      const unsigned char *data,
                                      unsigned short len,
                                      void *);
  static void callback_friend_message(Tox *,
                                      FriendNr nr,
                                      const unsigned char *data,
                                      unsigned short len,
                                      void *);
  static void callback_friend_action(Tox *,
                                     FriendNr nr,
                                     const unsigned char *data,
                                     unsigned short len,
                                     void *);
  static void callback_name_change(Tox *,
                                   FriendNr nr,
                                   const unsigned char *data,
                                   unsigned short len,
                                   void *);
  static void callback_status_message(Tox *,
                                      FriendNr nr,
                                      const unsigned char *data,
                                      unsigned short len,
                                      void *);
  static void callback_user_status(Tox *,
                                   FriendNr nr,
                                   unsigned char data,
                                   void *);
  static void callback_typing_change(Tox *,
                                     FriendNr nr,
                                     unsigned char data,
                                     void *);
  static void callback_read_receipt(Tox *, FriendNr nr, unsigned data, void *);
  static void callback_connection_status(Tox *,
                                         FriendNr nr,
                                         unsigned char data,
                                         void *);

  void inject_event(SEvent ev);
};
#endif
