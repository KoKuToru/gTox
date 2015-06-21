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
#include "WidgetChatLine.h"
#include "Chat/WidgetChatLabel.h"
#include "Tox/Toxmm.h"

WidgetChatLine::WidgetChatLine(gToxObservable* instance, Toxmm::FriendNr nr, Side left_side)
    : Glib::ObjectBase("WidgetChatLine"), m_side(left_side), m_row_count(0), m_avatar(instance, nr) {

    set_observable(instance);

    set_halign((m_side == LEFT) ? Gtk::Align::ALIGN_START : Gtk::Align::ALIGN_END);
    get_style_context()->add_class((m_side == LEFT) ? "gtox-left" : "gtox-right");

    auto hbox = Gtk::manage(new Gtk::HBox());
    auto frame = Gtk::manage(new Gtk::Frame());
    add(*hbox);
    if (m_side == LEFT) {
        hbox->pack_start(m_avatar);
        m_avatar.set_valign(Gtk::ALIGN_START);
        m_avatar.set_halign(Gtk::ALIGN_END);
    }
    hbox->add(*frame);
    if (m_side == RIGHT) {
        hbox->pack_end(m_avatar);
        m_avatar.set_valign(Gtk::ALIGN_END);
        m_avatar.set_halign(Gtk::ALIGN_START);
    }
    frame->add(m_grid);

    m_avatar.set_name("Avatar");
    m_avatar.set_size_request(0, 0);
    //TODO: replace this, it's deprecated
    m_avatar.property_xalign() = (m_side == LEFT) ? 1 : 0;
    m_avatar.property_yalign() = (m_side == LEFT) ? 1 : 0;
//    m_avatar.set_tooltip_text("TODO Display name here..");
    m_avatar.get_style_context()->add_class("frame");

    frame->get_style_context()->add_class("gtox-bubble");

    show_all();

    m_last_timestamp = 0;

    m_grid.show_all();
}

WidgetChatLine::~WidgetChatLine() {
}

WidgetChatLine::Side WidgetChatLine::get_side() {
    return m_side;
}

void WidgetChatLine::add_message(Line new_line) {
    auto msg_time = Glib::DateTime::create_now_utc(new_line.timestamp);
    // remove seconds
    msg_time = Glib::DateTime::create_utc(msg_time.get_year(),
                                          msg_time.get_month(),
                                          msg_time.get_day_of_month(),
                                          msg_time.get_hour(),
                                          msg_time.get_minute(),
                                          0);

    bool display_time = true;

    if (m_last_timestamp != 0) {
        auto old_time = Glib::DateTime::create_now_utc(m_last_timestamp);
        // remove seconds
        old_time = Glib::DateTime::create_utc(old_time.get_year(),
                                              old_time.get_month(),
                                              old_time.get_day_of_month(),
                                              old_time.get_hour(),
                                              old_time.get_minute(),
                                              0);
        // check
        display_time = !(msg_time.compare(old_time) == 0);
    }

    // create a new row
    auto msg  = Gtk::manage(new WidgetChatLabel());
    auto time = Gtk::manage(new Gtk::Label());
    m_last_timestamp = new_line.timestamp;

    // get local time
    msg_time = Glib::DateTime::create_now_local(m_last_timestamp);

    time->set_text(msg_time.format("%R"));
    msg->set_text(new_line.message);

    // add to grid
    if (m_side == RIGHT) {
        rows.emplace_back(m_grid, m_row_count, *msg, *time);
    } else {
        rows.emplace_back(m_grid, m_row_count, *time, *msg);
    }
    if (new_line.wait_for_receipt) {
        rows.back().set_class("message_pending");
        //callback !
        auto nr = new_line.nr;
        auto receipt = new_line.receipt;
        auto index = rows.size() - 1;
        rows.back().tox_callback = observer_add([this, index, nr, receipt](const ToxEvent &ev) {
            //wait for receipt
            if (ev.type() == typeid(Toxmm::EventReadReceipt)) {
                auto& row = rows[index];
                auto data = ev.get<Toxmm::EventReadReceipt>();
                if (data.nr == nr) {
                    if (data.receipt > receipt) {
                        //message failed !
                        row.set_class("message_failed");
                        row.tox_callback.reset();
                    } else if (data.receipt == receipt) {
                        //message got !
                        row.set_class("message_receipt");
                        row.tox_callback.reset();
                    }
                }
            }
        });
    }
    if (new_line.error) {
        rows.back().set_class("message_failed");
    }
    for(size_t i = 0; i < rows.size(); ++i) {
        rows[i].set_class(i == 0, i == rows.size()-1);
    }
    m_row_count += 1;

    // styling
    time->set_halign(Gtk::ALIGN_CENTER);
    time->set_valign(Gtk::ALIGN_START);
    time->get_style_context()->add_class("bubble_chat_line_time");

    msg->set_halign(Gtk::ALIGN_START);
    msg->set_valign(Gtk::ALIGN_CENTER);

    msg->show_all();
    time->show_all();
    time->set_no_show_all();
    if (!display_time) {
        time->hide();
    }
}

void WidgetChatLine::on_size_allocate(Gtk::Allocation& allocation) {
    int w = std::min(64, allocation.get_height() - 5); //5px radius

    //update widget size:
    if (w != m_avatar.get_width()) {
        Glib::signal_idle().connect_once([this, w](){
            m_avatar.set_size_request(w, w);
        });
    }

    Gtk::Box::on_size_allocate(allocation);
}

unsigned long long WidgetChatLine::last_timestamp() {
    return m_last_timestamp;
}
