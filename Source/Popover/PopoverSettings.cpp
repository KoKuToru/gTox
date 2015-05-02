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
#include "Dialog/DialogContact.h"
#include "Dialog/Debug/DialogCss.h"
#include "Dialog/DialogError.h"
#include <glibmm/i18n.h>

PopoverSettings::PopoverSettings(gToxObservable* observable,
                                 const Gtk::Widget& relative_to)
    : Gtk::Popover(relative_to),
      gToxObserver(observable),
      m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/popover_settings.ui")) {

    m_builder.get_widget("profile_username_entry", m_profile.username);
    m_builder.get_widget("profile_statusmessage_entry", m_profile.status);
    m_profile.avatar = m_builder.get_widget_derived<WidgetAvatar>("profile_avatar", observable, ~0u);

    m_builder.get_widget("add_contact_tox_id", m_add_contact.tox_id);
    m_builder.get_widget("add_contact_message", m_add_contact.message);

    m_builder.get_widget("popover_stack", m_stack);

    add(*m_stack);

    /* Tox-Id display */
    std::string hex = Toxmm::to_hex(tox().get_address().data(),
                                    tox().get_address().size());
    for (int i = 4; i > 0; --i) {
        hex.insert(2 * i * TOX_PUBLIC_KEY_SIZE / 4, 1, '\n');
    }

    m_builder.get_widget<Gtk::Label>("profile_tox_hex_id")
            ->set_markup("<span font_desc=\"mono\">" + hex + "</span>");

    m_builder.get_widget<Gtk::Button>("profile_copy_tox_id")
            ->signal_clicked().connect([this]() {
        Gtk::Clipboard::get()->set_text(
                    Toxmm::to_hex(tox().get_address().data(),
                                  tox().get_address().size()));
    });

    /* Open settings */
    m_builder.get_widget<Gtk::Button>("profile_open_settings")
            ->signal_clicked().connect([this]() {
        hide();
        //m_settings.show();
    });

    /* change avatar logic */
    m_builder.get_widget<Gtk::Button>("profile_avatar_btn")
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

    /* Update name / status message logic */
    auto update = [this](GdkEventFocus*) {
        observer_notify(ToxEvent(DialogContact::EventSetName{
                                     m_profile.username->get_text(),
                                     m_profile.status->get_text()
                                 }));
    };

    m_profile.username->signal_focus_out_event().connect_notify(update);
    m_profile.status  ->signal_focus_out_event().connect_notify(update);

    /* Add contact submit button handling */
    m_builder.get_widget<Gtk::Button>("add_contact_submit")
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

PopoverSettings::~PopoverSettings() {
}

void PopoverSettings::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    // update data
    if (!visible) {
        return;
    }

    m_stack->set_visible_child("popover_profile", Gtk::STACK_TRANSITION_TYPE_NONE);

    m_profile.username->set_text(tox().get_name());
    m_profile.status->set_text(tox().get_status_message());

    m_profile.username->grab_focus();
}

void PopoverSettings::add_contact() {
    Gtk::Window& parent = dynamic_cast<Gtk::Window&>(*this->get_toplevel());

    if (m_add_contact.tox_id->get_text().length() != TOX_ADDRESS_SIZE * 2) {
        DialogError(parent, false,
                    _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE_TITLE"),
                    _("ERROR_ADD_CONTACT_ADDR_WRONG_SIZE")).run();
        return;
    }

    try {
        Toxmm::FriendAddr adr;
        auto adr_c = Toxmm::from_hex(m_add_contact.tox_id->get_text());
        std::copy(adr_c.begin(), adr_c.end(), adr.begin());
        observer_notify(ToxEvent(DialogContact::EventAddContact{tox().add_friend(adr, m_add_contact.message->get_buffer()->get_text())}));
        tox().save();

        set_visible(false);
        m_add_contact.tox_id->set_text("");
        m_add_contact.message->get_buffer()->set_text("");
    } catch (Toxmm::Exception &ex) {
        if (ex.type() != typeid(TOX_ERR_FRIEND_ADD)) {
            throw;
        }
        switch(ex.what_id()) {
            case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_NO_MESSAGE_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_BAD_CHECKSUM_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_ALREADY_SENT_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_OWN_KEY:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_OWN_KEY_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM_UI")).run();
                break;
            case TOX_ERR_FRIEND_ADD_TOO_LONG:
                DialogError(parent, false,
                            _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI_TITLE"),
                            _("TOX_ERR_FRIEND_ADD_TOO_LONG_UI")).run();
                break;
            default:
                throw;
        }
    }
}
