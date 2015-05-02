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
      m_path(get_avatar_path(observable, nr)) {
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
      m_path(get_avatar_path(observable, nr)) {
    //load image
    load();
    show();
}

WidgetAvatar::WidgetAvatar(BaseObjectType* cobject, gToxBuilder builder,
                           const Glib::ustring& path) :
    Gtk::Image(cobject),
    gToxObserver(nullptr),
    m_builder(builder),
    m_path(path) {
    //load image
    load();
    show();
}

WidgetAvatar::~WidgetAvatar() {
}

void WidgetAvatar::observer_handle(const ToxEvent& ev) {
    //check if right type..
    if (ev.type() != typeid(EventUpdateAvatar)) {
        return;
    }
    auto data = ev.get<EventUpdateAvatar>();
    //check if we want it..
    if (data.path != m_path) {
        return;
    }
    //load image
    load();
}

void WidgetAvatar::load(bool force_reload) {
    int widget_w;
    int widget_h;
    get_size_request(widget_w, widget_h);

    if (widget_w <= 0 || widget_h <= 0) {
        return;
    }

    //load avatar from disk !
    auto pix = get_avatar(m_path, force_reload);

    set(pix);
}

void WidgetAvatar::set_size_request(int width, int height) {
    Gtk::Widget::set_size_request(width, height);
    load();
}

Glib::RefPtr<Gdk::Pixbuf> WidgetAvatar::get_avatar(Glib::ustring path, bool force_reload, bool use_cache) {
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
    if (use_cache) {
        auto pix_iter = pix_cache.find(path);
        if (pix_iter == pix_cache.end()) {
            pix_cache.insert({path, pix});
        } else {
            pix_iter->second = pix;
        }
    }

    return pix;
}

void WidgetAvatar::set_avatar(gToxObservable* observable, Glib::ustring path, Glib::RefPtr<Gdk::Pixbuf> pix) {
    unlink(path.c_str());
    if (pix) {
        pix->save(path, "png");
    }
    get_avatar(path, true);
    observable->observer_notify(
                ToxEvent(EventUpdateAvatar{
                             path
                         }));
}

Glib::ustring WidgetAvatar::get_avatar_path(gToxObservable* observable, Toxmm::FriendNr nr) {
    //load avatar from disk !
    auto addr = (nr != ~0u)?observable->tox().get_address(nr):observable->tox().get_address();
    auto avatar_path = Glib::build_filename(
                           Glib::get_user_config_dir(),
                           "tox", "avatars",
                           Toxmm::to_hex(addr.data(), TOX_PUBLIC_KEY_SIZE) + ".png");
    return avatar_path;
}

bool WidgetAvatar::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    Glib::RefPtr<Gtk::StyleContext> style = get_style_context();

    int w = get_allocated_width();
    int h = get_allocated_height();

    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, w * get_scale_factor(), h * get_scale_factor());
    auto tmp_cr = Cairo::Context::create(surface);
    tmp_cr->scale(get_scale_factor(), get_scale_factor());

    // Render image
    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        tmp_cr->save();
        double pw = pix->get_width();
        double ph = pix->get_height();
        tmp_cr->scale(w/pw, h/ph);
        Gdk::Cairo::set_source_pixbuf(tmp_cr, pix);
        tmp_cr->paint();
        tmp_cr->restore();
    }

    // Render the background
    tmp_cr->set_operator(Cairo::OPERATOR_DEST_ATOP);
    style->render_background(tmp_cr, 0, 0, w, h);

    // Change to default operator
    tmp_cr->set_operator(Cairo::OPERATOR_OVER);

    // Render frame
    style->render_frame(tmp_cr, 0, 0, w, h);

    // Render to the right surface
    cr->scale(1.0/get_scale_factor(), 1.0/get_scale_factor());
    cr->set_source(surface, 0, 0);
    cr->paint();

    return false;
}

Gtk::SizeRequestMode WidgetAvatar::get_request_mode_vfunc() const {
    return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void WidgetAvatar::get_preferred_width_vfunc(int& minimum_width,
                                             int& natural_width) const {
    Glib::RefPtr<Gtk::StyleContext> style = get_style_context();
    auto padding = style->get_padding();

    minimum_width = 0;
    natural_width = 0;

    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        int ignore = 0;
        get_size_request(minimum_width, ignore);
        minimum_width = std::max(minimum_width, 0);
        natural_width = minimum_width;

    }

    minimum_width += padding.get_left() + padding.get_right();
    natural_width += padding.get_left() + padding.get_right();
}

void WidgetAvatar::get_preferred_height_vfunc(int& minimum_height,
                                              int& natural_height) const {
    Glib::RefPtr<Gtk::StyleContext> style = get_style_context();
    auto padding = style->get_padding();

    minimum_height = 0;
    natural_height = 0;

    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        int ignore = 0;
        get_size_request(ignore, minimum_height);
        minimum_height = std::max(minimum_height, 0);
        natural_height = minimum_height;

    }

    minimum_height += padding.get_top() + padding.get_bottom();
    natural_height += padding.get_top() + padding.get_bottom();
}
