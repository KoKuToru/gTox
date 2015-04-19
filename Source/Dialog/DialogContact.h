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
#include "Popover/PopoverStatus.h"
#include "Popover/PopoverSettings.h"
#include "Popover/PopoverAddContact.h"
#include "Helper/ToxEventCallback.h"
#include "DialogChat.h"

// contact list with pinned chat
class DialogContact : public Gtk::Window {
  private:
    static DialogContact* m_instance;
    const Glib::RefPtr<Gtk::Builder> m_builder;

    Glib::RefPtr<Gtk::StatusIcon> m_status_icon;

    Gtk::HeaderBar* m_headerbar;

    Gtk::Button* m_btn_status;

    Gtk::Stack* m_stack_header;
    Gtk::Stack* m_stack;

    Gtk::Image m_icon_status;

    std::vector<std::shared_ptr<DialogChat>> m_chat;

    sigc::connection m_update_interval;

    std::string m_config_path;

    std::shared_ptr<PopoverStatus> m_popover_status;
    std::shared_ptr<PopoverSettings> m_popover_settings;
    std::shared_ptr<PopoverAddContact> m_popover_add_contact;

    ToxEventCallback m_tox_callback;

    bool is_chat_open(Toxmm::FriendNr nr);

  public:
    DialogContact(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~DialogContact();

    static std::shared_ptr<DialogContact> create(Glib::ustring file);

    static DialogContact& instance();

    void exit();

    class EventAttachWidget {
        public:
            Gtk::HeaderBar* header;
            Gtk::Widget* body;
    };

    class EventDetachWidget {
        public:
            bool close;
            Gtk::HeaderBar* header;
            Gtk::Widget* body;
            int &out_x;
            int &out_y;
            int &out_w;
            int &out_h;
    };

    class EventSetName {
        public:
            Glib::ustring name;
            Glib::ustring status;
    };

    class EventSetStatus {
        public:
            Toxmm::EUSERSTATUS status_code;
    };

    class EventPresentWidget {
        public:
            Gtk::HeaderBar* header;
            Gtk::Widget* body;
    };

    void set_status(Toxmm::EUSERSTATUS status_code);

  protected:
    void load_contacts();

    bool update();
    void tox_event_handling(const ToxEvent& event);

    WidgetChat* get_chat(Toxmm::FriendNr nr, DialogChat*& dialog);
};

#endif
