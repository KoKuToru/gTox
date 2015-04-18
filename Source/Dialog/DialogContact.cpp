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
#include "Generated/layout.h"
#include <gdkmm.h>
#include "Tox/Toxmm.h"
#include <iostream>
#include <libnotifymm.h>
#include "Generated/theme.h"
#include <iostream>
#include <glibmm/i18n.h>
#include "Helper/Canberra.h"
#include "Widget/WidgetContactListItem.h"

DialogContact* DialogContact::m_instance = nullptr;

DialogContact::DialogContact(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject), m_builder(builder) {
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

    m_builder->get_widget("headerbar", m_headerbar);
    m_builder->get_widget("status_btn", m_btn_status);
    m_builder->get_widget("stack_header", m_stack_header);
    m_builder->get_widget("stack", m_stack);

    set_icon(ICON::load_icon(ICON::icon_128));

    set_border_width(0);
    set_default_geometry(300, 600);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    set_title("gTox");

    //Connect the 2 paneds
    Gtk::Paned* paned_top;
    Gtk::Paned* paned_bottom;
    m_builder->get_widget("paned_top", paned_top);
    m_builder->get_widget("paned_bottom", paned_bottom);

    // Connect properties C++ version ?
    g_object_bind_property(
        paned_top->gobj(),
        "position",
        paned_bottom->gobj(),
        "position",
        GBindingFlags(G_BINDING_DEFAULT |
                      G_BINDING_BIDIRECTIONAL |
                      G_BINDING_SYNC_CREATE));

    m_builder->get_widget("headerbar", m_headerbar);
    m_builder->get_widget("stack_header", m_stack_header);

    m_headerbar->set_title(Toxmm::instance().get_name_or_address());
    m_headerbar->set_subtitle(Toxmm::instance().get_status_message());

    m_stack_header->signal_map().connect_notify([this](){
        m_headerbar->get_style_context()->add_class("gtox-headerbar-right");
    });
    m_stack_header->signal_unmap().connect_notify([this](){
        m_headerbar->get_style_context()->remove_class("gtox-headerbar-right");
    });

    m_btn_status->set_image(m_icon_status);
    m_btn_status->set_sensitive(false);
    m_btn_status->signal_clicked().connect([this]() {
        if (!m_popover_status) {
            m_popover_status = std::make_shared<PopoverStatus>(*m_btn_status);
        }
        m_popover_status->set_visible();
    });

    Gtk::Button* add_contact_btn;
    m_builder->get_widget("add_contact_btn", add_contact_btn);
    add_contact_btn->set_image(*Gtk::manage(new Gtk::Image(ICON::load_icon(ICON::plus))));
    add_contact_btn->signal_clicked().connect([this, add_contact_btn]() {
        if (!m_popover_add_contact) {
            m_popover_add_contact = std::make_shared<PopoverAddContact>(*add_contact_btn);
        }
        m_popover_add_contact->set_visible();
    });

    Gtk::Button* setting_btn;
    m_builder->get_widget("setting_btn", setting_btn);
    setting_btn->set_image(*Gtk::manage(new Gtk::Image(ICON::load_icon(ICON::settings))));
    setting_btn->signal_clicked().connect([this, setting_btn]() {
        if (!m_popover_settings) {
            m_popover_settings = std::make_shared<PopoverSettings>(*setting_btn);
        }
        m_popover_settings->set_visible();
    });

    Gtk::ListBox* list;
    m_builder->get_widget("list", list);
    list->signal_row_activated().connect([this](Gtk::ListBoxRow* row) {
        //FORWARD SIGNAL TO THE ITEM
        WidgetContactListItem* item = dynamic_cast<WidgetContactListItem*>(row);
        ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventActivated{item}));
    });

    load_contacts();

    set_status(Toxmm::OFFLINE);

    m_update_interval = Glib::signal_timeout().connect(
        sigc::mem_fun(this, &DialogContact::update),
        Toxmm::instance().update_optimal_interval());
}

std::shared_ptr<DialogContact> DialogContact::create(Glib::ustring file) {
    Toxmm::instance().init(file);
    auto builder = Gtk::Builder::create_from_string(LAYOUT::dialog_contact);
    DialogContact* tmp = nullptr;
    builder->get_widget_derived("dialog_contact", tmp);
    tmp->m_instance = tmp;
    return std::shared_ptr<DialogContact>(tmp);
}

void DialogContact::load_contacts() {
    Gtk::ListBox* list;
    m_builder->get_widget("list", list);
    if (!list) {
        return;
    }
    for (auto item : list->get_children()) {
        delete item;
    }
    bool first = true;
    for (auto contact : Toxmm::instance().get_friendlist()) {
        auto item = Gtk::manage(WidgetContactListItem::create(contact));
        list->add(*item);
        if (first) {
            //pixel perfect list size
            int min_height;
            int natural_height;
            item->get_preferred_height(min_height, natural_height);
            Gtk::ScrolledWindow* scroll;
            m_builder->get_widget("contact_scroll", scroll);
            scroll->set_size_request(-1, min_height*7);
            first = false;
        }
    }
}

