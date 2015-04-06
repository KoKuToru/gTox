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
#include "WidgetAudioVideo.h"
#include "Generated/icon.h"
#include <glibmm/i18n.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
#endif
#include <gstreamermm/bus.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/clock.h>
#include <gstreamermm/buffer.h>
#include <gstreamermm/event.h>
#include <gstreamermm/message.h>
#include <gstreamermm/query.h>
#include <gstreamermm/videooverlay.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/ximagesink.h>
#include <gstreamermm/appsink.h>


#include <iostream>

WidgetAudioVideo::WidgetAudioVideo() : Glib::ObjectBase("WidgetAudioVideo") {
    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto playbin = Gst::PlayBin::create("playbin");

    //setup pipeline
    auto sink = Gst::AppSink::create();
    sink->property_max_buffers() = 3;
    sink->property_drop() = true;
    sink->property_caps() = Gst::Caps::create_from_string("video/x-raw,format=RGB,pixel-aspect-ratio=1/1");
    playbin->property_video_sink() = sink; //overwrite sink

    //open video
    playbin->property_uri() = "v4l2:///dev/video0";

    //when to
    auto btn = Gtk::manage(new Gtk::Button("CLICK ME"));
    btn->signal_clicked().connect([this, sink]() {
        //TODO !! ERROR CHECKING !

        auto sample = sink->pull_sample();
        auto caps   = sample->get_caps();
        auto struc  = caps->get_structure(0);
        auto buffer = sample->get_buffer();

        auto map = Glib::RefPtr<Gst::MapInfo>(new Gst::MapInfo);
        buffer->map(map, Gst::MAP_READ);

        int w, h;
        if (!(struc.get_field("width", w) && struc.get_field("height", h))) {
            std::cerr << "Couldn't get dimension" << std::endl;
            return;
        }

        //read memory
        auto pix = Gdk::Pixbuf::create_from_data((const guint8*)map->get_data(),
                                                 Gdk::COLORSPACE_RGB,
                                                 false,
                                                 8,
                                                 w,
                                                 h,
                                                 GST_ROUND_UP_4( w*3 ));
        auto img = Gtk::manage(new Gtk::Image(pix));
        pack_start(*img);
        img->show();
    });

    btn->show();
    pack_end(*btn);

    //start recording..
    playbin->set_state(Gst::STATE_PLAYING);
}

WidgetAudioVideo::~WidgetAudioVideo() {
}
