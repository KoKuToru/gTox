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

#include <iostream>

WidgetAudioVideo::WidgetAudioVideo() : Glib::ObjectBase("WidgetAudioVideo") {
    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;
    pack_start(m_videoarea, Gtk::PACK_EXPAND_WIDGET);
    m_videoarea.set_size_request(256, 256);
    m_videoarea.show();

    playbin = Gst::PlayBin::create("playbin");

    // Get the bus from the pipeline:
    auto bus = playbin->get_bus();

    // Enable synchronous message emission to set up video (if any) at the
    // exact appropriate time
    bus->enable_sync_message_emission();

    // Connect to bus's synchronous message signal (this is done so that
    // m_video_area can be set up for drawing at the exact appropriate time):
    bus->signal_sync_message()
            .connect(sigc::mem_fun(*this,
                                   &WidgetAudioVideo::on_bus_message_sync));

    // Add a bus watch to receive messages from the pipeline's bus:
    bus->add_watch(sigc::mem_fun(*this, &WidgetAudioVideo::on_bus_message) );

    playbin->signal_video_changed().connect([this]() {
        Glib::RefPtr<Gst::Pad> pad = playbin->get_video_pad(0);
        if(pad) {
            // Add a buffer probe to the video sink pad which will be removed after
            // the first buffer is received in the on_video_pad_got_buffer method.
            // When the first buffer arrives, the video size can be extracted.
            pad->add_probe(Gst::PAD_PROBE_TYPE_BUFFER, sigc::mem_fun(*this, &WidgetAudioVideo::on_video_pad_got_buffer));
         }
    });

    m_videoarea.signal_realize().connect([this]() {
#ifdef GDK_WINDOWING_X11
        m_x_window_id = GDK_WINDOW_XID(m_videoarea.get_window()->gobj());
#endif
#ifdef GDK_WINDOWING_WIN32
        m_x_window_id = GDK_WINDOW_HWND(m_videoarea.get_window()->gobj());
#endif
    });

    //open video ?
    playbin->property_uri() = "v4l2:///dev/video0";

    //when to: playbin->set_state(Gst::STATE_PLAYING);
    auto btn = Gtk::manage(new Gtk::Button("CLICK ME"));
    btn->signal_clicked().connect([this]() {
        playbin->set_state(Gst::STATE_PLAYING);
    });
    btn->show();
    pack_end(*btn);
}

WidgetAudioVideo::~WidgetAudioVideo() {
}

// This function is used to receive asynchronous messages from mainPipeline's
// bus, specifically to prepare the Gst::XOverlay to draw inside the window
// in which we want it to draw to.
void WidgetAudioVideo::on_bus_message_sync(const Glib::RefPtr<Gst::Message>& message) {
  // ignore anything but 'prepare-xwindow-id' element messages
  if(message->get_message_type() != Gst::MESSAGE_ELEMENT) {
      return;
  }

  if(!message->get_structure().has_name("prepare-window-handle")) {
     return;
  }

  /*
   *
   * This just wont work:
   *
  auto element = Glib::RefPtr<Gst::Element>::cast_dynamic(message->get_source());

  auto videooverlay = Glib::RefPtr<Gst::VideoOverlay>::cast_dynamic(element);

  if(videooverlay) {
      videooverlay->set_window_handle(m_x_window_id);
  }*/

  //force it ?
  playbin->set_window_handle(m_x_window_id);
}

// This function is used to receive asynchronous messages from play_bin's bus
bool WidgetAudioVideo::on_bus_message(const Glib::RefPtr<Gst::Bus>& , const Glib::RefPtr<Gst::Message>& message) {
  switch(message->get_message_type()) {
    case Gst::MESSAGE_EOS:
    {
      //EOF ?
      break;
    }
    case Gst::MESSAGE_ERROR:
    {
      auto msgError = Glib::RefPtr<Gst::MessageError>::cast_static(message);
      if(msgError) {
          Glib::Error err;
          err = msgError->parse();
          std::cerr << "Error: " << err.what() << std::endl;
      } else {
          std::cerr << "Error." << std::endl;
      }
      break;
    }
    default:
          std::cout << "debug: on_bus_message: unhandled message=" << G_OBJECT_TYPE_NAME(message->gobj()) << std::endl;
  }

  return true;
}

Gst::PadProbeReturn WidgetAudioVideo::on_video_pad_got_buffer(const Glib::RefPtr<Gst::Pad>& pad, const Gst::PadProbeInfo&) {
  int width_value;
  int height_value;

  Glib::RefPtr<Gst::Caps> caps = pad->query_caps(Glib::RefPtr<Gst::Caps>());

  caps = caps->create_writable();

  const Gst::Structure structure = caps->get_structure(0);
  if(structure) {
    structure.get_field("width", width_value);
    structure.get_field("height", height_value);
  }

  m_videoarea.set_size_request(256, 256);

  /*
  pad->remove_probe(m_pad_probe_id);
  m_pad_probe_id = 0; // Clear probe id to indicate that it has been removed
  */

  return Gst::PAD_PROBE_OK;
}
