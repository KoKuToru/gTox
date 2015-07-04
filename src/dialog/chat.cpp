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
    m_builder.get_widget("chat_input", m_input);
    m_builder.get_widget("chat_input_revealer", m_input_revealer);
    m_builder.get_widget("chat_input_format_revealer", m_input_format_revealer);
    m_builder.get_widget("btn_prev", m_btn_prev);
    m_builder.get_widget("btn_next", m_btn_next);
    m_builder.get_widget("chat_attach", m_btn_attach);
    m_builder.get_widget("chat_detach", m_btn_detach);

    m_main->chat_add(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);

    m_binding_name[0] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_attached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);
    m_binding_name[1] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_detached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_name[0] = Glib::Binding::bind_property(m_contact->property_status_message(),
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

    m_binding_focus = Glib::Binding::bind_property(m_input->property_has_focus(),
                                 m_input_format_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

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
