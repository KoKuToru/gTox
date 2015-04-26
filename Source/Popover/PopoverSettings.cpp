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
#include <glibmm/i18n.h>

PopoverSettings::PopoverSettings(gToxObservable* observable,
                                 const Gtk::Widget& relative_to)
    : Gtk::Popover(relative_to),
      gToxObserver(observable),
      m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/popover_settings.ui")) {

    m_builder.get_widget("profile_username_entry", m_username);
    m_builder.get_widget("profile_statusmessage_entry", m_status);

    add(*m_builder.get_widget<Gtk::Grid>("popover_settings"));

    std::string hex = Toxmm::to_hex(tox().get_address().data(),
                                    tox().get_address().size());
    for (int i = 4; i > 0; --i) {
        hex.insert(2 * i * TOX_PUBLIC_KEY_SIZE / 4, 1, '\n');
    }

    m_builder.get_widget<Gtk::Label>("profile_tox_hex_id")
            ->set_markup("<span font_desc=\"mono\">" + hex + "</span>");

    m_builder.get_widget<Gtk::Button>("profile_open_settings")
            ->signal_clicked().connect([this]() {
        hide();
        //m_settings.show();
    });

    m_builder.get_widget<Gtk::Button>("profile_copy_tox_id")
            ->signal_clicked().connect([this]() {
        Gtk::Clipboard::get()->set_text(
                    Toxmm::to_hex(tox().get_address().data(),
                                  tox().get_address().size()));
    });

    auto update = [this](GdkEventFocus*) {
        observer_notify(ToxEvent(DialogContact::EventSetName{
                                     m_username->get_text(),
                                     m_status->get_text()
                                 }));
    };

    m_username->signal_focus_out_event().connect_notify(update);
    m_status  ->signal_focus_out_event().connect_notify(update);
}

PopoverSettings::~PopoverSettings() {
}

void PopoverSettings::set_visible(bool visible) {
    Gtk::Popover::set_visible(visible);

    // update data
    if (!visible) {
        return;
    }

    m_username->set_text(tox().get_name());
    m_status->set_text(tox().get_status_message());

    m_username->grab_focus();
}
