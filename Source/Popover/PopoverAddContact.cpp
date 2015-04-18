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
#include "Dialog/DialogError.h"
#include <glibmm/i18n.h>

#include <iostream>

PopoverAddContact::PopoverAddContact(const Gtk::Widget& relative_to)
    : Gtk::Popover(relative_to) {
    auto grid = Gtk::manage(new Gtk::Grid());
    auto label1 = Gtk::manage(new Gtk::Label(_("TOX_ID")));
    auto label2 = Gtk::manage(new Gtk::Label(_("MESSAGE")));
    auto btn_add = Gtk::manage(new Gtk::Button(_("ADD")));
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
        if (m_addr.get_text().length() != TOX_ADDRESS_SIZE * 2) {
            DialogError(false,
                        _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE_TITLE"),
                        _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE")).run();
            return;
        }

        try {
            Toxmm::FriendAddr adr;
            auto adr_c = Toxmm::from_hex(m_addr.get_text());
            std::copy(adr_c.begin(), adr_c.end(), adr.begin());
            /*DialogContact::instance().add_contact(Toxmm::instance().add_friend(
                adr, m_msg.get_buffer()->get_text()));*/

            set_visible(false);
            m_addr.set_text("");
            m_msg.get_buffer()->set_text("");
        } catch (Toxmm::Exception &ex) {
            if (ex.type() != typeid(TOX_ERR_FRIEND_ADD)) {
                throw;
            }
            switch(ex.what_id()) {
                case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI")).run();
                    break;
                case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI")).run();
                    break;
                case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI")).run();
                    break;
                case TOX_ERR_FRIEND_ADD_OWN_KEY:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI")).run();
                    break;
                case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI")).run();
                    break;
                case TOX_ERR_FRIEND_ADD_TOO_LONG:
                    DialogError(false,
                                _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI_TITLE"),
                                _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI")).run();
                    break;
                default:
                    throw;
            }
        }
    });

    m_addr.signal_changed().connect_notify(
        [this]() {
            Glib::ustring text;
            for (auto letter : m_addr.get_text()) {
                if ((letter >= '0' && letter <= '9')
                    || (letter >= 'a' && letter <= 'f')
                    || (letter >= 'A' && letter <= 'Z')) {
                    text.append(1, letter);
                }
            }
            m_addr.set_text(text);
        },
        true);
}

PopoverAddContact::~PopoverAddContact() {
}

void PopoverAddContact::set_visible(bool v) {
    if (v) {
        if (m_msg.get_buffer()->get_text().size() == 0) {
            m_msg.get_buffer()->set_text("I am using gTox. Add me");
        }
    }
    Gtk::Popover::set_visible(v);
}
