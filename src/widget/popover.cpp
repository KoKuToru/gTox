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
#include "popover.h"
#ifdef GDK_WINDOWING_WAYLAND
    #include <gdk/gdkwayland.h>
#endif
#include <gdk/gdkcairo.h>

using namespace widget;

popover::popwin::popwin(Gtk::Widget* widget)
    : m_widget(widget) {
    add(area);
    area.show();
    set_decorated(false);
    set_app_paintable(true);
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
    auto visual = get_screen()->get_rgba_visual();
    if (visual) {
        gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
}

bool popover::popwin::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    auto res = Gtk::Window::on_draw(cr);

    int x, y;
    auto alloc = m_widget->get_allocation();
    m_widget->translate_coordinates(*this,
                                    alloc.get_x(),
                                    alloc.get_y(),
                                    x,
                                    y);

    input_shape_combine_region(Cairo::Region::create(Cairo::RectangleInt{
                                                         x,
                                                         y,
                                                         alloc.get_width(),
                                                         alloc.get_height()
                                                     }));

    return res;
}

popover::popover() {
    signal_unmap().connect_notify(sigc::track_obj([this]() {
        if (m_popwin) {
            delete m_popwin;
            m_popwin = nullptr;
        }
    }, *this), true);
    property_relative_to().signal_changed().connect(sigc::track_obj([this]() {
        if (m_popwin) {
            delete m_popwin;
            show();
        }
    }, *this));
}

popover::~popover() {
    if (m_popwin) {
        delete m_popwin;
        m_popwin = nullptr;
    }
}

void popover::show() {
    if (m_popwin) {
        return;
    }

    if (!get_relative_to()) {
        // need a relative to
        return;
    }

#ifdef GDK_IS_WAYLAND_DISPLAY
    if (GDK_IS_WAYLAND_DISPLAY(gtk_widget_get_display(GTK_WIDGET(gobj())))) {
        // normal simple route
        Gtk::Popover::set_relative_to(*get_relative_to());
        Gtk::Popover::show();
        return;
    }
#endif
    auto visual = get_relative_to()->get_screen()->get_rgba_visual();
    if (!visual) {
        // normal route with cropped popover
        Gtk::Popover::set_relative_to(*get_relative_to());
        Gtk::Popover::show();
        return;
    }

    // open transparent window
    m_popwin = new popwin(this);
    m_popwin->set_focus_on_map();

    // set right screen
    m_popwin->set_screen(get_relative_to()->get_screen());
    // get monitor dimensions
    int mon_id = get_relative_to()->get_screen()
                 ->get_monitor_at_window(get_relative_to()->get_window());
    Gdk::Rectangle rect = m_popwin->get_screen()
                          ->get_monitor_workarea(mon_id);
    // move/resize to right monitor
    m_popwin->move(rect.get_x(), rect.get_y());
    m_popwin->resize(rect.get_width(), rect.get_height());
    m_popwin->set_transient_for(
                dynamic_cast<Gtk::Window&>(
                    *get_relative_to()->get_toplevel()));
    m_popwin->show_now();

    // calculate position for popover
    int wx, wy, nx, ny;
    get_relative_to()->get_toplevel()->get_window()
            ->get_root_origin(wx, wy);
    get_relative_to()
            ->translate_coordinates(*get_relative_to()->get_toplevel(),
                                    0, 0,
                                    nx, ny);
    wx += nx;
    wy += ny;
    int ww = get_relative_to()->get_allocated_width();
    int wh = get_relative_to()->get_allocated_height();;
    int rx = 0, ry = 0;
    m_popwin->get_window()->get_root_origin(rx, ry);

    // set position for popover
    Gtk::Popover::set_relative_to(m_popwin->area);
    set_pointing_to(Gdk::Rectangle(wx-rx, wy-ry, ww, wh));

    // open popover
    Gtk::Popover::show();

    // the following generates WARNINGS ?
    // GLib-GObject-WARNING **: gsignal.c:2595: handler 'x' of instance 'y' is not blocked
    get_relative_to()->get_toplevel()->signal_focus_in_event()
            .connect_notify(sigc::track_obj([this](GdkEventFocus*) {
        Gtk::Popover::hide();
    }, *this));
}

void popover::set_relative_to(Gtk::Widget& widget) {
    property_relative_to() = &widget;
}

Gtk::Widget* popover::get_relative_to() {
    return property_relative_to();
}

Glib::PropertyProxy<Gtk::Widget*> popover::property_relative_to() {
    return m_property_helper.m_property_relative_to.get_proxy();
}
