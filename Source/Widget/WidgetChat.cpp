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
#include "WidgetChat.h"
#include "Tox/Toxmm.h"
#include "Chat/WidgetChatBubble.h"
#include "Chat/WidgetChatLabel.h"
#include <glibmm/i18n.h>
#include <iostream>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

WidgetChat::WidgetChat(gToxObservable* instance, Toxmm::FriendNr nr)
    : Glib::ObjectBase("WidgetChat"), m_nr(nr), m_autoscroll(true) {

    set_observable(instance);

    m_scrolled.add(m_vbox);
    m_vbox.set_spacing(5);
    auto frame = Gtk::manage(new Gtk::Frame());
    frame->add(m_scrolled);
    pack1(*frame, true, false);
    frame->get_style_context()->remove_class("frame");
    // pack2(input, false, true);

    m_btn_send.set_label(_("SEND"));

    m_hbox.pack_start(m_input, true, true);
    m_hbox.pack_end(m_btn_send, false, false);
    pack2(m_hbox, false, false);

    m_input.set_wrap_mode(Gtk::WRAP_WORD_CHAR);

    // set_position(400);
    m_hbox.set_size_request(-1, 80);

    m_input.signal_key_press_event().connect(
        [this](GdkEventKey* event) {
            auto text_buffer = m_input.get_buffer();
            if (event->keyval == GDK_KEY_Return
                && !(event->state & GDK_SHIFT_MASK)) {
                if (text_buffer->begin() != text_buffer->end()) {
                    m_btn_send.clicked();
                    return true;
                }
            }

            return false;
        },
        false);

    m_btn_send.signal_clicked().connect([this]() {
        try {
            bool allow_send = m_input.get_buffer()->get_text().find_first_not_of(" \t\n\v\f\r") != std::string::npos;

            if(!allow_send)
                return;

            // add to chat
            auto text = m_input.get_serialized_text();
            add_message(WidgetChatBubble::RIGHT, WidgetChatBubble::Line{
                         false,
                         true,
                         tox().send_message(get_friend_nr(), text),
                         0,
                         get_friend_nr(),
                         text
                     });

            // clear chat input
            m_input.get_buffer()->set_text("");
        } catch (...) {
            // not online ?
        }
    });

    m_vbox.set_name("WidgetChat");
    m_vbox.property_margin() = 10;  // wont work via css

    m_scrolled.get_vadjustment()->signal_value_changed().connect_notify(
        [this]() {
            // check if lowest position
            auto adj = m_scrolled.get_vadjustment();
            m_autoscroll = adj->get_upper() - adj->get_page_size()
                           == adj->get_value();
        });

    m_vbox.signal_size_allocate().connect_notify([this](Gtk::Allocation&) {
        // auto scroll:
        if (m_autoscroll) {
            auto adj = m_scrolled.get_vadjustment();
            adj->set_value(adj->get_upper() - adj->get_page_size());
        }
    });

    // Disable scroll to focused child
    auto viewport = dynamic_cast<Gtk::Viewport*>(m_scrolled.get_child());
    if (viewport) {
        auto dummy_adj = Gtk::Adjustment::create(0, 0, 0);
        viewport->set_focus_hadjustment(dummy_adj);
        viewport->set_focus_vadjustment(dummy_adj);
    }

    // load log
    auto log = tox().get_log(nr);
    for (auto l : log) {
        switch (l.type) {
            case EToxLogType::LOG_MESSAGE_SEND:
            case EToxLogType::LOG_ACTION_SEND:
                add_message(WidgetChatBubble::RIGHT, WidgetChatBubble::Line{
                             false,
                             false,
                             0,
                             l.sendtime,
                             nr,
                             l.data
                         });
                break;
            case EToxLogType::LOG_MESSAGE_RECV:
            case EToxLogType::LOG_ACTION_RECV:
                add_message(WidgetChatBubble::LEFT, WidgetChatBubble::Line{
                             false,
                             false,
                             0,
                             l.recvtime,
                             0,
                             l.data
                         });
                break;
            case LOG_FILE_RECV: {
                Toxmm::EventFileRecv data;
                data.filename = l.data;
                data.file_number = l.filenumber;
                data.file_size = l.filesize;
                data.kind = TOX_FILE_KIND::TOX_FILE_KIND_DATA;
                data.nr = nr;
                //add_filerecv(data);
                } break;
            case LOG_FILE_SEND:
                add_filesend(l);
                break;
            default:
                break;
        }
    }

    m_tox_callback = observer_add([this, nr](const ToxEvent& ev) {
        if (ev.type() == typeid(Toxmm::EventFriendAction)) {
            auto data = ev.get<Toxmm::EventFriendAction>();
            if (nr == data.nr) {
                add_message(WidgetChatBubble::LEFT, WidgetChatBubble::Line{
                             false,
                             false,
                             0,
                             0,
                             data.nr,
                             data.message
                         });
            }
        } else if (ev.type() == typeid(Toxmm::EventFriendMessage)) {
            auto data = ev.get<Toxmm::EventFriendMessage>();
            if (nr == data.nr) {
                add_message(WidgetChatBubble::LEFT, WidgetChatBubble::Line{
                             false,
                             false,
                             0,
                             0,
                             data.nr,
                             data.message
                         });
            }
        } else if (ev.type() == typeid(gToxFileManager::EventNewFile)) {
            auto data = ev.get<gToxFileManager::EventNewFile>();
            if (nr == data.file->friend_nr() &&
                    data.file->file_kind() == TOX_FILE_KIND_DATA) {
                add_filerecv(data.file);
            }
        }
    });
}

