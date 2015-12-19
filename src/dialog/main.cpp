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
#include "main.h"
#include <gdkmm.h>
#include <iostream>
#include <glibmm/i18n.h>

#include "tox/core.h"
#include "tox/contact/manager.h"
#include "tox/contact/contact.h"

#include "dialog/main.h"
#include "widget/contact.h"

#include "gtox.h"

#ifndef SIGC_CPP11_HACK
#define SIGC_CPP11_HACK
namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}
#endif
using namespace dialog;

main::main(BaseObjectType* cobject,
           utils::builder builder,
           const Glib::ustring& file)
    : Gtk::Window(cobject)
{
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { file.raw() });
    m_storage = std::make_shared<utils::storage>();
    m_toxcore = toxmm::core::create(file, m_storage);
    m_menu = std::make_shared<widget::main_menu>(*this);
    m_config = std::make_shared<class config>(
                   Glib::build_filename(
                       Glib::get_user_config_dir(),
                       "gtox",
                       m_toxcore->property_addr_public().get_value(),
                       "config.bin"));

    builder.get_widget("headerbar", m_headerbar);
    builder.get_widget("status_btn", m_btn_status);
    builder.get_widget("stack_header", m_stack_header);
    builder.get_widget("stack", m_stack);
    builder.get_widget("list", m_list_contact);
    builder.get_widget("headerbar", m_headerbar);
    builder.get_widget("stack_header", m_stack_header);
    builder.get_widget("list", m_list_contact);
    builder.get_widget("list_active_chat", m_list_contact_active);
    builder.get_widget("list_notify", m_list_notify);
    builder.get_widget("contact_scroll", m_list_contact_scroll);
    builder.get_widget("status_icon", m_status_icon);
    builder.get_widget("request_revealer", m_request_revealer);
    builder.get_widget("request_btn", m_request_btn);

    set_icon(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/icon_128.svg"));
    signal_delete_event().connect(sigc::track_obj([this](GdkEventAny*) {
        // minimize window on close-btn
        iconify();
        return true;
    }, *this));
    signal_configure_event().connect_notify(sigc::track_obj([this](GdkEventConfigure*) {
        property_gravity() = Gdk::GRAVITY_NORTH_WEST;
        int root_x, root_y;
        get_position(root_x, root_y);
        int w, h;
        get_size(w, h);

        m_store_pos_size.disconnect();
        m_store_pos_size = Glib::signal_timeout()
                            .connect(sigc::track_obj([this, root_x, root_y, w, h]() {
            // store in config
            m_config->property_window_x() = root_x;
            m_config->property_window_y() = root_y;
            m_config->property_window_w() = w;
            m_config->property_window_h() = h;
            return false;
        },*this), 1000, Glib::PRIORITY_LOW);
    }, *this));

    set_border_width(0);

    if (m_config->property_window_x() == -1 ||
        m_config->property_window_y() == -1 ||
        m_config->property_window_w() == -1 ||
        m_config->property_window_h() == -1) {
        set_default_geometry(300, 600);
        set_position(Gtk::WindowPosition::WIN_POS_CENTER);
    } else {
        resize(m_config->property_window_w(),
               m_config->property_window_h());
        move(m_config->property_window_x(),
             m_config->property_window_y());
    }

    set_title("gTox");


    m_binding_title = Glib::Binding::bind_property(m_toxcore->property_name_or_addr(),
                                                   m_headerbar->property_title(),
                                                   Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_subtitle = Glib::Binding::bind_property(m_toxcore->property_status_message(),
                                                      m_headerbar->property_subtitle(),
                                                      Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    //Connect the 2 paneds
    auto paned_top = builder.get_widget<Gtk::Paned>("paned_top");
    auto paned_bottom = builder.get_widget<Gtk::Paned>("paned_bottom");

    m_binding_position = Glib::Binding::bind_property(paned_top->property_position(),
                                                      paned_bottom->property_position(),
                                                      Glib::BINDING_DEFAULT |
                                                      Glib::BINDING_BIDIRECTIONAL |
                                                      Glib::BINDING_SYNC_CREATE);

    m_stack_header->signal_map().connect_notify([this](){
        m_headerbar->get_style_context()->add_class("gtox-headerbar-right");
    });
    m_stack_header->signal_unmap().connect_notify([this](){
        m_headerbar->get_style_context()->remove_class("gtox-headerbar-right");
    });

    auto setting_btn = builder.get_widget<Gtk::Button>("setting_btn");
    setting_btn->signal_clicked().connect([this, setting_btn]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_menu->is_visible()) {
            return;
        }
        m_menu->set_position(Gtk::POS_BOTTOM);
        m_menu->set_relative_to(*setting_btn);
        m_menu->set_visible();
    });

    auto activated = [this](Gtk::ListBoxRow* row) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        //FORWARD SIGNAL TO THE ITEM
        auto item = dynamic_cast<widget::contact*>(row);
        if (item) {
            item->activated();
        }
    };
    m_list_contact->signal_row_activated().connect(activated);
    m_list_contact_active->signal_row_activated().connect(activated);

    auto sort_func = [this](Gtk::ListBoxRow* a, Gtk::ListBoxRow* b) {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
        auto item_a = dynamic_cast<widget::contact*>(a);
        auto item_b = dynamic_cast<widget::contact*>(b);
        if (item_a == nullptr) {
            return 0;
        }
        if (item_b == nullptr) {
            return 0;
        }
        return item_a->compare(item_b);
    };
    m_list_contact->set_sort_func(sort_func);
    m_list_contact_active->set_sort_func(sort_func);

    load_contacts();

    //Menu items
    auto contact_remove = Gtk::manage(new Gtk::MenuItem(_("Remove contact"), true));

    m_popup_menu.append(*contact_remove);
    m_popup_menu.accelerate(*this);
    m_popup_menu.show_all();

    m_list_contact->signal_button_press_event().connect_notify([this](GdkEventButton* event) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
            auto item = m_list_contact->get_row_at_y(event->y);
            if (item) {
                m_list_contact->select_row(*item);
                m_popup_menu.popup(event->button, event->time);
            }
        }
    });

    contact_remove->signal_activate().connect([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto row = dynamic_cast<widget::contact*>(m_list_contact->get_selected_row());
        if (!row) {
            return;
        }
        auto contact = row->get_contact();

        Gtk::Window& parent = dynamic_cast<Gtk::Window&>(*this->get_toplevel());
        Gtk::MessageDialog msg(parent,
                               _("Remove contact"),
                               false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_OK_CANCEL,
                               true);
        msg.set_secondary_text(
                    Glib::ustring::compose(_("Are you sure you want to remove this contact?"),
                                           contact->property_name_or_addr()));
        if (msg.run() != Gtk::RESPONSE_OK) {
            return;
        }

        auto cm = contact->contact_manager();
        if (cm) {
            cm->remove_contact(contact);
        }
    });

    m_toxcore
            ->contact_manager()
            ->signal_removed()
            .connect(sigc::track_obj(
                         [this](std::shared_ptr<toxmm::contact> contact) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        for (auto widget: {m_list_contact, m_list_contact_active}) {
            for (auto item: widget->get_children()) {
                auto item_r = dynamic_cast<widget::contact*>(item);
                if (item_r && item_r->get_contact() == contact) {
                    widget->remove(*item_r);
                    delete item_r;
                }
            }
        }
    }, *this));

    auto update_status_icon = sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto connection = m_toxcore->property_connection().get_value();
        if (connection == TOX_CONNECTION_NONE) {
            m_status_icon->set_from_icon_name("status_offline", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
            m_status_icon->reset_style();
            m_status_icon->queue_resize();
            return;
        }
        std::string tcp = "";
        if (connection == TOX_CONNECTION_TCP) {
            tcp = "_tcp";
        }
        switch (m_toxcore->property_status().get_value()) {
            case TOX_USER_STATUS_AWAY:
                m_status_icon->set_from_icon_name("status_away", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
            case TOX_USER_STATUS_BUSY:
                m_status_icon->set_from_icon_name("status_busy", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
            case TOX_USER_STATUS_NONE:
                m_status_icon->set_from_icon_name("status_online", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
                break;
        }
        m_status_icon->reset_style();
        m_status_icon->queue_resize();
    }, *this);

    m_toxcore->property_status().signal_changed().connect(update_status_icon);
    m_toxcore->property_connection().signal_changed().connect(update_status_icon);

    m_toxcore->contact_manager()->signal_removed().connect(sigc::track_obj([this](std::shared_ptr<toxmm::contact> contact) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { contact->property_name_or_addr().get_value().raw() });
        //remove contact from list
        for (auto item_ : m_list_contact->get_children()) {
            auto item = dynamic_cast<widget::contact*>(item_);
            if (item && item->get_contact() == contact) {
                delete item;
                break;
            }
        }
        for (auto item_ : m_list_contact_active->get_children()) {
            auto item = dynamic_cast<widget::contact*>(item_);
            if (item && item->get_contact() == contact) {
                delete item;
                break;
            }
        }
        m_list_contact->invalidate_sort();
        m_list_contact_active->invalidate_sort();
    }, *this));

    m_toxcore->contact_manager()->signal_added().connect(sigc::track_obj([this](std::shared_ptr<toxmm::contact> contact) {
    utils::debug::scope_log log(DBG_LVL_2("gtox"), { contact->property_name_or_addr().get_value().raw() });
        //add contact to list
        auto item_builder = widget::contact::create(*this, contact);
        auto item = Gtk::manage(item_builder.raw());
        m_list_contact->add(*item);

        item_builder = widget::contact::create(*this, contact, true);
        item = Gtk::manage(item_builder.raw());
        m_list_contact_active->add(*item);

        m_list_contact->invalidate_sort();
        m_list_contact_active->invalidate_sort();
    }, *this));

    //setup status change menu
    m_btn_status->set_use_popover(true);
    auto menu_builder = Gtk::Builder::create_from_resource("/org/gtox/ui/status_menu.ui");
    Glib::RefPtr<Glib::Object> object = menu_builder->get_object("status-menu");
    Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    m_btn_status->set_menu_model(gmenu);

    m_action = Gio::SimpleActionGroup::create();
    m_action->add_action("online", [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_toxcore->property_status() = TOX_USER_STATUS_NONE;
    });
    m_action->add_action("busy", [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_toxcore->property_status() = TOX_USER_STATUS_BUSY;
    });
    m_action->add_action("away", [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_toxcore->property_status() = TOX_USER_STATUS_AWAY;
    });
    m_action->add_action("offline", [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        hide();
    });
    m_action->add_action("switch", [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        exit();
        gTox::instance()->activate();
    });

    insert_action_group("status", m_action);

    m_binding_contact_active = Glib::Binding::bind_property(
                                   config()->property_contacts_display_active(),
                                   m_list_contact_active->property_visible(),
                                   Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    auto update_request = [this, format = m_request_btn->get_label()]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_requests.empty())  {
            m_request_revealer->property_reveal_child() = false;
        } else {
            m_request_btn->set_label(Glib::ustring::compose(
                                         format,
                                         m_requests.size()));
            m_request_revealer->property_reveal_child() = true;
        }
    };

    m_toxcore->contact_manager()->signal_request()
            .connect(sigc::track_obj([this,
                                     update_request](
                                     toxmm::contactAddrPublic addr,
                                     Glib::ustring message) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {
                                        std::string(addr),
                                        message.raw()
                                    });
        m_requests.push_back({addr, message});
        update_request();
    }, *this));

    m_request_btn->signal_clicked().connect([this, update_request]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_requests.empty()) {
            return;
        }
        Gtk::MessageDialog dialog(
                    *this,
                    _("Contact Request"),
                    false,
                    Gtk::MESSAGE_QUESTION,
                    Gtk::BUTTONS_YES_NO,
                    true);
        dialog.set_secondary_text(
                    Glib::ustring::compose(
                        _("%1 wants to add you to his/her contact list\n\nMessage: \n%2"),
                        std::string(m_requests.front().first),
                        m_requests.front().second));
        switch (dialog.run()) {
            case Gtk::RESPONSE_YES:
                m_toxcore->contact_manager()->add_contact(m_requests.front().first);
            case Gtk::RESPONSE_NO:
                m_requests.erase(m_requests.begin());
                update_request();
                break;
        }
    });
}

