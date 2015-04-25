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
#include "WidgetProfile.h"
#include <glibmm/i18n.h>
#include "Tox/Toxmm.h"
#include "Dialog/DialogContact.h"

WidgetProfile::WidgetProfile()
    : Glib::ObjectBase("WidgetProfile") {
    update();

    std::string hex = Toxmm::to_hex(Toxmm::instance().get_address().data(),
                                    Toxmm::instance().get_address().size());
    for (int i = 4; i > 0; --i) {
        hex.insert(2 * i * TOX_PUBLIC_KEY_SIZE / 4, 1, '\n');
    }

    property_valign() = Gtk::ALIGN_CENTER;
    property_halign() = Gtk::ALIGN_CENTER;

    auto grid = Gtk::manage(new Gtk::Grid);

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);

    m_avatar.set_image(
        *Gtk::manage(new Gtk::Image(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg"))));
    grid->attach(m_avatar, 0, 0, 1, 2);

    grid->attach(
        *Gtk::manage(new Gtk::Label(_("USERNAME"), 1, 0.5)), 1, 0, 1, 1);
    grid->attach(m_username, 2, 0, 1, 1);

    grid->attach(
        *Gtk::manage(new Gtk::Label(_("MESSAGE"), 1, 0.5)), 1, 1, 1, 1);
    grid->attach(m_status, 2, 1, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label(_("TOX_ID"), 1, 0)), 1, 2, 1, 1);
    auto tox_id = Gtk::manage(new Gtk::Label("", 0, 0.5));
    tox_id->set_selectable(true);
    tox_id->set_markup("<span font_desc=\"mono\">" + hex + "</span>");
    grid->attach(*tox_id, 2, 2, 1, 2);

    auto cpy_btn = Gtk::manage(new Gtk::Button());
    cpy_btn->set_image_from_icon_name("clipboard-symbolic");
    cpy_btn->set_halign(Gtk::ALIGN_END);
    cpy_btn->set_valign(Gtk::ALIGN_END);
    grid->attach(*cpy_btn, 1, 3, 1, 1);

    pack_start(*grid, false, false);

    show_all();

    m_username.signal_focus_out_event().connect_notify([this](GdkEventFocus*) {
        ToxEventCallback::notify(ToxEvent(DialogContact::EventSetName{
                                              m_username.get_text(),
                                              m_status.get_text()
                                          }));
    });

    m_status.signal_focus_out_event().connect_notify([this](GdkEventFocus*) {
        ToxEventCallback::notify(ToxEvent(DialogContact::EventSetName{
                                              m_username.get_text(),
                                              m_status.get_text()
                                          }));
    });

    cpy_btn->signal_clicked().connect([this]() {
        Gtk::Clipboard::get()->set_text(
            Toxmm::to_hex(Toxmm::instance().get_address().data(),
                        Toxmm::instance().get_address().size()));
    });
}

WidgetProfile::~WidgetProfile() {
}

void WidgetProfile::update() {
    m_username.set_text(Toxmm::instance().get_name_or_address());
    m_status.set_text(Toxmm::instance().get_status_message());
}
