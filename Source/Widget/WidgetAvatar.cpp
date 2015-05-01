/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "WidgetAvatar.h"
#include "Helper/gToxFileRecv.h"
#include <iostream>

WidgetAvatar::WidgetAvatar(gToxObservable* observable,
                           Toxmm::FriendNr nr)
    : gToxObserver(observable),
      m_builder(Glib::RefPtr<Gtk::Builder>()),
      m_nr(nr) {
    //load image
    load();
    show();
}

WidgetAvatar::WidgetAvatar(BaseObjectType* cobject, gToxBuilder builder,
                           gToxObservable* observable,
                           Toxmm::FriendNr nr)
    : Gtk::Image(cobject),
      gToxObserver(observable),
      m_builder(builder),
      m_nr(nr) {
    //load image
    load();
    show();
}

WidgetAvatar::~WidgetAvatar() {
}

void WidgetAvatar::observer_handle(const ToxEvent& ev) {
    //check if right type..
    if (ev.type() != typeid(gToxFileRecv::EventFileProgress)) {
        return;
    }
    auto data = ev.get<gToxFileRecv::EventFileProgress>();
    //check if we want it..
    if (data.nr != m_nr || data.file_kind != TOX_FILE_KIND_AVATAR) {
        return;
    }
    //check for progress..
    if (data.file_position != data.file_size) {
        return;
    }
    //load image
    load(true);
}

void WidgetAvatar::load(bool force_reload) {
    int widget_w;
    int widget_h;
    get_size_request(widget_w, widget_h);

    if (widget_w <= 0 || widget_h <= 0) {
        return;
    }

    //load avatar from disk !
    auto addr = (m_nr != ~0u)?tox().get_address(m_nr):tox().get_address();
    auto avatar_path = Glib::build_filename(Glib::get_user_config_dir(),
                                            "tox", "avatars",
                                            Toxmm::to_hex(addr.data(), TOX_PUBLIC_KEY_SIZE) + ".png");

    auto pix = get_avatar(avatar_path, force_reload);

    //calcualte size
    int w = pix->get_width();
    int h = pix->get_height();
    if (w < h) {
        h = h * widget_w / w;
        w = widget_w;
    } else {
        w = w * widget_h / h;
        h = widget_h;
    }

    //scale
    pix = pix->scale_simple(w, h ,Gdk::INTERP_BILINEAR);

    set(pix);
}

void WidgetAvatar::set_size_request(int width, int height) {
    Gtk::Widget::set_size_request(width, height);
    load();
}

Glib::RefPtr<Gdk::Pixbuf> WidgetAvatar::get_avatar(Glib::ustring path, bool force_reload) {
    static auto pix_default = Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg");
    static std::map<std::string, Glib::RefPtr<Gdk::Pixbuf>> pix_cache;

    if (!force_reload) {
        auto pix_iter = pix_cache.find(path);
        if (pix_iter != pix_cache.end()) {
            return pix_iter->second;
        }
    }

    if (!Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR)) {
        return pix_default;
    }

    //todo try to read w/h first. And block too big dimensions !
    Glib::RefPtr<Gdk::Pixbuf> pix;
    try {
        pix = Gdk::Pixbuf::create_from_file(path);
    } catch (...) {
        std::clog << "Couldn't load " << path << std::endl;
        return pix_default;
    }

    //scale to 128x128
    int w = pix->get_width();
    int h = pix->get_height();
    if (w < h) {
        h = h * 128 / w;
        w = 128;
    } else {
        w = w * 128 / h;
        h = 128;
    }
    pix = pix->scale_simple(w, h ,Gdk::INTERP_BILINEAR);

    //crop
    int src_x = pix->get_width()/2 - 128/2;
    int src_y = pix->get_height()/2 - 128/2;
    pix = Gdk::Pixbuf::create_subpixbuf(pix, src_x, src_y, 128, 128);

    //add to cache
    pix_cache.insert({path, pix});

    return pix;
}
