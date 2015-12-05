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
#include "imagescaled.h"
#include <iostream>
#include <mutex>

using namespace widget;

imagescaled::imagescaled():
    Glib::ObjectBase(typeid(imagescaled)) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    show();
}

imagescaled::imagescaled(BaseObjectType* cobject,
                         utils::builder)
    : Gtk::Image(cobject) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    show();
}

imagescaled::~imagescaled() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

void imagescaled::calculate_size(int w, int h,
                                 double& out_pw, double& out_ph,
                                 double& out_scale) const {
    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (!pix) {
        out_scale = 1;
        out_pw = 1;
        out_ph = 1;
        return;
    }
    w = std::max(1, w);
    h = std::max(1, h);
    out_pw = pix->get_width();
    out_ph = pix->get_height();
    double scale_w = w / out_pw;
    double scale_h = h / out_ph;
    out_scale = std::max(scale_w, scale_h);
    out_pw *= out_scale;
    out_ph *= out_scale;
}

bool imagescaled::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
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
        double pw = 1;
        double ph = 1;
        double scale = 1;
        calculate_size(w, h, pw, ph, scale);
        tmp_cr->scale(scale, scale);
        Gdk::Cairo::set_source_pixbuf(tmp_cr,
                                      pix,
                                      (w - pw) / 2,
                                      (h - ph) / 2);
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

Gtk::SizeRequestMode imagescaled::get_request_mode_vfunc() const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void imagescaled::get_preferred_width_vfunc(int& minimum_width,
                                            int& natural_width) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});

    minimum_width = 1;
    natural_width = 1;

    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        natural_width = pix->get_width();
    }
}
void imagescaled::get_preferred_height_vfunc(int& minimum_height,
                                             int& natural_height) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});

    int minimum_width = 1;
    int natural_width = 1;
    get_preferred_width_vfunc(minimum_width,
                              natural_width);
    get_preferred_height_for_width_vfunc(minimum_width,
                                         minimum_height,
                                         natural_height);
}

void imagescaled::get_preferred_height_for_width_vfunc(
        int width,
        int& minimum_height,
        int& natural_height) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});

    auto pix = property_pixbuf().get_value();
    if (pix) {
        double pw = 1;
        double ph = 1;
        double scale = 1;
        calculate_size(width, 0, pw, ph, scale);
        minimum_height = std::ceil(ph);
        natural_height = std::ceil(ph);
    }
}

void imagescaled::get_preferred_width_for_height_vfunc(
        int height,
        int& minimum_width,
        int& natural_width) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});

    auto pix = property_pixbuf().get_value();
    if (pix) {
        double pw = 1;
        double ph = 1;
        double scale = 1;
        calculate_size(0, height, pw, ph, scale);
        minimum_width = std::ceil(pw);
        natural_width = std::ceil(pw);
    }
}
