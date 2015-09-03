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
#include "chat_message.h"
#include "utils/debug.h"

using namespace widget;

chat_message::label::label(Glib::PropertyProxy_ReadOnly<Glib::ustring> name,
                           Glib::DateTime time,
                           const Glib::ustring& message):
    widget::label(message),
    m_name(name),
    m_time(time) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    name.get_value().raw(),
                                    time.format("%c").raw()
                                });
}

Glib::ustring chat_message::label::get_selection() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    auto selection = widget::label::get_selection();
    if (selection.length() == get_text().length()) {
        selection = Glib::ustring::compose("[%2] %1: %3",
                                           m_name,
                                           m_time.format("%c"),
                                           selection);
    }
    return selection;
}

chat_message::chat_message(Glib::PropertyProxy_ReadOnly<Glib::ustring> name,
                           Glib::DateTime time,
                           const Glib::ustring& text):
    m_label(name, time.to_local(), text) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    name.get_value().raw(),
                                    time.format("%c").raw()
                                });
    show();
    add(m_label);
    property_reveal_child() = false;
    m_dispatcher.emit([this]() {
        utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
        property_reveal_child() = true;
    });
}
