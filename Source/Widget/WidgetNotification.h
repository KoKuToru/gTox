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
#ifndef WIDGETNOTIFICATION_H
#define WIDGETNOTIFICATION_H

#include <gtkmm.h>
#include "Dialog/DialogContact.h"
#include <libnotifymm.h>
#include "Helper/gToxObserver.h"
#include "Helper/gToxBuilder.h"

class WidgetNotification : public Gtk::ListBoxRow, public gToxObserver {
  private:
    DialogContact::EventAddNotification m_event;
    std::shared_ptr<Notify::Notification> m_notify;

    Gtk::Label* m_title;
    Gtk::Label* m_message;
    Gtk::Image* m_image;
    Gtk::Image* m_icon;
    Gtk::Box* m_action_bar;

  public:
    WidgetNotification(BaseObjectType* cobject, gToxBuilder builder,
                       gToxObservable* observable,
                       DialogContact::EventAddNotification event);
    static gToxBuilderRef<WidgetNotification> create(gToxObservable* observable, DialogContact::EventAddNotification event);

    ~WidgetNotification();

    void activated();

  protected:
    void set_event(DialogContact::EventAddNotification event);
};

#endif
