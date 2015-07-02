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
#include "Helper/gToxObservable.h"
#include "Helper/gToxBuilder.h"
#include "Helper/gToxFileRecv.h"
#include "Helper/gToxFileManager.h"

// contact list with pinned chat
class DialogContact : public Gtk::Window, public gToxObservable {
  private:
    Glib::RefPtr<Gtk::StatusIcon> m_status_icon;

    Gtk::HeaderBar* m_headerbar;
    Gtk::Button* m_btn_status;
    Gtk::Stack* m_stack_header;
    Gtk::Stack* m_stack;
    Gtk::ListBox* m_list_contact;
    Gtk::ListBox* m_list_contact_active;
    Gtk::ListBox* m_list_notify;
    Gtk::ScrolledWindow* m_list_contact_scroll;

    Gtk::Image m_icon_status;

    sigc::connection m_update_interval;

    std::string m_config_path;

    std::shared_ptr<PopoverStatus> m_popover_status;
    std::shared_ptr<PopoverSettings> m_popover_settings;

    gToxObservable::Handler m_tox_callback;

    bool is_chat_open(Toxmm::FriendNr nr);

    Glib::RefPtr<Glib::Binding> m_position_binding;

    Gtk::Menu m_popup_menu;

  public:
    DialogContact(BaseObjectType* cobject, gToxBuilder builder,
                  const Glib::ustring& file);
    ~DialogContact();

    static gToxBuilderRef<DialogContact> create(const Glib::ustring& file);

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

    class EventAddNotification {
        public:
            bool show_on_desktop;
            Glib::ustring title;
            Glib::ustring message;
            Glib::RefPtr<Gdk::Pixbuf> image;
            std::vector<std::pair<Glib::ustring, ToxEvent>> actions;
            ToxEvent activated_action;
    };

    class EventCallback {
        public:
            std::function<void()> callback;
    };

    class EventAddContact {
        public:
            Toxmm::FriendNr nr;
    };

    void set_status(Toxmm::EUSERSTATUS status_code);

  protected:
    void load_contacts();

    bool update();
    void tox_event_handling(const ToxEvent& event);
};

#endif
