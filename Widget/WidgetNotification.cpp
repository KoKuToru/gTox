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
#include "WidgetNotification.h"
#include <libnotifymm.h>

WidgetNotification::WidgetNotification(){
    set_name("WidgetNotification");

    //not working with css ? why ?
    m_remove.set_margin_left(5);
    m_remove.set_margin_right(5);
    m_remove.set_margin_top(5);
    m_remove.set_margin_bottom(5);

    m_button.set_margin_left(5);
    m_button.set_margin_right(5);
    m_button.set_margin_top(5);
    m_button.set_margin_bottom(5);

    m_remove.set_label("X");
    attach(m_remove, 3, 0, 1, 2);
    attach(m_button, 1, 0, 1, 2);
    attach(m_title, 2, 0, 1, 1);
    attach(m_message, 2, 1, 1, 1);
    m_title.set_hexpand(true);

    m_title.set_halign(Gtk::Align::ALIGN_START);
    m_message.set_halign(Gtk::Align::ALIGN_START);

    m_title.set_text("Placeholder");
    m_message.set_text("Placeholder");

    m_button.signal_clicked().connect(sigc::mem_fun(this, &WidgetNotification::on_button_clicked));
}

WidgetNotification::~WidgetNotification() {

}

void WidgetNotification::add_notification(Glib::ustring titel, Glib::ustring message, Glib::ustring button, std::function<void(void)> callback) {
    Notify::Notification("gTox", titel, message).show();
    m_messages.push_back({titel, message, button, callback});

    m_title.set_text(titel);
    m_message.set_text(message);
    m_button.set_label(button);
    if (button.empty()) {
        m_button.hide();
    } else {
        m_button.show();
    }
    m_callback = callback;

    show();
}

void WidgetNotification::on_button_clicked() {
    m_callback();

    hide();
}
