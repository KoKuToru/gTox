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
#include "Helper/Canberra.h"

DialogContact* DialogContact::m_instance = nullptr;

DialogContact::DialogContact(const std::string& config_path)
    : m_icon_attach(ICON::load_icon(ICON::chat_attach)),
      m_icon_detach(ICON::load_icon(ICON::chat_detach)),
      m_icon_settings(ICON::load_icon(ICON::settings)),
      m_icon_status(ICON::load_icon(ICON::status_offline)),
      m_icon_add(ICON::load_icon(ICON::plus)),
      m_config_path(config_path),
      m_popover_status(m_btn_status),
      m_popover_settings(m_btn_settings),
      m_popover_add(m_btn_add) {

    m_tox_callback = [this](const ToxEvent& ev) { tox_event_handling(ev); };

    auto css = Gtk::CssProvider::create();
    if (!css->load_from_data(THEME::main)) {
        std::cerr << _("LOADING_THEME_FAILED") << std::endl;
    } else {
        auto screen = Gdk::Screen::get_default();
        auto ctx = get_style_context();
        ctx->add_provider_for_screen(
            screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    this->set_icon(ICON::load_icon(ICON::icon_128));

    this->set_border_width(0);
    this->set_default_geometry(300, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    m_headerbar_contact.set_name("HeaderBarRight");
    m_headerbar_chat.set_name("HeaderBarLeft");

    // Setup titleba
    set_title("gTox");
    m_headerbar_contact.set_title(Tox::instance().get_name_or_address());
    m_headerbar_contact.set_subtitle(Tox::instance().get_status_message());
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");
    m_headerbar_chat.set_subtitle("with DemoUser");
    m_headerbar_chat.signal_map().connect_notify([this](){
        m_headerbar_chat.get_style_context()->add_class("headerbar-left");
        m_headerbar_contact.get_style_context()->add_class("headerbar-right");
    });
    m_headerbar_chat.signal_unmap().connect_notify([this](){
        m_headerbar_chat.get_style_context()->remove_class("headerbar-left");
        m_headerbar_contact.get_style_context()->remove_class("headerbar-right");
    });

    m_btn_xxtach.set_image(m_icon_detach);

    m_headerbar_chat_box_left.get_style_context()->add_class("linked");
    m_headerbar_chat_box_left.add(m_btn_xxtach);
    m_headerbar_chat.pack_start(m_headerbar_chat_box_left);

    // custom close button for chat
    auto x_box = Gtk::manage(new Gtk::Box());
    x_box->add(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)));
    m_btn_xchat.set_image_from_icon_name("window-close-symbolic");
    m_btn_xchat.get_style_context()->add_class("titlebutton");
    m_btn_xchat.set_valign(Gtk::ALIGN_CENTER);
    x_box->add(m_btn_xchat);
    x_box->show_all();
    x_box->get_style_context()->add_class("right");
    x_box->set_spacing(6);
    m_headerbar_chat.pack_end(*x_box);

    // Status button
    m_btn_status.set_image(m_icon_status);
    m_headerbar_contact_box_left.get_style_context()->add_class("linked");
    m_headerbar_contact_box_left.add(m_btn_status);
    m_headerbar_contact.pack_start(m_headerbar_contact_box_left);

    // Add button
    m_btn_add.set_image(m_icon_add);
    m_headerbar_contact_box_right.add(m_btn_add);

    // Settings button
    m_btn_settings.set_image(m_icon_settings);
    m_headerbar_contact_box_right.get_style_context()->add_class("linked");
    m_headerbar_contact_box_right.add(m_btn_settings);
    m_headerbar_contact.pack_end(m_headerbar_contact_box_right);

    m_header_paned.pack1(m_headerbar_chat, true, false);
    m_header_paned.pack2(m_headerbar_contact, false, false);

    m_btn_status.signal_clicked().connect(
        [this]() { m_popover_status.set_visible(true); });

    m_btn_settings.signal_clicked().connect(
        [this]() { m_popover_settings.set_visible(true); });

    m_btn_add.signal_clicked().connect(
        [this]() { m_popover_add.set_visible(true); });

    m_btn_xchat.signal_clicked().connect([this]() {
        auto child = m_chat.get_visible_child();
        if (!child) {
            return;
        }
        m_chat.remove(*child);
        delete child;

        // check if empty
        child = m_chat.get_visible_child();
        if (!child) {
            property_gravity() = Gdk::GRAVITY_NORTH_EAST;
            if (m_headerbar_chat.is_visible()) {
                resize(m_headerbar_contact.get_width(), get_height());
            }
            m_headerbar_chat.hide();
            m_chat.hide();
        } else {
            WidgetChat* item = dynamic_cast<WidgetChat*>(child);
            m_headerbar_chat.set_title(
                Tox::instance().get_name_or_address(item->get_friend_nr()));
            m_headerbar_chat.set_subtitle(
                Tox::instance().get_status_message(item->get_friend_nr()));
        }
    });

    this->set_titlebar(m_header_paned);

    m_vbox.add(m_contact);
    m_vbox.pack_end(m_notification, false, false);

    // Setup content
    m_paned.pack1(m_chat, false, false);
    m_paned.pack2(m_vbox, true, true);
    this->add(m_paned);

    // Connect properties C++ version ?
    g_object_bind_property(
        m_header_paned.gobj(),
        "position",
        m_paned.gobj(),
        "position",
        GBindingFlags(G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL
                      | G_BINDING_SYNC_CREATE));

    // events
    m_btn_xxtach.signal_clicked().connect(
        sigc::mem_fun(this, &DialogContact::detach_chat));

    m_update_interval = Glib::signal_timeout().connect(
        sigc::mem_fun(this, &DialogContact::update),
        Tox::instance().update_optimal_interval());

    // display friend id
    Tox::FriendAddr my_addr = Tox::instance().get_address();
    std::cout << "PUB: ";
    for (auto c : my_addr) {
        static const char hex[] = "0123456789ABCDEF";
        std::cout << hex[(c >> 4) & 0xF] << hex[c & 0xF];
    }
    std::cout << std::endl;

    m_header_paned.show();
    m_headerbar_contact.show_all();
    m_contact.show_all();
    m_paned.show();
    m_vbox.show();
    m_headerbar_chat_box_left.show_all();
    show();

    m_btn_status.set_sensitive(false);

    m_notification.add_notification(
        _("PREALPHA_SOFTWARE"), _("NOT_READY_YET"), _("OKAY"), []() {
            // nothing
        });
}

DialogContact::~DialogContact() {
    // save ?
    Tox::instance().destroy();
}

void DialogContact::detach_chat() {
    auto child = m_chat.get_visible_child();
    if (!child) {
        return;
    }
    WidgetChat* item = dynamic_cast<WidgetChat*>(child);
    if (!item) {
        return;
    }

    this->property_gravity() = Gdk::GRAVITY_NORTH_WEST;
    int x, y, w, h;
    get_position(x, y);
    get_size(w, h);
    w -= m_headerbar_contact.get_width();

    auto new_chat = new DialogChat(item->get_friend_nr());
    m_chat_dialog.push_back(std::shared_ptr<DialogChat>(new_chat));
    new_chat->move(x, y);
    new_chat->resize(w, h);
    new_chat->show();

    // remove child
    m_btn_xchat.clicked();
}

void DialogContact::attach_chat(Tox::FriendNr nr) {
    DialogChat* dialog = nullptr;
    WidgetChat* item = get_chat(nr, dialog);

    if (!item || !dialog) {
        return;
    }

    m_chat_dialog.resize(std::distance(
        m_chat_dialog.begin(),
        std::remove_if(m_chat_dialog.begin(),
                       m_chat_dialog.end(),
                       [dialog](const std::shared_ptr<DialogChat>& o) {
            return o.get() == dialog;
        })));

    activate_chat(nr);
}

bool DialogContact::update() {
    if (!m_btn_status.get_sensitive() && Tox::instance().is_connected()) {
        m_btn_status.set_sensitive(true);
        set_status(Tox::instance().get_status());
    }

    ToxEvent ev;
    while (Tox::instance().update(ev)) {
        ToxEventCallback::notify(ev);
    }

    return true;
}

void DialogContact::tox_event_handling(const ToxEvent& ev) {
    if (ev.type() == typeid(Tox::EventFriendMessage)) {
        auto data = ev.get<Tox::EventFriendMessage>();
        DialogChat* chat;
        get_chat(data.nr, chat);
        if (is_chat_open(data.nr)) {
            ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{
                                                  data.nr
                                              }));
        }
        Canberra::play("message-new-instant");
    } else if (ev.type() == typeid(Tox::EventFriendAction)) {
        auto data = ev.get<Tox::EventFriendAction>();
        DialogChat* chat;
        get_chat(data.nr, chat);
        if (is_chat_open(data.nr)) {
            ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{
                                                  data.nr
                                              }));
        }
        Canberra::play("message-new-instant");
    } else if (ev.type() == typeid(Tox::EventFriendRequest)) {
        auto data = ev.get<Tox::EventFriendRequest>();
        m_notification.add_notification(
            "Friend request [" + Tox::to_hex(data.addr.data(), 32) + "]",
            data.message,
            "Accept",
            [this, data]() {
                add_contact(Tox::instance().add_friend_norequest(data.addr));
                Tox::instance().save();
            });
    }
}

