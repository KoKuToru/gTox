/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#include "chat.h"
#include "main.h"
#include "tox/contact/contact.h"
#include "tox/contact/file/manager.h"
#include "widget/chat_input.h"
#include "widget/chat_bubble.h"
#include "widget/chat_message.h"
#include "widget/chat_file.h"
#include "tox/contact/file/file.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace dialog;

chat::chat(Glib::RefPtr<dialog::main> main, std::shared_ptr<toxmm2::contact> contact):
    m_contact(contact),
    m_main(main),
    m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_chat.ui")) {

    m_builder.get_widget("chat_headerbar_attached", m_headerbar_attached);
    m_builder.get_widget("chat_headerbar_detached", m_headerbar_detached);
    m_builder.get_widget("chat_body", m_body);
    m_input = m_builder.get_widget_derived<widget::chat_input>("chat_input");
    m_builder.get_widget("chat_input_revealer", m_input_revealer);
    m_builder.get_widget("chat_input_format_revealer", m_input_format_revealer);
    m_builder.get_widget("btn_prev", m_btn_prev);
    m_builder.get_widget("btn_next", m_btn_next);
    m_builder.get_widget("chat_attach", m_btn_attach);
    m_builder.get_widget("chat_detach", m_btn_detach);
    m_builder.get_widget("close_btn_attached", m_btn_close_attached);
    m_builder.get_widget("close_btn_detached", m_btn_close_detached);
    m_builder.get_widget("chat_box", m_chat_box);
    m_builder.get_widget("eventbox", m_eventbox);
    m_builder.get_widget("scrolled", m_scrolled);
    m_builder.get_widget("viewport", m_viewport);

    m_main->chat_add(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);

    m_binding_name[0] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_attached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);
    m_binding_name[1] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_detached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_status[0] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                       m_headerbar_attached->property_subtitle(),
                                                       Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);
    m_binding_status[1] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                       m_headerbar_detached->property_subtitle(),
                                                       Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_online = Glib::Binding::bind_property(m_contact->property_connection(),
                                 m_input_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                                 [](const TOX_CONNECTION& connection, bool& is_online) {
        is_online = connection != TOX_CONNECTION_NONE;
        return true;
    });

    /*m_binding_focus = Glib::Binding::bind_property(m_input->property_has_focus(),
                                 m_input_format_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);*/

    set_titlebar(*m_headerbar_detached);
    m_btn_detach->signal_clicked().connect(sigc::track_obj([this]() {
        //TODO: take position w/h from main-window
        m_main->chat_remove(*m_headerbar_attached, *m_body);
        add(*m_body);
        show();
    }, *this));
    m_btn_attach->signal_clicked().connect(sigc::track_obj([this]() {
        remove();
        hide();
        m_main->chat_add(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);
    }, *this));
    m_btn_close_attached->signal_clicked().connect(sigc::track_obj([this]() {
        m_main->chat_remove(*m_headerbar_attached, *m_body);
    }, *this));
    m_btn_close_detached->signal_clicked().connect(sigc::track_obj([this]() {
        remove();
        hide();
    }, *this));

    m_input->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        auto text_buffer = m_input->get_buffer();
        if (event->keyval == GDK_KEY_Return && !(event->state & GDK_SHIFT_MASK)) {
            if (text_buffer->begin() != text_buffer->end()) {
                 bool allow_send = text_buffer->get_text().find_first_not_of(" \t\n\v\f\r") != std::string::npos;
                 if (allow_send) {
                     if (Glib::str_has_prefix(text_buffer->get_text(), "/me ")) {
                         m_contact->send_action(m_input->get_serialized_text().substr(4));
                     } else {
                         m_contact->send_message(m_input->get_serialized_text());
                     }
                     text_buffer->set_text("");
                     return true;
                 }
            }
        }
        return false;
    }, *this), false);

    m_contact->signal_send_message().connect(sigc::track_obj([this](Glib::ustring message, std::shared_ptr<toxmm2::receipt>) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        while(true) {
            auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);
            //check_timestamp(timestamp);
            if (m_last_bubble.side == SIDE::OWN &&
                bubble != nullptr) {
                auto message_widget = Gtk::manage(new widget::chat_message(message));
                bubble->add_row(*message_widget);
                m_last_bubble.timestamp = timestamp;
                return;
            }
            //need a new bubble
            auto bubble_widget = widget::chat_bubble::create(/*SIDE::OWN, */m_main->tox()/*, timestamp*/); //<- not correct !
            m_last_bubble.side = SIDE::OWN;
            m_last_bubble.widget = Gtk::manage(bubble_widget.raw());
            m_last_bubble.timestamp = timestamp;
            m_chat_box->add(*m_last_bubble.widget);
            //no goto.. thats why while(true) !
        }
    }, *this));

    m_contact->signal_recv_message().connect(sigc::track_obj([this](Glib::ustring message) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        while(true) {
            auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);
            //check_timestamp(timestamp);
            if (m_last_bubble.side == SIDE::OTHER &&
                bubble != nullptr) {
                auto message_widget = Gtk::manage(new widget::chat_message(message));
                bubble->add_row(*message_widget);
                m_last_bubble.timestamp = timestamp;
                return;
            }
            //need a new bubble
            auto bubble_widget = widget::chat_bubble::create(/*SIDE::OWN, */m_contact/*, timestamp*/); //<- not correct !
            m_last_bubble.side = SIDE::OTHER;
            m_last_bubble.widget = Gtk::manage(bubble_widget.raw());
            m_last_bubble.timestamp = timestamp;
            m_chat_box->add(*m_last_bubble.widget);
            //no goto.. thats why while(true) !
        }
    }, *this));

    m_contact->signal_send_action().connect(sigc::track_obj([this](Glib::ustring action, std::shared_ptr<toxmm2::receipt>) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        //auto action_widget = widget::chat_action(receipt, m_contact, action);
        m_last_bubble.side = SIDE::NONE;
        //TODO: just for testing now:
        m_last_bubble.widget = Gtk::manage(new widget::chat_message(m_main->tox()->property_name_or_addr() + " " + action));
        m_last_bubble.widget->property_halign() = Gtk::ALIGN_CENTER;
        //m_last_bubble.widget = Gtk::manage(row_widget.raw());
        m_last_bubble.timestamp = timestamp;
        m_chat_box->add(*m_last_bubble.widget);
    }, *this));

    m_contact->signal_recv_action().connect(sigc::track_obj([this](Glib::ustring action) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        //auto action_widget = widget::chat_action(receipt, m_contact, action);
        m_last_bubble.side = SIDE::NONE;
        //TODO: just for testing now:
        m_last_bubble.widget = Gtk::manage(new widget::chat_message(m_contact->property_name_or_addr().get_value() + " " + action));
        m_last_bubble.widget->property_halign() = Gtk::ALIGN_CENTER;
        //m_last_bubble.widget = Gtk::manage(row_widget.raw());
        m_last_bubble.timestamp = timestamp;
        m_chat_box->add(*m_last_bubble.widget);
    }, *this));

    m_contact->file_manager()->signal_recv_file().connect([this](std::shared_ptr<toxmm2::file>& file) {
        if (file->property_kind() != TOX_FILE_KIND_DATA) {
            return;
        }
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        while(true) {
            auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);
            //check_timestamp(timestamp);
            if (m_last_bubble.side == SIDE::OTHER &&
                bubble != nullptr) {
                auto b_ref = widget::file::create(file);
                auto widget = Gtk::manage(b_ref.raw());
                bubble->add_row(*widget);
                m_last_bubble.timestamp = timestamp;
                return;
            }
            //need a new bubble
            auto bubble_widget = widget::chat_bubble::create(/*SIDE::OWN, */m_contact/*, timestamp*/); //<- not correct !
            m_last_bubble.side = SIDE::OTHER;
            m_last_bubble.widget = Gtk::manage(bubble_widget.raw());
            m_last_bubble.timestamp = timestamp;
            m_chat_box->add(*m_last_bubble.widget);
            //no goto.. thats why while(true) !
        }
    });

    //logic for text-selection
    m_eventbox->add_events(Gdk::BUTTON_PRESS_MASK |
                           Gdk::BUTTON_RELEASE_MASK |
                           Gdk::BUTTON1_MOTION_MASK |
                           Gdk::KEY_PRESS_MASK);
    m_eventbox->set_can_focus(true);
    m_eventbox->signal_button_press_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        if (event->button != 1) {
            return false;
        }

        from_x = event->x;
        from_y = event->y;

        // update all children, reset selection
        GdkEventMotion dummy_event;
        dummy_event.x = from_x;
        dummy_event.y = from_y;
        update_children(&dummy_event, m_chat_box->get_children());

        grab_focus();

        return true;
    }, *this));
    m_eventbox->signal_button_release_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        if (event->button != 1) {
            return false;
        }
        from_x = -1;
        from_y = -1;
        return true;
    }, *this));
    m_eventbox->signal_motion_notify_event().connect(sigc::track_obj([this](GdkEventMotion* event) {
        if (from_x < 0 && from_y < 0) {
            return false;
        }
        // update all children
        update_children(event, m_chat_box->get_children());
        return true;
    }, *this));
    m_eventbox->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        if (!(event->state & GDK_CONTROL_MASK)) {
            return false;
        }
        if (event->keyval != 'c') {
            return false;
        }

        // copy to clipboard
        auto data = get_children_selection(m_chat_box->get_children());
        Gtk::Clipboard::get()->set_text(data);
        return true;
    }, *this));

    //auto scroll
    m_scrolled->get_vadjustment()->signal_value_changed()
            .connect_notify(sigc::track_obj([this]() {
        // check if lowest position
        auto adj = m_scrolled->get_vadjustment();
        m_autoscroll = adj->get_upper() - adj->get_page_size()
                       == adj->get_value();
    }, *this));
    m_chat_box->signal_size_allocate()
            .connect_notify(sigc::track_obj([this](Gtk::Allocation&) {
        // auto scroll:
        if (m_autoscroll) {
            auto adj = m_scrolled->get_vadjustment();
            adj->set_value(adj->get_upper() - adj->get_page_size());
        }
    }, *this));
    m_scrolled->signal_size_allocate()
            .connect_notify(sigc::track_obj([this](Gtk::Allocation&) {
        // auto scroll:
        if (m_autoscroll) {
            auto adj = m_scrolled->get_vadjustment();
            adj->set_value(adj->get_upper() - adj->get_page_size());
        }
    }, *this));

    // Disable auto scroll to focused child
    auto dummy_adj = Gtk::Adjustment::create(0, 0, 0);
    m_viewport->set_focus_hadjustment(dummy_adj);
    m_viewport->set_focus_vadjustment(dummy_adj);

    //drag and drop
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("text/uri-list"));
    m_scrolled->drag_dest_set(targets,
                              Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP,
                              Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
    m_scrolled->signal_drag_data_received().connect(sigc::track_obj([this](const Glib::RefPtr<Gdk::DragContext>&, int, int, const Gtk::SelectionData& data, guint, guint) {
        for (auto uri : data.get_uris()) {
            //do something with the files
        }
    }, this));
}

