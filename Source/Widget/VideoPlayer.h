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
#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <gtkmm.h>
#include <glibmm.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/appsink.h>
class VideoPlayer : public Gtk::DrawingArea {
    private:
        enum State {
            INIT,
            PLAY,
            PAUSE,
            STOP
        };

        Glib::RefPtr<Gst::PlayBin> m_playbin;
        Glib::RefPtr<Gst::AppSink> m_appsink;
        Glib::RefPtr<Gdk::Pixbuf>  m_lastimg;
        State                      m_state;

        sigc::connection m_signal_helper;

        int m_videowidth;
        int m_videoheight;

    public:
        class Device {
            public:
                Glib::ustring name;
                GstDevice* device;

                Device(const Glib::ustring& name, GstDevice* device): name(name), device((decltype(device))gst_object_ref(device)) {
                }
                Device(const Device& other): name(other.name), device((decltype(device))gst_object_ref(other.device)) {
                }
                void operator=(const Device& other) {
                    name = other.name;
                    auto tmp = device;
                    device = (decltype(device))gst_object_ref(other.device);
                    gst_object_unref(tmp);
                }
                ~Device() {
                    gst_object_unref(device);
                }
        };

        VideoPlayer();
        virtual ~VideoPlayer();

        bool set_uri(Glib::ustring uri);
        bool set_device(Device uri);

        void play();
        void pause();
        void stop();

        Glib::RefPtr<Gdk::Pixbuf> snapshot();

        constexpr static const char* CLASS_VIDEO_INPUT = "Video/Source";
        constexpr static const char* CLASS_AUDIO_INPUT = "Audio/Source";
        constexpr static const char* CLASS_AUDIO_OUTPUT = "Audio/Sink";

        static std::vector<Device> probe_devices(const char* classes = CLASS_VIDEO_INPUT);

    protected:
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
