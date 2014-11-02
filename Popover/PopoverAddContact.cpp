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
#include "PopoverAddContact.h"
#include "Generated/icon.h"
#include "Dialog/DialogContact.h"
#include <glibmm/i18n.h>

#include <iostream>

PopoverAddContact::PopoverAddContact(const Gtk::Widget& relative_to)
    : Gtk::Popover(relative_to) {
    auto grid = Gtk::manage(new Gtk::Grid());
    auto label1 = Gtk::manage(new Gtk::Label(_("Tox ID")));
    auto label2 = Gtk::manage(new Gtk::Label(_("Message")));
    auto btn_add = Gtk::manage(new Gtk::Button(_("Add")));
    auto box_msg = Gtk::manage(new Gtk::Frame());
    box_msg->add(m_msg);
    box_msg->get_style_context()->add_class("entry");
    box_msg->get_style_context()->remove_class("frame");
    m_addr.set_size_request(300);
    m_msg.set_size_request(300, 200);
    m_msg.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    grid->set_column_homogeneous(false);
    grid->set_row_spacing(5);
    grid->set_column_spacing(10);
    grid->attach(*label1, 0, 0, 1, 1);
    grid->attach(*label2, 0, 1, 1, 1);
    grid->attach(m_addr, 1, 0, 1, 1);
    grid->attach(*box_msg, 1, 1, 1, 1);
    grid->attach(*btn_add, 1, 2, 1, 1);
    grid->show_all();
    add(*grid);

    btn_add->set_hexpand(false);
    btn_add->set_halign(Gtk::ALIGN_END);

    label1->set_halign(Gtk::ALIGN_START);
    label2->set_halign(Gtk::ALIGN_START);
    label1->set_valign(Gtk::ALIGN_BASELINE);
    label2->set_valign(Gtk::ALIGN_START);

    btn_add->signal_clicked().connect([this]() {
        std::cout << m_addr.get_text().length() << std::endl;
        if (m_addr.get_text().length() != 76) {
            // TODO error message
            std::cout << "ERROR2" << std::endl;
            return;
        }

        if (m_msg.get_buffer()->get_text(true).size() == 0) {
            m_msg.get_buffer()->set_text("I am using gTox. Add me");
        }

        try {
            Tox::FriendAddr adr;
            auto adr_c = Tox::from_hex(m_addr.get_text());
            std::copy(adr_c.begin(), adr_c.end(), adr.begin());
            DialogContact::instance().add_contact(Tox::instance().add_friend(
                adr, m_msg.get_buffer()->get_text()));

            m_addr.set_text("");
            m_msg.get_buffer()->set_text("");
            set_visible(false);
        }
        catch (...) {
            // TODO: Error handling
            std::cout << "ERROR" << std::endl;
        }
    });
}

PopoverAddContact::~PopoverAddContact() {
}
