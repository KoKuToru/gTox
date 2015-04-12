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
#ifndef DIALOGCONTACT_H
#define DIALOGCONTACT_H

#include <gtkmm.h>

#include "Widget/WidgetChat.h"
#include "Widget/WidgetContact.h"
#include "Popover/PopoverStatus.h"
#include "Popover/PopoverSettings.h"
#include "Popover/PopoverAddContact.h"
#include "Helper/ToxEventCallback.h"
#include "DialogChat.h"

// contact list with pinned chat
class DialogContact : public Gtk::Window {
  private:
    static DialogContact* m_instance;

    Glib::RefPtr<Gtk::StatusIcon> m_status_icon;

    Gtk::Paned m_header_paned;
    Gtk::HeaderBar m_headerbar_chat;
    Gtk::HeaderBar m_headerbar_contact;

    Gtk::Paned m_paned;

    Gtk::Image m_icon_attach;
    Gtk::Image m_icon_detach;
    Gtk::Image m_icon_settings;
    Gtk::Image m_icon_status;
    Gtk::Image m_icon_add;

    Gtk::Button m_btn_xxtach;
    Gtk::Button m_btn_status;
    Gtk::Button m_btn_settings;
    Gtk::Button m_btn_add;
    Gtk::Button m_btn_xchat;

    Gtk::Box m_headerbar_chat_box_left;
    Gtk::Box m_headerbar_contact_box_left;
    Gtk::Box m_headerbar_contact_box_right;

    Gtk::Stack m_chat;

    Gtk::VBox m_vbox;

    WidgetContact m_contact;

    std::vector<std::shared_ptr<DialogChat>> m_chat_dialog;

    sigc::connection m_update_interval;

    std::string m_config_path;

    PopoverStatus m_popover_status;
    PopoverSettings m_popover_settings;
    PopoverAddContact m_popover_add;

    DialogContact(const std::string& config_path);

    ToxEventCallback m_tox_callback;
    ToxEventCallback m_tox_callback_chat;

    bool is_chat_open(Tox::FriendNr nr);

  public:
    ~DialogContact();
    static void init(const std::string& config_path);
    static DialogContact& instance();
    static void destroy();

    void activate_chat(Tox::FriendNr nr);
    void set_status(Tox::EUSERSTATUS status_code);
    void exit();
    void add_contact(Tox::FriendNr nr);
    void change_name(Glib::ustring name, Glib::ustring msg);
    void delete_friend(Tox::FriendNr nr);

    void attach_chat(Tox::FriendNr nr);

  protected:
    void detach_chat();
    bool update();
    void tox_event_handling(const ToxEvent& event);
    WidgetChat* get_chat(Tox::FriendNr nr, DialogChat*& dialog);
};

#endif
