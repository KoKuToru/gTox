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
#include "DialogSettings.h"
#include "DialogContact.h"

DialogSettings::DialogSettings(gToxObservable* observable)
    : gToxObserver(observable),
      m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_settings.ui")) {

    set_titlebar(m_headerbar_box);
    m_headerbar_box.property_hexpand() = true;
    m_headerbar_box.show();

    add(m_body_box);
    m_body_box.property_expand() = true;
    m_body_box.show();

    m_builder.get_widget("settings_headerbar", m_headerbar);
    m_builder.get_widget("settings_widget", m_body);

    m_size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
    m_size_group->add_widget(*m_body);
    m_size_group->add_widget(*m_headerbar);

    m_builder.get_widget<Gtk::Button>("close_btn")->signal_clicked().connect([this](){
        hide();
    });
}

DialogSettings::~DialogSettings() {
    if (!m_in_window) {
        int x,y,w,h;
        observer_notify(ToxEvent(DialogContact::EventDetachWidget{
                                     false,
                                     m_headerbar,
                                     m_body,
                                     x,
                                     y,
                                     w,
                                     h
                                 }));
    }
}

void DialogSettings::show() {
    if (!m_in_window) {
        int x,y,w,h;
        observer_notify(ToxEvent(DialogContact::EventDetachWidget{
                                     true,
                                     m_headerbar,
                                     m_body,
                                     x,
                                     y,
                                     w,
                                     h
                                 }));
        move(x, y);
        resize(w, h);
        m_headerbar_box.add(*m_headerbar);
        m_body_box.add(*m_body);
        m_in_window = true;
    }
    Gtk::Window::show();
}

void DialogSettings::hide() {
    if (m_in_window) {
        m_headerbar_box.remove(*m_headerbar);
        m_body_box.remove(*m_body);
        m_in_window = false;

        observer_notify(ToxEvent(DialogContact::EventAttachWidget{
                                     m_headerbar,
                                     m_body
                                 }));
    } else {
        int x,y,w,h;
        observer_notify(ToxEvent(DialogContact::EventDetachWidget{
                                     false,
                                     m_headerbar,
                                     m_body,
                                     x,
                                     y,
                                     w,
                                     h
                                 }));
    }
    Gtk::Window::hide();
}

void DialogSettings::present() {
    if (m_in_window) {
        Gtk::Window::present();
    } else {
        observer_notify(ToxEvent(DialogContact::EventPresentWidget{
                                     m_headerbar,
                                     m_body
                                 }));
    }
}

bool DialogSettings::is_visible() {
    if (m_in_window) {
        return property_is_active();
    } else {
        return m_body_box.get_mapped();
    }
}
