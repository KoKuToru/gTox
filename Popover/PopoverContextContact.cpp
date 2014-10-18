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

#include "PopoverContextContact.h"
#include "../Generated/icon.h"
#include "../Dialog/DialogContact.h"
#include <glibmm/i18n.h>

PopoverContextContact::PopoverContextContact(Tox::FriendNr friendNr): friendNr(friendNr){
    m_listbox.add(create_item(ICON::load_icon(ICON::remove), _("Remove")));
    m_listbox.show_all();
    add(m_listbox);

    //signal handling
    m_listbox.signal_row_activated().connect([this](Gtk::ListBoxRow* row) {
       switch (row->get_index()) {
           case 0:
               DialogContact::instance().delete_friend(this->friendNr);
               break;
       }
       set_visible(false);
    });
}

Gtk::ListBoxRow& PopoverContextContact::create_item(Glib::RefPtr<Gdk::Pixbuf> icon, Glib::ustring text) {
    auto row   = Gtk::manage(new Gtk::ListBoxRow());
    auto hbox  = Gtk::manage(new Gtk::HBox());
    auto label = Gtk::manage(new Gtk::Label(text));
    auto img = Gtk::manage(new Gtk::Image(icon));
    hbox->set_homogeneous(false);
    img->set_valign(Gtk::Align::ALIGN_START);
    label->set_valign(Gtk::Align::ALIGN_START);
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
    row->set_name("PopoverContextContactListItem");
    return *row;
}

PopoverContextContact::~PopoverContextContact() {
}

