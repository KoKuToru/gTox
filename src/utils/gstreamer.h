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
#ifndef GTOX_GSTREAMER_H
#define GTOX_GSTREAMER_H

#include <gtkmm.h>
#include <glibmm.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/appsink.h>
#include "dispatcher.h"

namespace utils {
    class gstreamer: public Glib::Object {
        private:
            utils::dispatcher m_dispatcher;

            Glib::RefPtr<Gst::PlayBin> m_playbin;
            Glib::RefPtr<Gst::AppSink> m_appsink;

            Glib::RefPtr<Glib::Binding> m_uri_binding;
            Glib::RefPtr<Glib::Binding> m_volume;

        public:
            auto property_uri()      -> Glib::PropertyProxy<Glib::ustring>;
            auto property_state()    -> Glib::PropertyProxy<Gst::State>;
            auto property_position() -> Glib::PropertyProxy<gint64>;
            auto property_duration() -> Glib::PropertyProxy_ReadOnly<gint64>;
            auto property_volume()   -> Glib::PropertyProxy<double>;
            auto property_pixbuf()   -> Glib::PropertyProxy_ReadOnly<Glib::RefPtr<Gdk::Pixbuf>>;

            using type_signal_error = sigc::signal<void, Glib::ustring>;

            auto signal_error() -> type_signal_error;

            gstreamer();
            virtual ~gstreamer();

            gstreamer(const gstreamer&) = delete;
            void operator=(const gstreamer&) = delete;

        private:
            Glib::Property<Glib::ustring>             m_property_uri;
            Glib::Property<Gst::State>                m_property_state;
            Glib::Property<gint64>                    m_property_position;
            Glib::Property<gint64>                    m_property_duration;
            Glib::Property<double>                    m_property_volume;
            Glib::Property<Glib::RefPtr<Gdk::Pixbuf>> m_property_pixbuf;

            type_signal_error m_signal_error;
    };
}
#endif