chat::~chat() {
    m_main->chat_remove(*m_headerbar_attached, *m_body);
}

void chat::activated() {
    if (is_visible()) {
        present();
    } else {
        m_main->chat_show(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);
    }
}

void chat::update_children(GdkEventMotion* event,
                           std::vector<Gtk::Widget*> children) {
    for (auto c : children) {
        // check if it's a container
        auto c_container = dynamic_cast<Gtk::Container*>(c);
        if (c_container) {
            update_children(event, c_container->get_children());
            continue;
        }
        // check if it's WidgetChatMessage
        auto c_message = dynamic_cast<widget::label*>(c);
        if (!c_message) {
            //
            continue;
        }
        // correct x,y to widget x,y
        int w_from_x, w_from_y, w_to_x, w_to_y;
        int to_x = event->x;
        int to_y = event->y;
        if (m_chat_box->translate_coordinates(*c_message, from_x, from_y, w_from_x, w_from_y) &&
            m_chat_box->translate_coordinates(*c_message, to_x, to_y, w_to_x, w_to_y)) {

            // fix order
            if (w_to_y < w_from_y) {
                std::swap(w_to_y, w_from_y);
                std::swap(w_to_x, w_from_x);
            }

            // check if within
            if (w_from_y < c_message->get_allocated_height() && w_to_y > 0) {
                c_message->on_selection(w_from_x, w_from_y, w_to_x, w_to_y);
            } else {
                c_message->on_selection(0, 0, 0, 0);
            }
        }
    }
}

Glib::ustring chat::get_children_selection(
    std::vector<Gtk::Widget*> children) {
    Glib::ustring res;
    Glib::ustring tmp;
    for (auto c : children) {
        tmp.clear();

        // check if it's a container
        auto c_container = dynamic_cast<Gtk::Container*>(c);
        if (c_container) {
            tmp = get_children_selection(c_container->get_children());
        }
        // check if it's WidgetChatMessage
        auto c_message = dynamic_cast<widget::label*>(c);
        if (c_message) {
            tmp = c_message->get_selection();
        }

        // add to result
        if (!tmp.empty()) {
            if (!res.empty()) {
                res += '\n';
            }
            res += tmp;
        }
    }
    return res;
}
