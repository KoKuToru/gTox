/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#include "chat_bubble.h"
#include "avatar.h"
#include "tox/contact/contact.h"
#include "tox/core.h"
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace widget;

void chat_bubble::init(utils::builder builder) {
    builder.get_widget("row_box", m_row_box);
    builder.get_widget("username", m_username);
    builder.get_widget("frame", m_frame);

    property_reveal_child() = false;
    m_dispatcher.emit([this]() {
        property_reveal_child() = true;
    });

    //change size of avatar based on the size of the rows in row_box
    m_frame->signal_size_allocate().connect_notify(sigc::track_obj([this](Gtk::Allocation& allocation) {
        auto w = std::min(64, allocation.get_height() - 5); //5px radius
        if (w != m_avatar->get_width()) {
            m_dispatcher.emit([this, w]() {
                m_avatar->set_size_request(w, w);
            });
        }
    }, *this));
}

chat_bubble::chat_bubble(BaseObjectType* cobject, utils::builder builder, std::shared_ptr<toxmm2::core> core): Gtk::Revealer(cobject) {
    m_avatar = builder
               .get_widget_derived<avatar>("avatar",
                                           core->property_addr_public());
    init(builder);

    auto username = core->property_name_or_addr();
    auto format = m_username->property_label().get_value();
    auto transform_text = [this, format](const Glib::ustring& input,
                          Glib::ustring& output) {
        auto escaped_username = Glib::Markup::escape_text(input);
        auto date = Glib::DateTime::create_now_local(
                        Glib::DateTime::create_now_utc().to_unix())
                    .format(_("DATE_FORMAT"));
        //TODO: use right timestamp
        //TODO: only show month,year when different than today
        output = Glib::ustring::compose(format,
                                        escaped_username,
                                        date);
        return true;
    };

    auto flag = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;
    m_binding_name = Glib::Binding::bind_property(username,
                                                  m_username->property_label(),
                                                  flag,
                                                  sigc::track_obj(transform_text,
                                                                  *this));
}

chat_bubble::chat_bubble(BaseObjectType* cobject, utils::builder builder, std::shared_ptr<toxmm2::contact> contact): Gtk::Revealer(cobject) {
    m_avatar = builder
               .get_widget_derived<avatar>("avatar",
                                           contact->property_addr_public());
    init(builder);

    auto username = contact->property_name_or_addr();
    auto format = m_username->property_label().get_value();
    auto transform_text = [this, format](const Glib::ustring& input,
                          Glib::ustring& output) {
        auto escaped_username = Glib::Markup::escape_text(input);
        auto date = Glib::DateTime::create_now_local(
                        Glib::DateTime::create_now_utc().to_unix())
                    .format(_("DATE_FORMAT"));
        //TODO: use right timestamp
        //TODO: only show month,year when different than today
        output = Glib::ustring::compose(format,
                                        escaped_username,
                                        date);
        return true;
    };

    auto flag = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;
    m_binding_name = Glib::Binding::bind_property(username,
                                                  m_username->property_label(),
                                                  flag,
                                                  sigc::track_obj(transform_text,
                                                                  *this));
}

chat_bubble::~chat_bubble() {
    //
}

utils::builder::ref<chat_bubble> chat_bubble::create(std::shared_ptr<toxmm2::core> contact) {
    return utils::builder(Gtk::Builder::create_from_resource("/org/gtox/ui/chat_bubble_right.ui"))
            .get_widget_derived<chat_bubble>("chat_bubble", contact);
}

utils::builder::ref<chat_bubble> chat_bubble::create(std::shared_ptr<toxmm2::contact> contact) {
    return utils::builder(Gtk::Builder::create_from_resource("/org/gtox/ui/chat_bubble_left.ui"))
            .get_widget_derived<chat_bubble>("chat_bubble", contact);
}

void chat_bubble::add_row(Gtk::Widget& widget) {
    m_row_box->add(widget);
}
