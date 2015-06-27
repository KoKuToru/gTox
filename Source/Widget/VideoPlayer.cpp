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
#include "VideoPlayer.h"
#include <gstreamermm/bus.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/buffer.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/appsink.h>
#include <gstreamermm/elementfactory.h>
#include <glibmm.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

void VideoPlayer::init() {
    m_playing = false;

    signal_unmap().connect_notify([this](){
        m_signal_connection.disconnect();
        m_error_connection.disconnect();
        m_streamer.reset();
    });

    signal_map().connect_notify([this](){
        if (!m_uri.empty()) {
            bool playing = m_playing;
            set_uri(m_uri);
            if (playing) {
                play();
            }
        }
    });
}

VideoPlayer::VideoPlayer(BaseObjectType* cobject, gToxBuilder): Gtk::DrawingArea(cobject) {
    init();
}

VideoPlayer::VideoPlayer() : Glib::ObjectBase("VideoPlayer") {
    init();
}

VideoPlayer::~VideoPlayer() {
    m_signal_connection.disconnect();
    m_streamer.reset();
}

static gStreamerHelper streamer;

bool VideoPlayer::set_uri(Glib::ustring uri) {
    stop();

    m_uri = uri;

    m_last_error.clear();

    m_signal_connection.disconnect();
    m_error_connection.disconnect();

    m_streamer = streamer.create(uri);

    m_signal_connection = m_streamer->signal_update().connect([this](int w, int h, const std::vector<unsigned char>& frame) {
        m_lastimg_data = frame;
        m_lastimg = Gdk::Pixbuf::create_from_data(m_lastimg_data.data(),
                                                  Gdk::COLORSPACE_RGB,
                                                  false,
                                                  8,
                                                  w,
                                                  h,
                                                  GST_ROUND_UP_4( w*3 ));

        if (m_w != w || m_h != h) {
            m_w = w;
            m_h = h;
        }
        queue_draw();
    });

    m_error_connection = m_streamer->signal_error().connect([this](std::string error) {
        m_last_error = error;
        queue_draw();
    });

    queue_draw();
    m_streamer->emit_update_signal();

    return true;
}

void VideoPlayer::play() {
    m_playing = true;
    if (get_mapped()) {
        if (m_streamer) {
            m_streamer->play();
        }
    }
}

void VideoPlayer::pause() {
    m_playing = false;
    if (m_streamer) {
        m_streamer->pause();
    }
}

void VideoPlayer::stop() {
    m_playing = false;
    if (m_streamer) {
        m_streamer->stop();
    }
}

Glib::RefPtr<Gdk::Pixbuf> VideoPlayer::snapshot() {
    return m_lastimg;
}

bool VideoPlayer::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    auto img = snapshot();
    if (img) {
        cr->save();

        double w_scaled;
        double h_scaled;

        int max_size = std::max(512, std::min(get_width(), get_height()));

        if (m_w > m_h) {
            w_scaled = max_size;
            h_scaled = m_h * max_size / m_w;
        } else {
            h_scaled = get_height();
            w_scaled = m_w * max_size / m_h;
        }
        set_size_request(w_scaled, h_scaled);
        cr->scale(w_scaled / m_w, h_scaled / m_h);

        Gdk::Cairo::set_source_pixbuf(cr, img);
        cr->paint();
        cr->restore();
    }
    if (!m_last_error.empty()) {
        //render text
        cr->save();

        Gdk::RGBA color;

        color.set_rgba(0.0, 0.0, 0.0, 0.7);
        Gdk::Cairo::set_source_rgba(cr, color);

        Cairo::TextExtents extents;
        cr->get_text_extents(m_last_error, extents);

        if (m_w < 320 || m_w < (extents.width + 10) || m_h < 240) {
            m_w = std::max(320, (int)extents.width + 10);
            m_h = 240;
            set_size_request(m_w, m_h);
            queue_resize();
            return true;
        }

        cr->translate((get_width() - extents.width) / 2, (get_height() + extents.height) / 2);
        cr->rectangle(-5, 5, extents.width + 10, -(extents.height + 10));
        cr->fill();

        color.set_rgba(1.0, 1.0, 1.0, 1.0);
        Gdk::Cairo::set_source_rgba(cr, color);
        cr->show_text(m_last_error);

        cr->restore();
    }
    return true;
}

std::shared_ptr<gStreamerVideo> VideoPlayer::get_streamer() {
    return m_streamer;
}