WidgetChat::~WidgetChat() {
}

void WidgetChat::focus() {
    m_input.grab_focus();
}

Toxmm::FriendNr WidgetChat::get_friend_nr() const {
    return m_nr;
}

void WidgetChat::add_message(WidgetChatBubble::Side side, WidgetChatBubble::Line message) {
    // check if time i set, if not we will give it actual time
    if (message.timestamp == 0) {
        message.timestamp = Glib::DateTime::create_now_utc().to_unix();
    }

    auto last_timestmap = m_last_timestamp;
    auto last_side = m_last_side;

    if (need_date(last_timestmap, message.timestamp)) {
        // add a date message
        auto msg = Gtk::manage(new WidgetChatLabel());
        msg->set_text(Glib::DateTime::create_now_local(message.timestamp)
                          .format(_("DATE_FORMAT")));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();

        add_widget(*msg);

        last_side   = WidgetChatBubble::NONE;
    }

    bool action = message.message.find("/me ") == 0;

    if (!action && same_bubble(last_timestmap, last_side,
                               message.timestamp, side)) {
        //on same bubble !
        WidgetChatBubble* bubble = dynamic_cast<WidgetChatBubble*>(m_vbox.get_children().back());
        if (bubble) {
            bubble->add_message(message);

            m_last_timestamp = message.timestamp;
            m_last_side = side;
            return;
        }
    }

    if (action) {
        // TODO new action line
        auto msg = Gtk::manage(new WidgetChatLabel());
        auto name = (side == WidgetChatBubble::LEFT)
                    ? tox().get_name_or_address(m_nr)
                    : tox().get_name_or_address();
        msg->set_text(name + message.message.substr(Glib::ustring("/me").size()));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();

        add_widget(*msg);

        m_last_side = WidgetChatBubble::NONE;
        return;
    }

    //add new bubble
    auto new_bubble = Gtk::manage(new WidgetChatBubble(observable(), (side == WidgetChatBubble::LEFT)?m_nr:~0u, side));
    new_bubble->add_message(message);
    new_bubble->show_all();

    add_widget(*new_bubble);

    m_last_timestamp = message.timestamp;
    m_last_side = side;
}

void WidgetChat::add_filerecv(std::shared_ptr<gToxFileTransf> file) {
    //more or less the same as add_message..
    WidgetChatBubble::Side side = WidgetChatBubble::Side::LEFT;

    // check if time i set, if not we will give it actual time
    auto timestamp = Glib::DateTime::create_now_utc().to_unix();

    auto last_timestmap = m_last_timestamp;
    auto last_side = m_last_side;

    m_last_timestamp = timestamp;
    m_last_side = side;

    if (need_date(last_timestmap, timestamp)) {
        // add a date message
        auto msg = Gtk::manage(new WidgetChatLabel());
        msg->set_text(Glib::DateTime::create_now_local(timestamp)
                          .format(_("DATE_FORMAT")));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();

        add_widget(*msg);

        last_side   = WidgetChatBubble::NONE;
    }

    if (same_bubble(last_timestmap, last_side,
                    timestamp, side)) {
        //on same bubble !
        WidgetChatBubble* bubble = dynamic_cast<WidgetChatBubble*>(m_vbox.get_children().back());
        if (bubble) {
            bubble->add_filerecv(file);

            m_last_timestamp = timestamp;
            m_last_side = side;
            return;
        }
    }

    //add new bubble
    auto new_bubble = Gtk::manage(new WidgetChatBubble(observable(), (side == WidgetChatBubble::LEFT)?m_nr:~0u, side));
    new_bubble->add_filerecv(file);
    new_bubble->show_all();

    add_widget(*new_bubble);

    m_last_timestamp = timestamp;
    m_last_side = side;
}