void DialogContact::add_contact(Tox::FriendNr nr) {
    m_contact.add_contact(nr);
    Tox::instance().save();
}

WidgetChat* DialogContact::get_chat(Tox::FriendNr nr, DialogChat*& dialog) {
    dialog = nullptr;
    for (Gtk::Widget* it : m_chat.get_children()) {
        WidgetChat* item = dynamic_cast<WidgetChat*>(it);
        if (item->get_friend_nr() == nr) {
            return item;
        }
    }
    for (auto c : m_chat_dialog) {
        WidgetChat* item = &(c->get_chat());
        if (item->get_friend_nr() == nr) {
            dialog = c.get();
            return item;
        }
    }
    auto item = Gtk::manage(new WidgetChat(nr));
    item->show_all();

    m_chat.add(*item,
               Tox::to_hex(Tox::instance().get_address(nr).data(),
                           TOX_PUBLIC_KEY_SIZE));

    return item;
}

bool DialogContact::is_chat_open(Tox::FriendNr nr) {
    for (Gtk::Widget* it : m_chat.get_children()) {
        WidgetChat* item = dynamic_cast<WidgetChat*>(it);
        if (item->get_friend_nr() == nr) {
            if (m_headerbar_chat.is_visible()) {
                return it == m_chat.get_visible_child();
            } else {
                return false;
            }
        }
    }
    for (auto c : m_chat_dialog) {
        WidgetChat* item = &(c->get_chat());
        if (item->get_friend_nr() == nr) {
            return item->is_visible();
        }
    }
    return false;
}

