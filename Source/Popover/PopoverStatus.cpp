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
#include "Dialog/DialogContact.h"
#include <glibmm/i18n.h>

PopoverStatus::PopoverStatus(gToxObservable* instance, const Gtk::Widget& relative_to)
    : Gtk::Popover(relative_to) {
    // add_label("Settings");

    set_observable(instance);

    m_listbox.add(create_item(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/status_online.svg"), _("ONLINE")));
    m_listbox.add(create_item(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/status_busy.svg"), _("BUSY")));
    m_listbox.add(create_item(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/status_away.svg"), _("AWAY")));
    m_listbox.add(create_item(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/status_offline.svg"), _("EXIT")));
    m_listbox.show_all();
    add(m_listbox);

    // signal handling
    m_listbox.signal_row_activated().connect([this](Gtk::ListBoxRow* row) {
        switch (row->get_index()) {
            case 0:
                observer_notify(ToxEvent(DialogContact::EventSetStatus{
                                              Toxmm::NONE
                                          }));
                break;
            case 1:
                observer_notify(ToxEvent(DialogContact::EventSetStatus{
                                              Toxmm::BUSY
                                          }));
                break;
            case 2:
                observer_notify(ToxEvent(DialogContact::EventSetStatus{
                                              Toxmm::AWAY
                                          }));
                break;
            case 3:
                observer_notify(ToxEvent(DialogContact::EventSetStatus{
                                              Toxmm::OFFLINE
                                          }));
                break;
        }
        set_visible(false);
    });
}

PopoverStatus::~PopoverStatus() {
}

Gtk::ListBoxRow& PopoverStatus::create_item(Glib::RefPtr<Gdk::Pixbuf> icon,
                                            Glib::ustring text) {
    auto row = Gtk::manage(new Gtk::ListBoxRow());
    auto hbox = Gtk::manage(new Gtk::HBox());
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

    // update selection
    if (!visible) {
        return;
    }

    int select = 3;
    switch (tox().get_status()) {
        case Toxmm::NONE:
            select = 0;
            break;
        case Toxmm::BUSY:
            select = 1;
            break;
        case Toxmm::AWAY:
            select = 2;
            break;
        default:
            break;
    }
    m_listbox.select_row(*m_listbox.get_row_at_index(select));
}