utils::builder::ref<main> main::create(const Glib::ustring& file) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { file.raw() });
    return utils::builder::create_ref<main>(
                "/org/gtox/ui/dialog_contact.ui",
                "dialog_contact",
                file);
}

void main::load_contacts() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    for (auto item : m_list_contact->get_children()) {
        delete item;
    }
    for (auto item : m_list_contact_active->get_children()) {
        delete item;
    }

    for (auto contact : m_toxcore->contact_manager()->get_all()) {
        //normal contact list:
        auto item_builder = widget::contact::create(*this, contact);
        auto item = Gtk::manage(item_builder.raw());
        m_list_contact->add(*item);

        //active chat list:
        item_builder = widget::contact::create(*this, contact, true);
        item = Gtk::manage(item_builder.raw());
        m_list_contact_active->add(*item);
    }

    m_list_contact->invalidate_sort();
    m_list_contact_active->invalidate_sort();
}

main::~main() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    for (auto item : m_list_contact->get_children()) {
        delete item;
    }
    for (auto item : m_list_contact_active->get_children()) {
        delete item;
    }
    // save ?
    auto t = tox();
    if (t) {
        t->save();
    }
}

void main::exit() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    hide();
}

void main::detachable_window_add(dialog::detachable_window* window) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    Gtk::Widget& headerbar = *window->property_headerbar();
    Gtk::Widget& body = *window->property_body();

    for (size_t i = 0; i < m_stack_data.size(); ++i) {
        if (m_stack_data[i].first == &headerbar
                && m_stack_data[i].second == &body) {
            m_stack_header->set_visible_child(headerbar);
            m_stack->set_visible_child(body);
            return;
        }
    }

    headerbar.get_style_context()->add_class("gtox-headerbar-left");

    m_stack_header->add(headerbar);
    m_stack->add(body);

    m_stack_header->set_visible_child(headerbar);
    m_stack->set_visible_child(body);

    m_stack_data.push_back({&headerbar, &body});

    //present
    property_gravity() = Gdk::GRAVITY_NORTH_EAST;
    if (!m_stack_header->is_visible()) {
        resize(800 + get_width(), get_height());
    }

    m_stack_header->show();
    m_stack->show();
}


void main::detachable_window_del(dialog::detachable_window* window) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    Gtk::Widget& headerbar = *window->property_headerbar();
    Gtk::Widget& body = *window->property_body();

    for (size_t i = 0; i < m_stack_data.size(); ++i) {
        if (m_stack_data[i].first == &headerbar && m_stack_data[i].second == &body) {
            m_stack_header->remove(headerbar);
            m_stack->remove(body);
            m_stack_data.erase(m_stack_data.begin() + i);
            break;
        }
    }

    if (m_stack_data.empty()) {
        property_gravity() = Gdk::GRAVITY_NORTH_EAST;
        if (m_stack_header->is_visible()) {
            resize(m_headerbar->get_width(), get_height());
        }
        m_stack_header->hide();
        m_stack->hide();
    }
}

std::shared_ptr<toxmm::core>& main::tox() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_toxcore;
}

std::shared_ptr<class config>& main::config() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_config;
}
