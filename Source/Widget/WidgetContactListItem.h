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
#ifndef WIDGETCONTACTLISTITEM_H
#define WIDGETCONTACTLISTITEM_H

#include <gtkmm.h>
#include "Tox/Toxmm.h"
#include "Helper/ToxEventCallback.h"
#include "Dialog/DialogChat.h"

class WidgetContact;
class WidgetContactListItem : public Gtk::ListBoxRow {
  private:
    const Glib::RefPtr<Gtk::Builder> m_builder;
    Gtk::Image* m_avatar;
    Gtk::Label* m_name;
    Gtk::Label* m_status_msg;
    Gtk::Image* m_status_icon;

    Toxmm::FriendNr m_friend_nr;

    ToxEventCallback m_tox_callback;

    Glib::RefPtr<Gdk::PixbufAnimation> generate_animation(
        const Glib::RefPtr<Gdk::Pixbuf>& icon);

    bool spin;

    std::shared_ptr<DialogChat> m_chat;

  public:
    WidgetContactListItem(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    static WidgetContactListItem* create(Toxmm::FriendNr nr);

    ~WidgetContactListItem();

    Toxmm::FriendNr get_friend_nr();

    class EventStopSpin {
        public:
            Toxmm::FriendNr nr;
    };

    class EventActivated {
        public:
            WidgetContactListItem* target;
    };

  protected:
    void refresh();
    void set_contact(Toxmm::FriendNr nr);
};

#endif
