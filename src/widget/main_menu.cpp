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
#include "main_menu.h"
#include "utils/builder.h"
#include <glibmm/i18n.h>
#include "dialog/error.h"
#include "tox/contact/manager.h"
#include "tox/exception.h"
#include "dialog/settings.h"
#include "dialog/main.h"

using namespace widget;

main_menu::main_menu(dialog::main& main): m_main(main) {

    utils::builder builder = Gtk::Builder::create_from_resource("/org/gtox/ui/popover_settings.ui");

    builder.get_widget("profile_username_entry", m_profile.username);
    builder.get_widget("profile_statusmessage_entry", m_profile.status);
    m_profile.avatar = builder.get_widget_derived<avatar>("profile_avatar", m_main.tox()->property_addr_public());

    builder.get_widget("add_contact_tox_id", m_add_contact.tox_id);
    builder.get_widget("add_contact_message", m_add_contact.message);

    builder.get_widget("popover_stack", m_stack);
    builder.get_widget("profile_open_settings", m_settings_btn);

    add(*m_stack);

    /* Tox-Id display */
    std::string hexfull = m_main.tox()->property_addr().get_value();
    std::string hex = m_main.tox()->property_addr().get_value();
    for (int i = 4; i > 0; --i) {
        hex.insert(2 * i * TOX_PUBLIC_KEY_SIZE / 4, 1, '\n');
    }

    builder.get_widget<Gtk::Label>("profile_tox_hex_id")
            ->set_markup("<span font_desc=\"mono\">" + hex + "</span>");

    builder.get_widget<Gtk::Button>("profile_copy_tox_id")
            ->signal_clicked().connect([this, hexfull]() {
        Gtk::Clipboard::get()->set_text(hexfull);
    });

    /* Open settings */
    m_settings_btn->signal_clicked().connect(sigc::track_obj([this]() {
        hide();
        if (m_settings) {
            m_settings->activated();
        } else {
            m_settings = Glib::RefPtr<dialog::settings>(new dialog::settings(m_main));
        }
    }, *this));

    /* change avatar logic */
    /*
    builder.get_widget<Gtk::Button>("profile_avatar_btn")
            ->signal_clicked().connect([this, observable](){
        Gtk::FileChooserDialog dialog(_("PROFILE_AVATAR_SELECT_TITLE"), Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.add_button (Gtk::Stock::OPEN,
                               Gtk::RESPONSE_ACCEPT);
        dialog.add_button (Gtk::Stock::CANCEL,
                           Gtk::RESPONSE_CANCEL);

        Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
        filter->add_pixbuf_formats();
        filter->set_name(_("IMAGE"));
        dialog.add_filter (filter);

        Gtk::Image image;
        image.set(WidgetAvatar::get_avatar(""));
        dialog.set_preview_widget(image);

        dialog.signal_update_preview().connect([&dialog, &image](){
            auto uri = dialog.get_preview_uri();
            if (uri.empty()) {
                image.hide();
                return;
            }
            if (Glib::str_has_prefix(uri, "file://")) {
                image.set(WidgetAvatar::get_avatar(uri.substr(7), true, false));
                image.show();
            }
        });

        const int response = dialog.run();
        if (response == Gtk::RESPONSE_ACCEPT) {
            WidgetAvatar::set_avatar(observable,
                                     WidgetAvatar::get_avatar_path(observable),
                                     image.get_pixbuf());
        }

        dialog.hide();
    });
    */
    /* Update name / status message logic */
    m_profile.username->signal_focus_out_event().connect_notify(sigc::hide([this]() {
        m_main.tox()->property_name() = m_profile.username->get_text();
    }));
    m_profile.status  ->signal_focus_out_event().connect_notify(sigc::hide([this]() {
        m_main.tox()->property_status_message() = m_profile.status->get_text();
    }));

    /* Add contact submit button handling */
    builder.get_widget<Gtk::Button>("add_contact_submit")
            ->signal_clicked().connect([this](){
        add_contact();
    });

    /* limit text input for tox-id */
    m_add_contact.tox_id->signal_changed().connect_notify([this](){
        Glib::ustring text;
        for (auto letter : m_add_contact.tox_id->get_text()) {
            if ((letter >= '0' && letter <= '9')
                || (letter >= 'a' && letter <= 'f')
                || (letter >= 'A' && letter <= 'Z')) {
                text.append(1, letter);
            }
        }
        m_add_contact.tox_id->set_text(text);
    }, true);
}

main_menu::~main_menu() {
}

void main_menu::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    // update data
    if (!visible) {
        return;
    }

    m_profile.username->set_text(m_main.tox()->property_name());
    m_profile.status->set_text(m_main.tox()->property_status_message());

    m_stack->set_visible_child("popover_profile", Gtk::STACK_TRANSITION_TYPE_NONE);
    m_profile.username->grab_focus();
}


void main_menu::add_contact() {
    Gtk::Window& parent = dynamic_cast<Gtk::Window&>(*this->get_toplevel());

    if (m_add_contact.tox_id->get_text().length() != TOX_ADDRESS_SIZE * 2) {
        dialog::error(parent, false,
                      _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE_TITLE"),
                      _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE")).run();
        return;
    }

    try {
        auto message = m_add_contact.message->get_buffer()->get_text();
        if (message.empty()) {
            message = " ";
        }
        m_main.tox()->contact_manager()->add_contact(toxmm2::contactAddr(m_add_contact.tox_id->get_text()), message);

        set_visible(false);
        m_add_contact.tox_id->set_text("");
        m_add_contact.message->get_buffer()->set_text("");
    } catch (toxmm2::exception &ex) {
        if (ex.type() != typeid(TOX_ERR_FRIEND_ADD)) {
            throw;
        }
        switch(ex.what_id()) {
            case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_OWN_KEY:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_TOO_LONG:
                dialog::error(parent, false,
                              _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI_TITLE"),
                              _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI")).run();
                break;
            default:
                throw;
        }
    }
}
