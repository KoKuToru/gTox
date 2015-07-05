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
#include "widget/chat_input.h"
#include "widget/chat_bubble.h"

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
    m_builder.get_widget("chat_box", m_chat_box);

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

    m_input->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        auto text_buffer = m_input->get_buffer();
        if (event->keyval == GDK_KEY_Return && !(event->state & GDK_SHIFT_MASK)) {
            if (text_buffer->begin() != text_buffer->end()) {
                 bool allow_send = text_buffer->get_text().find_first_not_of(" \t\n\v\f\r") != std::string::npos;
                 if (allow_send) {
                     if (Glib::str_has_prefix(text_buffer->get_text(), "/me ")) {
                         m_contact->send_action(m_input->get_serialized_text());
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

    m_contact->signal_send_message().connect(sigc::track_obj([this](Glib::ustring message, std::shared_ptr<toxmm2::receipt> receipt) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        while(true) {
            auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);
            //check_timestamp(timestamp);
            if (m_last_bubble.side == SIDE::OWN &&
                bubble != nullptr) {
                /*auto message_widget = widget::chat_message::create(receipt, message);
                auto row_widget     = widget::chat_row::create(receipt, timestamp, Gtk::manage(message_widget.raw()));
                bubble->add_row(*Gtk::manage(row_widget.raw()));
                m_dispatcher.emit([]() {
                    row_widget.raw()->property_reveal_child() = true;
                });*/
                m_last_bubble.timestamp = timestamp;
                return;
            }
            //need a new bubble
            auto bubble_widget = widget::chat_bubble::create(/*SIDE::OWN, */m_contact);
            m_last_bubble.side = SIDE::OWN;
            m_last_bubble.widget = Gtk::manage(bubble_widget.raw());
            m_last_bubble.timestamp = timestamp;
            m_chat_box->add(*m_last_bubble.widget);
            //no goto.. thats why while(true) !
        }
    }, *this));

    m_contact->signal_send_action().connect(sigc::track_obj([this](Glib::ustring action, std::shared_ptr<toxmm2::receipt> receipt) {
        auto timestamp = Glib::DateTime::create_now_utc().to_unix();
        /*auto action_widget = widget::chat_action(receipt, m_contact, action);
        auto row_widget    = widget::chat_row::create(receipt, timestamp, Gtk::manage(action_widget.raw()));
        m_last_bubble.side = SIDE::NONE;
        m_last_bubble.widget = Gtk::manage(row_widget.raw());
        m_last_bubble.timestamp = timestamp;
        m_chat_box->add(*m_last_bubble.widget);*/
    }, *this));
}

chat::~chat() {
    m_main->chat_remove(*m_headerbar_attached, *m_body);
}

void chat::activated() {
    if (is_visible()) {
        present();
    } else {
        m_main->chat_show(*m_headerbar_attached, *m_body);
    }
}
