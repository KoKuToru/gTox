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
#ifndef WIDGETCHATMESSAGE_H
#define WIDGETCHATMESSAGE_H

#include <gtkmm.h>

class WidgetChatLabel : public Gtk::DrawingArea {
 private:
  Glib::RefPtr<Pango::Layout> m_text;
  Cairo::RefPtr<Cairo::Region> m_clip;

  int selection_index_from;
  int selection_index_to;

  void force_redraw();

 public:
  WidgetChatLabel();
  ~WidgetChatLabel();

  void set_text(const Glib::ustring &text);
  Glib::ustring get_text();

  void on_selection(int from_x, int from_y, int to_x, int to_y);
  Glib::ustring get_selection();

 protected:
  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

  virtual Gtk::SizeRequestMode get_request_mode_vfunc() const;
  virtual void get_preferred_width_vfunc(int &minimum_width,
                                         int &natural_width) const;
  virtual void get_preferred_height_for_width_vfunc(int width,
                                                    int &minimum_height,
                                                    int &natural_height) const;
  virtual void get_preferred_height_vfunc(int &minimum_height,
                                          int &natural_height) const;

  bool is_shape(PangoLayoutRun *run);
};

#endif
