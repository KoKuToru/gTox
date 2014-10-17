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
#include <glibmm/i18n.h>

DialogContact* DialogContact::m_instance = nullptr;

DialogContact::DialogContact(const std::string &config_path):
    m_icon_attach(ICON::load_icon(ICON::chat_attach)),
    m_icon_detach(ICON::load_icon(ICON::chat_detach)),
    m_icon_settings(ICON::load_icon(ICON::settings)),
    m_icon_status(ICON::load_icon(ICON::status_offline)),
    m_icon_add(ICON::load_icon(ICON::plus)),
    m_config_path(config_path),
    m_popover_status(m_btn_status),
    m_popover_settings(m_btn_settings),
    m_popover_add(m_btn_add)
{
    auto css = Gtk::CssProvider::create();
    if(!css->load_from_data(THEME::main)) {
        std::cerr << _("Failed to load theme") << std::endl;
    } else {
        auto screen = Gdk::Screen::get_default();
        auto ctx = get_style_context();
        ctx->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    this->set_icon(ICON::load_icon(ICON::icon_128));

    this->set_border_width(0);
    this->set_default_geometry(300, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    m_headerbar_contact.set_name("HeaderBarRight");
    m_headerbar_chat   .set_name("HeaderBarLeft");

    //Setup titleba
    set_title("gTox");
    m_headerbar_contact.set_title(Tox::instance().get_name_or_address());
    m_headerbar_contact.set_subtitle(Tox::instance().get_status_message());
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");
    m_headerbar_chat.set_subtitle("with DemoUser");

    m_btn_xxtach.set_image(m_icon_detach);

    m_headerbar_chat_box_left.get_style_context()->add_class("linked");
    m_headerbar_chat_box_left.add(m_btn_xxtach);
    m_headerbar_chat.pack_start(m_headerbar_chat_box_left);

    //Status button
    m_btn_status.set_image(m_icon_status);
    m_headerbar_contact_box_left.get_style_context()->add_class("linked");
    m_headerbar_contact_box_left.add(m_btn_status);
    m_headerbar_contact.pack_start(m_headerbar_contact_box_left);

    //Add button
    m_btn_add.set_image(m_icon_add);
    m_headerbar_contact_box_right.add(m_btn_add);

    //Settings button
    m_btn_settings.set_image(m_icon_settings);
    m_headerbar_contact_box_right.get_style_context()->add_class("linked");
    m_headerbar_contact_box_right.add(m_btn_settings);
    m_headerbar_contact.pack_end(m_headerbar_contact_box_right);

    m_header_paned.pack1(m_headerbar_chat  , true, false);
    m_header_paned.pack2(m_headerbar_contact, false, false);

    m_btn_status.signal_clicked().connect([this]() {
        m_popover_status.set_visible(true);
    });

    m_btn_settings.signal_clicked().connect([this]() {
        m_popover_settings.set_visible(true);
    });

    m_btn_add.signal_clicked().connect([this]() {
        m_popover_add.set_visible(true);
    });

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
    m_btn_xxtach.signal_clicked().connect(sigc::mem_fun(this, &DialogContact::detach_chat));

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

    m_btn_status.set_sensitive(false);

    m_notification.add_notification(_("PreAlpha Software"), _("Not ready yet"), "Okay", []() {
        //nothing
    });
}

DialogContact::~DialogContact() {
    //save ?
    Tox::instance().destroy();
}

void DialogContact::detach_chat() {
    /*this->property_gravity() = Gdk::GRAVITY_NORTH_WEST;
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
    m_chat_dialog.show();*/
}

bool DialogContact::update() {
    if (!m_btn_status.get_sensitive() && Tox::instance().is_connected()) {
        m_btn_status.set_sensitive(true);
        set_status(Tox::instance().get_status());
    }
    Tox::SEvent ev;
    bool save = false;
    while(Tox::instance().update(ev)) {
        switch(ev.event) {
            case Tox::EEventType::FRIENDACTION:
                std::cout << "FRIENDACTION !" << ev.friend_action.nr << " -> " << ev.friend_action.data << std::endl;
                {
                    WidgetChat* item = get_chat(ev.friend_message.nr);
                    item->add_line(0, true, ev.friend_message.data);
                }
                break;
            case Tox::EEventType::FRIENDMESSAGE:
                std::cout << "FRIENDMESSAGE !" << ev.friend_message.nr << " -> " << ev.friend_message.data << std::endl;
                {
                    WidgetChat* item = get_chat(ev.friend_message.nr);
                    item->add_line(0, true, ev.friend_message.data);
                }
                break;
            case Tox::EEventType::FRIENDREQUEST:
                std::cout << "FRIENDREQUEST ! " << ev.friend_request.message << std::endl;
                m_notification.add_notification("Friend request ["+ Tox::to_hex(ev.friend_request.addr.data(), 32) +"]", ev.friend_request.message, "Accept", [this, ev]() {
                    add_contact(Tox::instance().add_friend_norequest(ev.friend_request.addr));
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
                m_contact.refresh_contact(ev.user_status.nr);
                save = true;
                break;
        }
    }
    if (save) {
        Tox::instance().save(m_config_path);
    }
    return true;
}

void DialogContact::add_contact(Tox::FriendNr nr) {
    m_contact.add_contact(nr);
    Tox::instance().save(m_config_path);
}

WidgetChat* DialogContact::get_chat(Tox::FriendNr nr) {
    //TODO check detached windows
    for(Gtk::Widget* it : m_chat.get_children()) {
        WidgetChat* item = dynamic_cast<WidgetChat*>(it);
        if (item->get_friend_nr() == nr) {
            return item;
        }
    }
    auto item = Gtk::manage(new WidgetChat(nr));
    item->show_all();
    //item->hide();
    m_chat.add(*item/*, true, true*/);
    return item;
}

void DialogContact::activate_chat(Tox::FriendNr nr) {
    property_gravity() = Gdk::GRAVITY_NORTH_EAST;
    if (!m_headerbar_chat.is_visible()) {
        resize(600+get_width(), get_height());
    }
    m_headerbar_chat.show();
    m_chat.show();

    //1. Search if contact has already a open chat
    WidgetChat* item = get_chat(nr);

    //2. hide all chats
    /*for(Gtk::Widget* w : m_chat.get_children()) {
        w->hide();
    }*/
    //3. make the actual chat visible
    //item->show_all();
    m_chat.set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    m_chat.set_visible_child(*item);
    //4. update headerbard
    m_headerbar_chat.set_title(Tox::instance().get_name_or_address(nr));
    m_headerbar_chat.set_subtitle(Tox::instance().get_status_message(nr));
    //6. change focus to inputfiled
    item->focus();
}

DialogContact& DialogContact::instance() {
    if (m_instance == nullptr) {
       throw "Error";
    }
    return *m_instance;
}

void DialogContact::init(const std::string& config_path) {
    if (m_instance != nullptr) {
        destroy();
    }
    m_instance = new DialogContact(config_path);
}

void DialogContact::destroy() {
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void DialogContact::set_status(Tox::EUSERSTATUS status_code) {
    Tox::instance().set_status(status_code);
    //TODO: implement a get_status_icon function
    switch(status_code) {
        case Tox::NONE:
            m_icon_status.property_pixbuf() = ICON::load_icon(ICON::status_online);
            break;
        case Tox::BUSY:
            m_icon_status.property_pixbuf() = ICON::load_icon(ICON::status_busy);
            break;
        case Tox::AWAY:
            m_icon_status.property_pixbuf() = ICON::load_icon(ICON::status_away);
            break;
        default:
             break;
    }
    Tox::instance().save(m_config_path);
}

void DialogContact::exit() {
    Tox::instance().save(m_config_path);
    //TODO: ask for confirmation
    Gtk::Main::quit();
}

void DialogContact::change_name(Glib::ustring name, Glib::ustring msg) {
    Tox::instance().set_name(name);
    Tox::instance().set_status_message(msg);
    m_headerbar_contact.set_title(Tox::instance().get_name_or_address());
    m_headerbar_contact.set_subtitle(Tox::instance().get_status_message());
    Tox::instance().save(m_config_path);
}

void DialogContact::delete_friend(Tox::FriendNr nr){
    m_contact.delete_contact(nr);
    Tox::instance().del_friend(nr);
    Tox::instance().save(m_config_path);
}
