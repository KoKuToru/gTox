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
#ifndef WIDGETNETWORK_H
#define WIDGETNETWORK_H

#include <gtkmm.h>
class WidgetNetwork : public Gtk::VBox {
  private:
    Gtk::Switch m_ipv6;
    Gtk::Switch m_udp;
    Gtk::Switch m_proxy;
    Gtk::Entry m_proxy_host;
    Gtk::Entry m_proxy_port;

  public:
    WidgetNetwork();
    ~WidgetNetwork();
};

#endif
