/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#ifndef WIDGETAUDIOVIDEO_H
#define WIDGETAUDIOVIDEO_H

#include <gtkmm.h>
#include <gstreamermm.h>
#include <gstreamermm/playbin.h>
class WidgetAudioVideo : public Gtk::VBox {
    private:
        Gtk::DrawingArea m_videoarea;
        gulong m_x_window_id;
        Glib::RefPtr<Gst::PlayBin> playbin;
    public:
        WidgetAudioVideo();
        ~WidgetAudioVideo();
    protected:
        void on_bus_message_sync(const Glib::RefPtr<Gst::Message>& message);
        bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);
        Gst::PadProbeReturn on_video_pad_got_buffer(const Glib::RefPtr<Gst::Pad>& pad, const Gst::PadProbeInfo& data);
};

#endif
