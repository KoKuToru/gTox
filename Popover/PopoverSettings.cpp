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
#include "PopoverSettings.h"
#include "Generated/icon.h"
#include "Dialog/DialogContact.h"

PopoverSettings::PopoverSettings(const Widget& relative_to): Gtk::Popover(relative_to) {
    auto grid = Gtk::manage(new Gtk::Grid());
    auto label1 = new Gtk::Label("Username");
    auto label2 = new Gtk::Label("Message");
    //auto btn_about = new Gtk::Button("About");

    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);
    grid->attach(*label1, 0, 0, 1, 1);
    grid->attach(*label2, 0, 1, 1, 1);
    grid->attach(m_name, 1, 0, 1, 1);
    grid->attach(m_msg , 1, 1, 1, 1);
    //grid->attach(*btn_about , 0, 2, 2, 1);
    grid->show_all();
    add(*grid);

    m_name.set_size_request(300);
    m_msg.set_size_request(300);

    grid->set_margin_top(5);
    grid->set_margin_bottom(5);
    grid->set_margin_left(5);
    grid->set_margin_right(5);

    label1->set_halign(Gtk::ALIGN_START);
    label2->set_halign(Gtk::ALIGN_START);
    label1->set_valign(Gtk::ALIGN_START);
    label2->set_valign(Gtk::ALIGN_START);

    m_name.signal_changed().connect([this]() {
        DialogContact::instance().change_name(m_name.get_text(), m_msg.get_text());
    });

    m_msg.signal_changed().connect([this]() {
        DialogContact::instance().change_name(m_name.get_text(), m_msg.get_text());
    });
}

PopoverSettings::~PopoverSettings() {

}

void PopoverSettings::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    //update data
    if (!visible) {
        return;
    }

    //if (Tox::instance().is_connected()) {
    //    m_name.set_sensitive(true);
    //    m_msg.set_sensitive(true);
        m_name.set_text(Tox::instance().get_name_or_address());
        m_msg.set_text(Tox::instance().get_status_message());
    /*} else {
        m_name.set_sensitive(false);
        m_msg.set_sensitive(false);
        m_name.set_text(Tox::instance().get_name_or_address());
        m_msg.set_text(Tox::instance().get_status_message());
    }*/
}
