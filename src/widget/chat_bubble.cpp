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
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    builder.get_widget("row_box", m_row_box);
    builder.get_widget("username", m_username);
    builder.get_widget("frame", m_frame);

    property_reveal_child() = false;
    m_dispatcher.emit([this]() {
        property_reveal_child() = true;
    });

    //change size of avatar based on the size of the rows in row_box
    m_frame->signal_size_allocate().connect_notify(sigc::track_obj([this](Gtk::Allocation& allocation) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto w = std::min(64, allocation.get_height() - 5); //5px radius
        if (w != m_avatar->get_width()) {
            m_dispatcher.emit([this, w]() {
                m_avatar->set_size_request(w, w);
            });
        }
    }, *this));
}

chat_bubble::chat_bubble(BaseObjectType* cobject,
                         utils::builder builder,
                         Glib::PropertyProxy_ReadOnly<Glib::ustring> username,
                         Glib::PropertyProxy_ReadOnly<toxmm::contactAddrPublic> addr,
                         Glib::DateTime time): Gtk::Revealer(cobject) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_avatar = builder
               .get_widget_derived<avatar>("avatar",
                                           addr);
    init(builder);

    auto format = m_username->property_label().get_value();
    auto transform_text = [this, format, time](const Glib::ustring& input,
                          Glib::ustring& output) {
        utils::debug::scope_log log(DBG_LVL_4("gtox"), { input.raw() });
        auto escaped_username = Glib::Markup::escape_text(input);
        auto datetime     = time.to_local();
        auto datetime_now = Glib::DateTime::create_now_local();

        auto hour_diff = datetime_now.difference(datetime) / G_TIME_SPAN_HOUR;
        auto min_diff  = datetime_now.difference(datetime) / G_TIME_SPAN_MINUTE;

        Glib::ustring date;
        if (datetime.get_year() != datetime_now.get_year()) {
            //display day month year and time
            date = datetime.format(_("%e. %B %Y, %R"));
        } else if (datetime.get_week_of_year() != datetime_now.get_week_of_year()) {
            //display day month and time
            date = datetime.format(_("%e. %B, %R"));
        } else if (hour_diff >= 24) {
            //display weekday and time
            date = datetime.format(_("%a. %B, %R"));
        } else if (min_diff >= 60) {
            //display difference in h
            date = Glib::ustring::compose(_("%1 hr"), hour_diff);
        } else if (min_diff >= 1) {
            //display difference in min
            date = Glib::ustring::compose(_("%1 min"), min_diff);
        } else {
            //display just now
            date = _("Just now");
        }

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

    Glib::signal_timeout().connect_seconds(sigc::track_obj([this, username, transform_text]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        Glib::ustring input = username;
        Glib::ustring output;
        transform_text(input, output);
        m_username->property_label() = output;
        return true;
    },*this), 60);
}

chat_bubble::~chat_bubble() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

utils::builder::ref<chat_bubble> chat_bubble::create(std::shared_ptr<toxmm::core> core, Glib::DateTime time) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    core->property_name_or_addr().get_value().raw(),
                                    time.format("%c").raw()
                                });
    return utils::builder::create_ref<chat_bubble>(
                "/org/gtox/ui/chat_bubble_right.ui",
                "chat_bubble",
                core->property_name_or_addr(),
                core->property_addr_public(),
                time);
}

utils::builder::ref<chat_bubble> chat_bubble::create(std::shared_ptr<toxmm::contact> contact, Glib::DateTime time) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    contact->property_name_or_addr().get_value().raw(),
                                    time.format("%c").raw()
                                });
    return utils::builder::create_ref<chat_bubble>(
                "/org/gtox/ui/chat_bubble_left.ui",
                "chat_bubble",
                contact->property_name_or_addr(),
                contact->property_addr_public(),
                time);
}

void chat_bubble::add_row(Gtk::Widget& widget) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_row_box->add(widget);
}