void DialogContact::activate_chat(Tox::FriendNr nr) {
    ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin()));

    // 1. Search if contact has already a open chat
    DialogChat* dialog = nullptr;
    WidgetChat* item = get_chat(nr, dialog);

    if (dialog) {
        dialog->present();
        return;
    }

    // 2. resize window
    property_gravity() = Gdk::GRAVITY_NORTH_EAST;
    if (!m_headerbar_chat.is_visible()) {
        resize(600 + get_width(), get_height());
    }
    m_headerbar_chat.show();
    m_chat.show();

    // 3. make the actual chat visible
    m_chat.set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    m_chat.set_visible_child(*item);

    // 4. update headerbard
    m_headerbar_chat.set_title(Tox::instance().get_name_or_address(nr));
    m_headerbar_chat.set_subtitle(Tox::instance().get_status_message(nr));
    m_tox_callback_chat = [this, nr](const ToxEvent& ev) {
        if (ev.type() == typeid(Tox::EventName)) {
            auto data = ev.get<Tox::EventName>();
            if (data.nr == nr) {
                m_headerbar_chat
                        .set_title(Tox::instance().get_name_or_address(nr));
            }
        } else if (ev.type() == typeid(Tox::EventStatusMessage)) {
            auto data = ev.get<Tox::EventStatusMessage>();
            if (data.nr == nr) {
                m_headerbar_chat
                        .set_subtitle(Tox::instance().get_status_message(nr));
            }
        }
    };

    // 6. change focus to inputfiled
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
    // TODO: implement a get_status_icon function
    switch (status_code) {
        case Tox::NONE:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_online);
            break;
        case Tox::BUSY:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_busy);
            break;
        case Tox::AWAY:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_away);
            break;
        default:
            break;
    }
    Tox::instance().save();
}

void DialogContact::exit() {
    Tox::instance().save();
    // TODO: ask for confirmation
    Gtk::Main::quit();
}

void DialogContact::change_name(Glib::ustring name, Glib::ustring msg) {
    Tox::instance().set_name(name);
    Tox::instance().set_status_message(msg);
    m_headerbar_contact.set_title(Tox::instance().get_name_or_address());
    m_headerbar_contact.set_subtitle(Tox::instance().get_status_message());
    Tox::instance().save();
}

void DialogContact::delete_friend(Tox::FriendNr nr) {
    m_contact.delete_contact(nr);
    Tox::instance().del_friend(nr);
    Tox::instance().save();
}
