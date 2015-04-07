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
            PLAYING,
            PAUSE,
            STOP
        };

        Glib::RefPtr<Gst::PlayBin> m_playbin;
        Glib::RefPtr<Gst::AppSink> m_appsink;
        sigc::connection           m_interval;
        int                        m_fps;
        Glib::RefPtr<Gdk::Pixbuf>  m_lastimg;
        State                      m_state;

        int m_videowidth;
        int m_videoheight;
    public:
        VideoPlayer();
        virtual ~VideoPlayer();

        bool set_uri(Glib::ustring uri);
        void set_fps(int fps);

        void play();
        void pause();
        void stop();

        Glib::RefPtr<Gdk::Pixbuf> snapshot();
    protected:
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
