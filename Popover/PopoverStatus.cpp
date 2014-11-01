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
#include "PopoverStatus.h"
#include "Generated/icon.h"
#include "Dialog/DialogContact.h"
#include <glibmm/i18n.h>

PopoverStatus::PopoverStatus(const Widget& relative_to): Gtk::Popover(relative_to) {
    //add_label("Settings");

    m_listbox.add(create_item(ICON::load_icon(ICON::status_online), _("Online")));
    m_listbox.add(create_item(ICON::load_icon(ICON::status_busy), _("Busy")));
    m_listbox.add(create_item(ICON::load_icon(ICON::status_away), _("Away")));
    m_listbox.add(create_item(ICON::load_icon(ICON::status_offline), _("Exit")));
    m_listbox.show_all();
    add(m_listbox);

    //signal handling
    m_listbox.signal_row_activated().connect([this](Gtk::ListBoxRow* row) {
       switch (row->get_index()) {
           case 0:
               DialogContact::instance().set_status(Tox::NONE);
               break;
           case 1:
               DialogContact::instance().set_status(Tox::BUSY);
               break;
           case 2:
               DialogContact::instance().set_status(Tox::AWAY);
               break;
           case 3:
               DialogContact::instance().exit();
               break;
       }
       set_visible(false);
    });
}

PopoverStatus::~PopoverStatus() {

}

Gtk::ListBoxRow& PopoverStatus::create_item(Glib::RefPtr<Gdk::Pixbuf> icon, Glib::ustring text) {
    auto row   = Gtk::manage(new Gtk::ListBoxRow());
    auto hbox  = Gtk::manage(new Gtk::HBox());
    auto label = Gtk::manage(new Gtk::Label(text));
    auto img = Gtk::manage(new Gtk::Image(icon));
    hbox->set_homogeneous(false);
    img->set_valign(Gtk::Align::ALIGN_CENTER);
    label->set_valign(Gtk::Align::ALIGN_CENTER);
    img->set_margin_top(5);
    img->set_margin_bottom(5);
    img->set_margin_left(5);
    img->set_margin_right(5);
    label->set_margin_top(5);
    label->set_margin_bottom(5);
    label->set_margin_left(5);
    label->set_margin_right(5);
    hbox->pack_start(*img, false, false);
    hbox->pack_start(*label, false, true);
    row->add(*hbox);
    row->set_name("PopoverStatusListItem");
    return *row;
}

void PopoverStatus::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    //update selection
    if (!visible) {
        return;
    }

    int select = 3;
    switch(Tox::instance().get_status()) {
        case Tox::NONE:
            select = 0;
            break;
        case Tox::BUSY:
            select = 1;
            break;
        case Tox::AWAY:
            select = 2;
            break;
        default:
            break;
    }
    m_listbox.select_row(*m_listbox.get_row_at_index(select));
}
