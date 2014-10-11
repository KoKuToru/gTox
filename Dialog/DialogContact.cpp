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
#include "DialogContact.h"
#include "Generated/icon.h"
#include <gdkmm.h>
#include "Tox/Tox.h"
#include <iostream>
#include <libnotifymm.h>
#include "Generated/theme.h"
#include <iostream>

DialogContact::DialogContact(const std::string &config_path):
    m_icon_attach(ICON::load_icon(ICON::chat_attach)),
    m_icon_detach(ICON::load_icon(ICON::chat_detach)),
    m_icon_settings(ICON::load_icon(ICON::settings)),
    m_config_path(config_path)
{
    auto css = Gtk::CssProvider::create();
    if(!css->load_from_data(THEME::main)) {
        std::cerr << "Failed to load theme\n";
    } else {
        auto screen = Gdk::Screen::get_default();
        auto ctx = get_style_context();
        ctx->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    this->set_icon(ICON::load_icon(ICON::icon_128));

    this->set_border_width(1);
    this->set_default_geometry(300, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    m_headerbar_contact.set_name("HeaderBarRight");
    m_headerbar_chat   .set_name("HeaderBarLeft");

    //Setup titlebar
    m_headerbar_contact.set_title("Contacts");
    m_headerbar_contact.set_subtitle(Tox::instance().get_name());
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");
    m_headerbar_chat.set_subtitle("with DemoUser");

    m_btn_xxtach.set_image(m_icon_detach);

    m_headerbar_chat_box_left.get_style_context()->add_class("linked");
    m_headerbar_chat_box_left.add(m_btn_xxtach);
    m_headerbar_chat.pack_start(m_headerbar_chat_box_left);

    m_btn_settings.set_image(m_icon_settings);

    m_headerbar_contact_box_left.get_style_context()->add_class("linked");
    m_headerbar_contact_box_left.add(m_btn_settings);
    m_headerbar_contact.pack_start(m_headerbar_contact_box_left);

    m_header_paned.pack1(m_headerbar_chat  , true, false);
    m_header_paned.pack2(m_headerbar_contact, false, false);

    this->set_titlebar(m_header_paned);

    m_vbox.add(m_contact);
    m_vbox.pack_end(m_notification, false, false);

    //Setup content
    m_paned.pack1(m_chat, false, false);
    m_paned.pack2(m_vbox, true, true);
    this->add(m_paned);

    //Connect properties C++ version ?
    g_object_bind_property(m_header_paned.gobj(), "position",
                           m_paned.gobj(),        "position",
                           GBindingFlags(G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE));

    //events
    m_btn_xxtach.signal_clicked().connect(sigc::mem_fun(this, &DialogContact::detachChat));

    m_contact.load_list();

    m_update_interval = Glib::signal_timeout().connect(sigc::mem_fun(this, &DialogContact::update), Tox::instance().update_optimal_interval());

    //display friend id
    Tox::FriendAddr my_addr = Tox::instance().get_address();
    std::cout << "PUB: ";
    for(auto c : my_addr) {
        static const char hex[] = "0123456789ABCDEF";
        std::cout << hex[(c >> 4) & 0xF] << hex[c & 0xF];
    }
    std::cout << std::endl;

    m_header_paned.show();
    m_headerbar_contact.show_all();
    m_contact.show_all();
    m_paned.show();
    m_vbox.show();
    show();
}

DialogContact::~DialogContact() {
    //save ?
    Tox::instance().destroy();
}

void DialogContact::detachChat() {
    this->property_gravity() = Gdk::GRAVITY_NORTH_WEST;
    int x,y;
    this->get_position(x, y);
    this->property_gravity() = Gdk::GRAVITY_NORTH_EAST;
    int w,h;
    this->get_size(w, h);
    int hw = m_headerbar_chat.get_width();
    w -= hw;
    this->resize(w, h);

    m_header_paned.remove(m_headerbar_chat);
    m_paned.remove(m_chat);

    m_chat_dialog.move(x, y);
    m_chat_dialog.resize(hw, h); //too small why ?
    m_chat_dialog.show();
}

bool DialogContact::update() {
    Tox::SEvent ev;
    bool save = false;
    while(Tox::instance().update(ev)) {
        switch(ev.event) {
            case Tox::EEventType::FRIENDACTION:
                std::cout << "FRIENDACTION !" << ev.friend_action.nr << " -> " << ev.friend_action.data << std::endl;

                break;
            case Tox::EEventType::FRIENDMESSAGE:
                std::cout << "FRIENDMESSAGE !" << ev.friend_message.nr << " -> " << ev.friend_message.data << std::endl;
                Tox::instance().send_message(ev.friend_message.nr, "I read your \""+ev.friend_message.data+"\" message");
                break;
            case Tox::EEventType::FRIENDREQUEST:
                std::cout << "FRIENDREQUEST ! " << ev.friend_request.message << std::endl;
                m_notification.add_notification("Friend request [Addr in future]", ev.friend_request.message, "Accept", [this, ev]() {
                    m_contact.add_contact(Tox::instance().add_friend_norequest(ev.friend_request.addr));
                    Tox::instance().save(m_config_path);
                });
                break;
            case Tox::EEventType::NAMECHANGE:
                std::cout << "NAMECHANGE !" << ev.name_change.nr << " -> " << ev.name_change.data << std::endl;
                m_contact.refresh_contact(ev.name_change.nr);
                save = true;
                break;
            case Tox::EEventType::READRECEIPT:
                std::cout << "READRECEIPT !" << ev.readreceipt.nr << " -> " << ev.readreceipt.data << std::endl;
                break;
            case Tox::EEventType::STATUSMESSAGE:
                std::cout << "STATUSMESSAGE !"<< ev.status_message.nr << " -> " << ev.status_message.data << std::endl;
                m_contact.refresh_contact(ev.status_message.nr);
                save = true;
                break;
            case Tox::EEventType::TYPINGCHANGE:
                std::cout << "TYPINGCHANGE !" << ev.typing_change.nr << " -> " << ev.typing_change.data << std::endl;
                break;
            case Tox::EEventType::USERSTATUS:
                std::cout << "USERSTATUS !" << ev.user_status.nr << " -> " << ev.user_status.data << std::endl;
                break;
        }
    }
    if (save) {
        Tox::instance().save("demo.state");
    }
    return true;
}
