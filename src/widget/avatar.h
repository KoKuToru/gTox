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
#ifndef WIDGETAVATAR_H
#define WIDGETAVATAR_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include "tox/types.h"

namespace widget {
    class avatar : public Gtk::Image {
        public:
            avatar(BaseObjectType* cobject,
                   utils::builder,
                   toxmm::contactAddrPublic addr);
            ~avatar();

        protected:
            bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
            Gtk::SizeRequestMode get_request_mode_vfunc() const override;
            void get_preferred_width_vfunc(int& minimum_width,
                                           int& natural_width) const override;
            void get_preferred_height_vfunc(int& minimum_height,
                                            int& natural_height) const override;

        private:
            class image: public Glib::Object {
                public:
                    Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf>> property_pixbuf();

                    image(toxmm::contactAddrPublic addr);

                private:
                    Glib::Property<Glib::RefPtr<Gdk::Pixbuf>> m_property_pixbuf;

                    Glib::RefPtr<Gio::File> m_file;
                    Glib::RefPtr<Gio::FileMonitor> m_monitor;
                    utils::dispatcher m_dispatcher;
                    int m_version = 0;

                    void load();
            };

            Glib::RefPtr<Glib::Binding> m_binding;
    };
}
#endif
