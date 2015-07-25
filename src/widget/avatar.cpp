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
#include "avatar.h"
#include <iostream>
#include <mutex>

using namespace widget;

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf>> avatar::image::property_pixbuf() {
    return m_property_pixbuf.get_proxy();
}

avatar::image::image(toxmm2::contactAddrPublic addr):
    Glib::ObjectBase(typeid(avatar::image)),
    m_property_pixbuf(*this,
                      "image-pixbuf") {

    auto path = Glib::build_filename(
                    Glib::get_user_config_dir(),
                    "tox", "avatars",
                    std::string(addr) + ".png");
    m_file = Gio::File::create_for_path(path);
    m_monitor = m_file->monitor_file();

    m_monitor->signal_changed().connect(sigc::track_obj([this](const Glib::RefPtr<Gio::File>&,
                                        const Glib::RefPtr<Gio::File>&,
                                        Gio::FileMonitorEvent event_type) {
        switch (event_type) {
            case Gio::FILE_MONITOR_EVENT_CREATED:
            case Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
            case Gio::FILE_MONITOR_EVENT_DELETED:
                load();
                break;
            default:
                //ignore
                break;
        }
    }, *this));

    load();
}

void avatar::image::load() {
    property_pixbuf() =
            Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg");
    if (!m_file->query_exists()) {
        return;
    }
    //load async
    utils::dispatcher::ref dispatcher(m_dispatcher); //take a reference
    auto file = m_file;
    auto version = ++m_version;
    auto self = this;
    Glib::Thread::create([dispatcher, file, self, version](){
        static std::mutex mutex;
        std::lock_guard<std::mutex> lg(mutex); //limit parallel load

        //todo try to read w/h first. And block too big dimensions !
        Glib::RefPtr<Gdk::Pixbuf> pix;
        try {
            pix = Gdk::Pixbuf::create_from_file(file->get_path());
        } catch (...) {
            //couldn't load it
            return;
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

        dispatcher.emit([pix, self, version]() {
            if (version == self->m_version) {
                self->property_pixbuf() = pix;
            }
        });
    }, false);
}

avatar::avatar(BaseObjectType* cobject,
               utils::builder,
               toxmm2::contactAddrPublic addr)
    : Gtk::Image(cobject) {
    //load image
    static std::map<toxmm2::contactAddrPublic, std::shared_ptr<image>> m_image;
    auto iter = m_image.find(addr);
    if (iter == m_image.end()) {
        //create new
        iter = m_image.insert({addr, std::make_shared<image>(addr)}).first;
    }

    m_binding = Glib::Binding::bind_property(
                    iter->second->property_pixbuf(),
                    property_pixbuf(),
                    Glib::BINDING_DEFAULT |
                    Glib::BINDING_SYNC_CREATE |
                    Glib::BINDING_BIDIRECTIONAL);

    show();
}

avatar::~avatar() {
}

bool avatar::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
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

Gtk::SizeRequestMode avatar::get_request_mode_vfunc() const {
    return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void avatar::get_preferred_width_vfunc(int& minimum_width,
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

void avatar::get_preferred_height_vfunc(int& minimum_height,
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
