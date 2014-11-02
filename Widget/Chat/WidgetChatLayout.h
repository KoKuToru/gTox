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
#ifndef WIDGETCHATLAYOUTE_H
#define WIDGETCHATLAYOUTE_H

#include <gtkmm.h>

class WidgetChatLayout : public Gtk::EventBox {
  private:
    Gtk::VBox m_vbox;
    int from_x;
    int from_y;

    void update_children(GdkEventMotion *event,
                         std::vector<Gtk::Widget *> children);
    Glib::ustring get_children_selection(std::vector<Gtk::Widget *> children);

  public:
    WidgetChatLayout();
    ~WidgetChatLayout();

    void set_spacing(int space);
    void pack_start(Gtk::Widget &w, bool a, bool b);

    std::vector<Gtk::Widget *> get_children();
    std::vector<const Gtk::Widget *> get_children() const;

  protected:
    virtual bool on_button_press_event(GdkEventButton *event);
    virtual bool on_button_release_event(GdkEventButton *event);
    virtual bool on_motion_notify_event(GdkEventMotion *event);
    virtual bool on_key_press_event(GdkEventKey *event);
};

#endif
