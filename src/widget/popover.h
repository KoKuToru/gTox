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
#ifndef WIDGETPOPOVERHACK_H
#define WIDGETPOPOVERHACK_H

#include <gtkmm.h>

namespace widget {
    /**
     * @brief The popover class
     *
     * This is a popover hack for non wayland enviroments.
     * It creates a maximized transparent window for the popover.
     * This adds the ability to render popovers bigger than the parent window.
     *
     */
    class popover: public Gtk::Popover {
        private:
            class popwin: public Gtk::Window {
                    Gtk::Widget* m_widget;
                public:
                    Gtk::Box area;
                    popwin(Gtk::Widget* widget);
                    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
            };

            popwin* m_popwin = nullptr;

            // no idea why I can't do this directly in popover
            class property_helper: public Glib::Object {
                public:
                    Glib::Property<Gtk::Widget*> m_property_relative_to;
                    property_helper():
                        Glib::ObjectBase(typeid(property_helper)),
                        m_property_relative_to(*this, "popover-relative-to") {}
            } m_property_helper;

        public:
            popover();
            ~popover();

            void show();
            void set_visible(bool v) {
                if (v) {
                    show();
                } else {
                    hide();
                }
            }

            Glib::PropertyProxy<Gtk::Widget*> property_relative_to();
            void set_relative_to(Gtk::Widget& widget);
            Gtk::Widget* get_relative_to();
    };
}
#endif