DialogContact::~DialogContact() {
    // save ?
    Toxmm::instance().destroy();
}

bool DialogContact::update() {
    if (!m_btn_status->get_sensitive() && Toxmm::instance().is_connected()) {
        m_btn_status->set_sensitive(true);
        set_status(Toxmm::instance().get_status());
    }

    ToxEvent ev;
    while (Toxmm::instance().update(ev)) {
        ToxEventCallback::notify(ev);
    }

    return true;
}

void DialogContact::tox_event_handling(const ToxEvent& ev) {
    if (ev.type() == typeid(Toxmm::EventFriendRequest)) {
        auto data = ev.get<Toxmm::EventFriendRequest>();
        /*
        m_notification.add_notification(
            "Friend request [" + Toxmm::to_hex(data.addr.data(), 32) + "]",
            data.message,
            "Accept",
            [this, data]() {
                add_contact(Toxmm::instance().add_friend_norequest(data.addr));
                Toxmm::instance().save();
            });*/
    } else if (ev.type() == typeid(EventAttachWidget)) {
        auto data = ev.get<EventAttachWidget>();

        property_gravity() = Gdk::GRAVITY_NORTH_EAST;
        if (!m_stack_header->is_visible()) {
            resize(600 + get_width(), get_height());
        }

        data.header->get_style_context()->add_class("gtox-headerbar-left");
        m_stack_header->add(*data.header);
        m_stack_header->set_visible_child(*data.header);
        m_stack->add(*data.body);
        m_stack->set_visible_child(*data.body);
        m_stack_header->show();
        m_stack->show();
    } else if (ev.type() == typeid(EventDetachWidget)) {
        auto data = ev.get<EventDetachWidget>();

        /*property_gravity() = Gdk::GRAVITY_NORTH_WEST;
        int x, y, w, h;
        get_position(x, y);
        get_size(w, h);
        w -= m_headerbar->get_width();*/

        data.header->get_style_context()->remove_class("gtox-headerbar-left");
        m_stack_header->remove(*data.header);
        m_stack->remove(*data.body);

        auto child = m_stack->get_visible_child();
        if (!child || data.close) {
            property_gravity() = Gdk::GRAVITY_NORTH_EAST;
            if (m_stack_header->is_visible()) {
                resize(m_headerbar->get_width(), get_height());
            }
            m_stack_header->hide();
            m_stack->hide();
        }
    } else if (ev.type() == typeid(EventSetName)) {
        auto data = ev.get<EventSetName>();

        Toxmm::instance().set_name(data.name);
        Toxmm::instance().set_status_message(data.status);
        m_headerbar->set_title(Toxmm::instance().get_name_or_address());
        m_headerbar->set_subtitle(Toxmm::instance().get_status_message());
        Toxmm::instance().save();
    } else if (ev.type() == typeid(EventSetStatus)) {
        auto data = ev.get<EventSetStatus>();
        set_status(data.status_code);
    } else if (ev.type() == typeid(EventPresentWidget)) {
        auto data = ev.get<EventPresentWidget>();

        m_stack_header->set_visible_child(*data.header);
        m_stack->set_visible_child(*data.body);
    }
}

DialogContact& DialogContact::instance() {
    if (m_instance == nullptr) {
        throw "Error";
    }
    return *m_instance;
}

void DialogContact::destroy() {
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void DialogContact::set_status(Toxmm::EUSERSTATUS status_code) {
    Toxmm::instance().set_status(status_code);
    // TODO: implement a get_status_icon function
    switch (status_code) {
        case Toxmm::NONE:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_online);
            break;
        case Toxmm::BUSY:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_busy);
            break;
        case Toxmm::AWAY:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_away);
            break;
        default:
            m_icon_status.property_pixbuf()
                = ICON::load_icon(ICON::status_offline);
            break;
    }
    if (!m_status_icon) {
        m_status_icon = Gtk::StatusIcon::create(m_icon_status.property_pixbuf());
        m_status_icon->set_visible(true);
        m_status_icon->set_name("gTox");
        m_status_icon->signal_activate().connect([this](){
            present();
        });
        auto menu = Gtk::manage(new Gtk::Menu);
        menu->add(*Gtk::manage(new Gtk::MenuItem("Test")));
        menu->show_all();
        m_status_icon->signal_popup_menu().connect([this, menu](const unsigned int& btn, const unsigned int& time){
            m_status_icon->popup_menu_at_position(*menu, btn, time);
        });
    } else {
        m_status_icon->set(m_icon_status.property_pixbuf());
    }
}

void DialogContact::exit() {
    Toxmm::instance().save();
    // TODO: ask for confirmation
    Gtk::Main::quit();
}
