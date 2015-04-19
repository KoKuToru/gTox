/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "Generated/layout.h"
#include "Generated/icon.h"

WidgetNotification* WidgetNotification::create(DialogContact::EventAddNotification event) {
    auto builder = Gtk::Builder::create_from_string(LAYOUT::list_item_notification);
    WidgetNotification* tmp = nullptr;
    builder->get_widget_derived("contact_list_item", tmp);
    tmp->set_event(event);
    tmp->show();
    return tmp;
}

WidgetNotification::WidgetNotification(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ListBoxRow(cobject),
      m_builder(builder) {

}

WidgetNotification::~WidgetNotification() {
}

void WidgetNotification::set_event(DialogContact::EventAddNotification event) {
    m_event = event;

    Gtk::Label* title;
    Gtk::Label* message;
    Gtk::Image* image;
    Gtk::Image* icon;

    m_builder->get_widget("title", title);
    m_builder->get_widget("message", message);
    m_builder->get_widget("image", image);
    m_builder->get_widget("icon", icon);

    title->set_text(m_event.title);
    message->set_text(m_event.message);
    if (m_event.image) {
        image->property_pixbuf() = m_event.image->scale_simple(
                                       64,
                                       64,
                                       Gdk::INTERP_BILINEAR);
    }

    icon->property_pixbuf() = ICON::load_icon(ICON::notification);
}

void WidgetNotification::activated() {
    ToxEventCallback::notify(m_event.activated_action);
}
