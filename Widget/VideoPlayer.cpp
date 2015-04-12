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
#include <gstreamermm/caps.h>
#include <gstreamermm/buffer.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/appsink.h>

#include <iostream>
#include <memory>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

VideoPlayer::VideoPlayer() : Glib::ObjectBase("VideoPlayer"), m_state(STOP) {
    m_playbin = Gst::PlayBin::create("playbin");

    //setup pipeline
    m_appsink = Gst::AppSink::create();
    m_appsink->property_caps() = Gst::Caps::create_from_string("video/x-raw,format=RGB,pixel-aspect-ratio=1/1");
    m_appsink->property_max_buffers() = 3;
    m_appsink->property_drop() = true;
    m_playbin->property_video_sink() = m_appsink; //overwrite sink

    //events
    m_appsink->property_emit_signals() = true;
    m_appsink->signal_new_sample().connect_notify([this](){
        auto sample = m_appsink->pull_sample();
        if (!sample) {
            return;
        }
        //switch to GTK-Mainloop
        m_signal_helper = Glib::signal_idle().connect([this, sample](){
            auto state = m_state;
            if (state == INIT) {
                auto caps = sample->get_caps();

                if (!caps) {
                    return false;
                }

                if (caps->empty()) {
                    return false;
                }

                auto struc = caps->get_structure(0);
                if (!(struc.get_field("width", m_videowidth) &&
                      struc.get_field("height", m_videoheight))) {
                    return false;
                }

                set_size_request(m_videowidth, m_videoheight);
                queue_resize();

                m_state = STOP;
                m_playbin->set_state(Gst::STATE_NULL);
            }
            if (state == PLAY || state == INIT) {
                auto buffer = sample->get_buffer();
                if (!buffer) {
                    return false;
                }

                auto map = Glib::RefPtr<Gst::MapInfo>(new Gst::MapInfo);
                if (!buffer->map(map, Gst::MAP_READ)) {
                    return false;
                }

                //convert to pixbuf
                m_lastimg = Gdk::Pixbuf::create_from_data(map->get_data(),
                                                          Gdk::COLORSPACE_RGB,
                                                          false,
                                                          8,
                                                          m_videowidth,
                                                          m_videoheight,
                                                          GST_ROUND_UP_4( m_videowidth*3 ),
                                                          [sample, buffer, map](const guint8*){
                            //free memory
                            buffer->unmap(map);
                });

                queue_draw();
            }
            return false;
        });
    });
}

VideoPlayer::~VideoPlayer() {
    stop();
}

bool VideoPlayer::set_uri(Glib::ustring uri) {
    stop();
    m_playbin->property_uri() = uri;

    //Get first frame:
    m_lastimg.reset();
    m_state = INIT;
    m_playbin->set_state(Gst::STATE_PLAYING);
    return true;
}

void VideoPlayer::play() {
    if (m_state == INIT) {
        throw std::runtime_error("NOT READY");
    }
    m_state = PLAY;
    m_playbin->set_state(Gst::STATE_PLAYING);
}

void VideoPlayer::pause() {
    if (m_state == INIT) {
        throw std::runtime_error("NOT READY");
    }
    m_state = PAUSE;
    m_playbin->set_state(Gst::STATE_PAUSED);
}

void VideoPlayer::stop() {
    m_state = STOP;
    m_playbin->set_state(Gst::STATE_NULL);
}

Glib::RefPtr<Gdk::Pixbuf> VideoPlayer::snapshot() {
    return m_lastimg;
}

bool VideoPlayer::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    auto img = snapshot();
    if (img) {
        Gdk::Cairo::set_source_pixbuf(cr, img, 0, 0);
        cr->paint();
    }
    return true;
}
