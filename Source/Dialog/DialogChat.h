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
#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <gtkmm.h>
#include "Tox/Toxmm.h"
#include "Widget/WidgetChat.h"
#include <libnotifymm.h>
#include <memory>
#include "Dialog/DialogContact.h"

// Single chat window
class DialogChat : public Gtk::Window, public gToxChild {
  private:
    bool m_in_window;
    Gtk::HeaderBar m_header;
    Gtk::HeaderBar m_header2;
    Gtk::Box m_headerbar_btn_left;
    Gtk::Button m_btn_xxtach;
    Gtk::Box m_header_box;

    std::shared_ptr<Notify::Notification> m_notify;

    WidgetChat m_chat;
    Gtk::Box m_chat_box;

    gToxInstance::CallbackHandler m_tox_callback;

    void notify(const Glib::ustring& title, const Glib::ustring& message);

  public:
    DialogChat(gToxInstance* instance, Toxmm::FriendNr nr);
    ~DialogChat();

    void show();
    void hide();
    void present();

    bool is_visible();
};

#endif
