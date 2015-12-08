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
#ifndef GTOX_WEBCAM_H
#define GTOX_WEBCAM_H

#include <gtkmm.h>
#include <glibmm.h>
#include <gstreamermm/pipeline.h>
#include <gstreamermm/appsink.h>
#include "dispatcher.h"
#include "utils/debug.h"

namespace utils {
    class webcam: public Glib::Object, public debug::track_obj<webcam> {
        private:
            utils::dispatcher m_dispatcher;

            Glib::RefPtr<Gst::Pipeline> m_pipeline;
            Glib::RefPtr<Gst::AppSink>  m_appsink;

            void init();
            void destroy();

        public:
            auto property_device()-> Glib::PropertyProxy<std::shared_ptr<GstDevice>>;
            auto property_state() -> Glib::PropertyProxy<Gst::State>;
            auto property_pixbuf()-> Glib::PropertyProxy_ReadOnly<Glib::RefPtr<Gdk::Pixbuf>>;

            using type_signal_error = sigc::signal<void, Glib::ustring>;

            auto signal_error() -> type_signal_error;

            webcam();
            virtual ~webcam();

            webcam(const webcam&) = delete;
            void operator=(const webcam&) = delete;

            // ! the following function will block !
            static std::vector<std::shared_ptr<GstDevice>> get_webcam_devices();
            static Glib::ustring get_webcam_device_name(const std::shared_ptr<GstDevice>& device);
            // ! the following function will block !
            static std::shared_ptr<GstDevice> get_webcam_device_by_name(const Glib::ustring& name);

        private:
            Glib::Property<std::shared_ptr<GstDevice>> m_property_device;
            Glib::Property<Gst::State>                 m_property_state;
            Glib::Property<Glib::RefPtr<Gdk::Pixbuf>>  m_property_pixbuf;

            type_signal_error m_signal_error;

            static Glib::RefPtr<Gdk::Pixbuf> extract_frame(Glib::RefPtr<Gst::Sample> sample,
                                                           std::shared_ptr<std::pair<int, int> > resolution);
    };
}
#endif
