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
#include "Helper/gStreamerHelper.h"
#include "Helper/gToxBuilder.h"

class VideoPlayer : public Gtk::DrawingArea {
    private:
        std::vector<unsigned char>  m_lastimg_data;
        Glib::RefPtr<Gdk::Pixbuf>   m_lastimg;
        bool m_playing;
        bool m_auto_reset = true;

        int m_w = 1;
        int m_h = 1;
        double m_w_scaled = 1;
        double m_h_scaled = 1;

        std::shared_ptr<gStreamerVideo> m_streamer;
        sigc::connection m_signal_connection;
        sigc::connection m_error_connection;

        Glib::ustring m_uri;
        Glib::ustring m_last_error;

        void init();

    public:
        VideoPlayer();
        VideoPlayer(BaseObjectType* cobject, gToxBuilder builder);
        virtual ~VideoPlayer();

        bool set_uri(Glib::ustring uri, bool generate_preview = true);

        //default = true
        void set_auto_reset(bool enable);

        void play();
        void pause();
        void stop();

        void set_volume(double vol);

        Glib::RefPtr<Gdk::Pixbuf> snapshot();

        std::shared_ptr<gStreamerVideo> get_streamer();

    protected:
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
