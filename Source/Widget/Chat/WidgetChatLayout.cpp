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
#include "WidgetChatLayout.h"
#include <pangomm/renderer.h>
#include "Widget/Chat/WidgetChatBubble.h"
#include "WidgetChatLabel.h"

WidgetChatLayout::WidgetChatLayout() : Glib::ObjectBase("WidgetChatLayout") {
    add(m_vbox);
    show_all();

    m_vbox.set_valign(Gtk::ALIGN_END);

    add_events(Gdk::BUTTON_PRESS_MASK);
    add_events(Gdk::BUTTON_RELEASE_MASK);
    add_events(Gdk::BUTTON1_MOTION_MASK);
    add_events(Gdk::KEY_PRESS_MASK);

    set_can_focus(true);
}

WidgetChatLayout::~WidgetChatLayout() {
}

void WidgetChatLayout::set_spacing(int space) {
    m_vbox.set_spacing(space);
}

void WidgetChatLayout::pack_start(Gtk::Widget& w, bool a, bool b) {
    m_vbox.pack_start(w, a, b);
}

std::vector<Gtk::Widget*> WidgetChatLayout::get_children() {
    return m_vbox.get_children();
}

std::vector<const Gtk::Widget*> WidgetChatLayout::get_children() const {
    return m_vbox.get_children();
}

bool WidgetChatLayout::on_button_press_event(GdkEventButton* event) {
    if (event->button != 1) {
        return false;
    }

    from_x = event->x;
    from_y = event->y;

    // update all children, reset selection
    GdkEventMotion dummy_event;
    dummy_event.x = from_x;
    dummy_event.y = from_y;
    update_children(&dummy_event, get_children());

    grab_focus();

    return true;
}

bool WidgetChatLayout::on_button_release_event(GdkEventButton* event) {
    if (event->button != 1) {
        return false;
    }
    from_x = -1;
    from_y = -1;
    return true;
}

bool WidgetChatLayout::on_motion_notify_event(GdkEventMotion* event) {
    if (from_x < 0 && from_y < 0) {
        return false;
    }
    // update all children
    update_children(event, get_children());
    return true;
}

void WidgetChatLayout::update_children(GdkEventMotion* event,
                                       std::vector<Gtk::Widget*> children) {
    for (auto c : children) {
        // check if it's a container
        auto c_container = dynamic_cast<Gtk::Container*>(c);
        if (c_container) {
            update_children(event, c_container->get_children());
            continue;
        }
        // check if it's WidgetChatMessage
        auto c_message = dynamic_cast<WidgetChatLabel*>(c);
        if (!c_message) {
            //
            continue;
        }
        // correct x,y to widget x,y
        int w_from_x, w_from_y, w_to_x, w_to_y;
        int to_x = event->x;
        int to_y = event->y;
        if (translate_coordinates(
                *c_message, from_x, from_y, w_from_x, w_from_y)
            && translate_coordinates(*c_message, to_x, to_y, w_to_x, w_to_y)) {

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

Glib::ustring WidgetChatLayout::get_children_selection(
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
        auto c_message = dynamic_cast<WidgetChatLabel*>(c);
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

bool WidgetChatLayout::on_key_press_event(GdkEventKey* event) {
    if (!(event->state & GDK_CONTROL_MASK)) {
        return false;
    }
    if (event->keyval != 'c') {
        return false;
    }

    // copy to clipboard
    auto data = get_children_selection(get_children());
    Gtk::Clipboard::get()->set_text(data);
    return true;
}

