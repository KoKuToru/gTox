/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
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
#include "WidgetChatLabel.h"
#include <pangomm/renderer.h>
#include <cmath>
#include <iostream>

WidgetChatLabel::WidgetChatLabel()
    : Glib::ObjectBase("WidgetChatLabel"), m_clip(nullptr) {
  // add default name because
  // styling with "gtkmm_CustomObject_WudgetChatMessage" is not nice
  get_style_context()->add_class("chatmsg");
}

WidgetChatLabel::~WidgetChatLabel() {}

bool WidgetChatLabel::is_shape(PangoLayoutRun *run) {
  if (!run) {
    return false;
  }

  auto attr = run->item->analysis.extra_attrs;

  while (attr) {
    if (((PangoAttribute *)attr->data)->klass->type == PANGO_ATTR_SHAPE) {
      return true;
    }

    attr = attr->next;
  }

  return false;
}

bool WidgetChatLabel::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
  Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
  auto padding = stylecontext->get_padding();

  // Render the background
  stylecontext->render_background(
      cr, 0, 0, get_allocated_width(), get_allocated_height());

  // Translate rendering by the padding
  cr->save();
  cr->translate(padding.get_left(), padding.get_top());

  // draw selection
  if (m_clip) {
    Gdk::Cairo::add_region_to_path(cr, m_clip);
    cr->save();
    cr->clip();

    cr->set_source_rgba(0.5, 0.5, 0.5, 0.5);  // get color from settings ?
    cr->rectangle(0, 0, get_allocated_width(), get_allocated_height());
    cr->fill();

    cr->restore();
  }

  // draw the text
  m_text->set_width(
      Pango::SCALE *
      (get_allocated_width() - (padding.get_left() + padding.get_right())));
  stylecontext->render_layout(cr, 0, 0, m_text);

  // draw emojis
  auto iter = m_text->get_iter();
  if (iter.gobj() != nullptr)
    do {
      // auto run = iter.get_run(); //crashes
      // work around via C
      auto run_gobj = pango_layout_iter_get_run_readonly(iter.gobj());
      if (run_gobj == nullptr) {
        continue;
      }

      if (is_shape(run_gobj)) {
        auto run_extends = iter.get_run_logical_extents();

        // render emoji in future
        cr->rectangle(run_extends.get_x() / Pango::SCALE,
                      run_extends.get_y() / Pango::SCALE,
                      run_extends.get_width() / Pango::SCALE,
                      run_extends.get_height() / Pango::SCALE);

        cr->set_source_rgb(1, 0, 0);
        cr->fill();
      }
    } while (iter.next_run());

  cr->restore();
  return true;
}

void WidgetChatLabel::set_text(const Glib::ustring &text) {
  m_text = create_pango_layout(text);
  m_text->set_wrap(Pango::WRAP_WORD_CHAR);

  // add emojis
  /*auto attr_list = Pango::AttrList();
  //replace every 2nd with a 20px emoji
  for(size_t i = 1; i < text.bytes(); i+=2) {
      auto rect =
  Pango::Rectangle(0,-20*Pango::SCALE,20*Pango::SCALE,20*Pango::SCALE);
      auto attr = Pango::Attribute::create_attr_shape(rect, rect);
      attr.set_start_index(i);
      attr.set_end_index(i+1);
      attr_list.insert(attr);
  }
  m_text->set_attributes(attr_list);*/

  queue_resize();
}

void WidgetChatLabel::force_redraw() {
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win) {
    win->invalidate(false);
  }
}

Gtk::SizeRequestMode WidgetChatLabel::get_request_mode_vfunc() const {
  return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void WidgetChatLabel::get_preferred_width_vfunc(int &minimum_width,
                                                int &natural_width) const {
  Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
  auto padding = stylecontext->get_padding();

  m_text->set_width(-1);
  int height;
  m_text->get_pixel_size(natural_width, height);
  minimum_width = 0;

  minimum_width += padding.get_left() + padding.get_right();
  natural_width += padding.get_left() + padding.get_right();
}

void WidgetChatLabel::get_preferred_height_for_width_vfunc(int width,
                                                           int &minimum_height,
                                                           int &natural_height)
    const {
  Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
  auto padding = stylecontext->get_padding();

  m_text->set_width(Pango::SCALE *
                    (width - (padding.get_left() + padding.get_right())));
  m_text->get_pixel_size(width, minimum_height);
  natural_height = minimum_height;

  minimum_height += padding.get_top() + padding.get_bottom();
  natural_height += padding.get_top() + padding.get_bottom();
}

void WidgetChatLabel::get_preferred_height_vfunc(int &minimum_height,
                                                 int &natural_height) const {
  Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
  auto padding = stylecontext->get_padding();

  m_text->set_width(
      Pango::SCALE *
      (get_allocated_width() - (padding.get_left() + padding.get_right())));
  int width;
  m_text->get_pixel_size(width, natural_height);
  minimum_height = 0;

  minimum_height += padding.get_top() + padding.get_bottom();
  natural_height += padding.get_top() + padding.get_bottom();
}

Glib::ustring WidgetChatLabel::get_text() { return m_text->get_text(); }

void WidgetChatLabel::on_selection(int from_x, int from_y, int to_x, int to_y) {
  if (from_x == to_x && from_y == to_y) {
    if (m_clip) {
      m_clip.clear();
      force_redraw();
    }
    return;
  }

  Glib::RefPtr<Gtk::StyleContext> stylecontext = get_style_context();
  auto padding = stylecontext->get_padding();

  m_text->set_width(
      Pango::SCALE *
      (get_allocated_width() - (padding.get_left() + padding.get_right())));

  // fix coordiantes because of padding
  from_x -= padding.get_left();
  from_y -= padding.get_top();
  to_x -= padding.get_left();
  to_y -= padding.get_top();

  // get real size
  int w, h;
  m_text->get_pixel_size(w, h);

  // fix selection
  if (from_y < 0) {
    from_x = 0;
  }
  if (to_y > h) {
    to_x = w;
  }

  // get the selection
  int trailing;
  m_text->xy_to_index(from_x * Pango::SCALE,
                      from_y * Pango::SCALE,
                      selection_index_from,
                      trailing);
  if (trailing) {
    selection_index_from += 1;
  }
  m_text->xy_to_index(
      to_x * Pango::SCALE, to_y * Pango::SCALE, selection_index_to, trailing);
  if (trailing) {
    selection_index_to += 1;
  }

  // get the selected region
  gint reg[] = {std::min(selection_index_from, selection_index_to),
                std::max(selection_index_from, selection_index_to)};
  selection_index_from = reg[0];
  selection_index_to = reg[1];
  m_clip = Cairo::RefPtr<Cairo::Region>(
      new Cairo::Region(gdk_pango_layout_get_clip_region(
          m_text->gobj(), 0, 0, reg, 1)));  // cpp-version ?
  force_redraw();
}

Glib::ustring WidgetChatLabel::get_selection() {
  if (!m_clip) {
    return Glib::ustring();
  }
  // todo append date/time
  return std::string(m_text->get_text())
      .substr(selection_index_from, selection_index_to - selection_index_from);
}