void WidgetChat::add_widget(Gtk::Widget& widget) {
    m_vbox.pack_start(widget, false, false);
}

bool WidgetChat::same_bubble(unsigned long long a_timestamp, WidgetChatBubble::Side a_side, unsigned long long b_timestamp, WidgetChatBubble::Side b_side) {
    return (a_side == b_side) && !need_date(a_timestamp, b_timestamp);
}

bool WidgetChat::need_date(unsigned long long a_timestamp, unsigned long long b_timestamp) {
    auto a_time = Glib::DateTime::create_now_utc(a_timestamp);
    auto b_time = Glib::DateTime::create_now_utc(b_timestamp);

    return  a_time.get_year() != b_time.get_year() ||
            a_time.get_month() != b_time.get_month() ||
            a_time.get_day_of_month() != b_time.get_day_of_month();
}

void WidgetChat::add_filesend(Glib::ustring uri) {
    //more or less the same as add_message...
    WidgetChatBubble::Side side = WidgetChatBubble::Side::RIGHT;

    auto timestamp = Glib::DateTime::create_now_utc().to_unix();

    auto last_timestmap = m_last_timestamp;
    auto last_side = m_last_side;

    m_last_timestamp = timestamp;
    m_last_side = side;

    if (need_date(last_timestmap, timestamp)) {
        // add a date message
        auto msg = Gtk::manage(new WidgetChatLabel());
        msg->set_text(Glib::DateTime::create_now_local(timestamp)
                          .format(_("DATE_FORMAT")));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();

        add_widget(*msg);

        last_side   = WidgetChatBubble::NONE;
    }

    if (same_bubble(last_timestmap, last_side,
                    timestamp, side)) {
        //on same bubble !
        WidgetChatBubble* bubble = dynamic_cast<WidgetChatBubble*>(m_vbox.get_children().back());
        if (bubble) {
            bubble->add_filesend(m_nr, uri);

            m_last_timestamp = timestamp;
            m_last_side = side;
            return;
        }
    }

    //add new bubble
    auto new_bubble = Gtk::manage(new WidgetChatBubble(observable(), (side == WidgetChatBubble::LEFT)?m_nr:~0u, side));
    new_bubble->add_filesend(m_nr, uri);
    new_bubble->show_all();

    add_widget(*new_bubble);

    m_last_timestamp = timestamp;
    m_last_side = side;
}

void WidgetChat::add_filesend(ToxLogEntity data) {
    //more or less the same as add_message...
    WidgetChatBubble::Side side = WidgetChatBubble::Side::RIGHT;
    auto timestamp = data.sendtime;

    auto last_timestmap = m_last_timestamp;
    auto last_side = m_last_side;

    m_last_timestamp = timestamp;
    m_last_side = side;

    if (need_date(last_timestmap, timestamp)) {
        // add a date message
        auto msg = Gtk::manage(new WidgetChatLabel());
        msg->set_text(Glib::DateTime::create_now_local(timestamp)
                          .format(_("DATE_FORMAT")));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();

        add_widget(*msg);

        last_side   = WidgetChatBubble::NONE;
    }

    if (same_bubble(last_timestmap, last_side,
                    timestamp, side)) {
        //on same bubble !
        WidgetChatBubble* bubble = dynamic_cast<WidgetChatBubble*>(m_vbox.get_children().back());
        if (bubble) {
            bubble->add_filesend(m_nr, data);

            m_last_timestamp = timestamp;
            m_last_side = side;
            return;
        }
    }

    //add new bubble
    auto new_bubble = Gtk::manage(new WidgetChatBubble(observable(), (side == WidgetChatBubble::LEFT)?m_nr:~0u, side));
    new_bubble->add_filesend(m_nr, data);
    new_bubble->show_all();

    add_widget(*new_bubble);

    m_last_timestamp = timestamp;
    m_last_side = side;
}
