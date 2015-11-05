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

imagescaled::imagescaled() {
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
        double pw = pix->get_width();
        double ph = pix->get_height();
        double scale_w = w/pw;
        double scale_h = h/ph;
        double scale = std::max(scale_w, scale_h);
        pw *= scale;
        ph *= scale;
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
    Glib::RefPtr<Gtk::StyleContext> style = get_style_context();
    auto padding = style->get_padding();

    minimum_width = 0;
    natural_width = 0;

    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        natural_width = pix->get_width();
    }

    minimum_width += padding.get_left() + padding.get_right();
    natural_width += padding.get_left() + padding.get_right();
}
void imagescaled::get_preferred_height_vfunc(int& minimum_height,
                                             int& natural_height) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    Glib::RefPtr<Gtk::StyleContext> style = get_style_context();
    auto padding = style->get_padding();

    minimum_height = 0;
    natural_height = 0;

    Glib::RefPtr<Gdk::Pixbuf> pix = property_pixbuf();
    if (pix) {
        natural_height = pix->get_height();
    }

    minimum_height += padding.get_top() + padding.get_bottom();
    natural_height += padding.get_top() + padding.get_bottom();
}

void imagescaled::get_preferred_height_for_width_vfunc(
        int width,
        int& minimum_height,
        int& natural_height) const {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
    auto padding = stylecontext->get_padding();

    auto pix = property_pixbuf().get_value();
    if (pix) {
        double pw = pix->get_width();
        double ph = pix->get_height();
        double nh = ph * (width - padding.get_left() - padding.get_right()) / pw;
        minimum_height = nh;
        natural_height = nh;
    }
    minimum_height += padding.get_top() + padding.get_bottom();
    natural_height += padding.get_top() + padding.get_bottom();
}
