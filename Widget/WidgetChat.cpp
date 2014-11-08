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
#include "Tox/Tox.h"
#include "WidgetChatLine.h"
#include "Chat/WidgetChatLabel.h"
#include <glibmm/i18n.h>
#include <iostream>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

WidgetChat::WidgetChat(Tox::FriendNr nr)
    : Glib::ObjectBase("WidgetChat"), m_nr(nr), m_autoscroll(true) {
    m_output.set_editable(false);
    m_scrolled.add(m_vbox);
    m_vbox.set_spacing(5);
    auto frame = Gtk::manage(new Gtk::Frame());
    frame->add(m_scrolled);
    pack1(*frame, true, false);
    // pack2(input, false, true);

    m_btn_send.set_label(_("SEND"));

    m_hbox.pack_start(m_input, true, true);
    m_hbox.pack_end(m_btn_send, false, false);
    pack2(m_hbox, false, false);

    m_input.set_wrap_mode(Gtk::WRAP_WORD_CHAR);

    // set_position(400);
    m_hbox.set_size_request(-1, 80);

    auto text_buffer = m_input.get_buffer();

    auto bold = text_buffer->create_tag("bold");
    auto italic = text_buffer->create_tag("italic");
    auto underline = text_buffer->create_tag("underline");

    bold->property_weight() = Pango::WEIGHT_BOLD;
    italic->property_style() = Pango::STYLE_ITALIC;
    underline->property_underline() = Pango::UNDERLINE_SINGLE;

    m_input.signal_key_press_event().connect(
        [this, text_buffer, bold, italic, underline](GdkEventKey* event) {
            if (event->keyval == GDK_KEY_Return
                && !(event->state & GDK_SHIFT_MASK)) {
                if (text_buffer->begin() != text_buffer->end()) {
                    m_btn_send.clicked();
                    return true;
                }
            }
            // text formating
            Gtk::TextBuffer::iterator begin;
            Gtk::TextBuffer::iterator end;
            if (text_buffer->get_selection_bounds(begin, end) && begin != end
                && event->state & GDK_CONTROL_MASK) {
                auto mode = bold;
                switch (event->keyval) {
                    case 'b':
                        // default
                        break;
                    case 'i':
                        mode = italic;
                        break;
                    case 'u':
                        mode = underline;
                        break;
                    default:
                        return false;
                }
                // toggle text
                while (begin < end) {
                    if (begin.has_tag(mode)) {
                        // remove
                        auto tag_end = begin;
                        tag_end.forward_to_tag_toggle(mode);
                        if (tag_end > end) {
                            tag_end = end;
                        }
                        text_buffer->remove_tag(mode, begin, tag_end);
                        begin = tag_end;
                    } else {
                        // add
                        auto tag_start = begin;
                        do {
                            if (!tag_start.forward_to_tag_toggle(mode)) {
                                tag_start = end;
                                break;
                            }
                        } while (!tag_start.begins_tag(mode));
                        if (tag_start > end) {
                            tag_start = end;
                        }
                        text_buffer->apply_tag(mode, begin, tag_start);
                        begin = tag_start;
                    }
                }
            }
            return false;
        },
        false);

    m_btn_send.signal_clicked().connect([this, bold, italic, underline]() {
        try {
            Glib::ustring text;
            auto begin = m_input.get_buffer()->begin();
            auto end = m_input.get_buffer()->end();

            for (; begin != end; begin++) {
                // open
                if (begin.begins_tag(bold)) {
                    text += gunichar(0xFDD0);
                    text += "**";
                    text += gunichar(0xFDD1);
                }
                if (begin.begins_tag(italic)) {
                    text += gunichar(0xFDD0);
                    text += "*";
                    text += gunichar(0xFDD1);
                }
                if (begin.begins_tag(underline)) {
                    text += gunichar(0xFDD0);
                    text += "_";
                    text += gunichar(0xFDD1);
                }
                // close
                if (begin.ends_tag(bold)) {
                    text += gunichar(0xFDD1);
                    text += "**";
                    text += gunichar(0xFDD0);
                }
                if (begin.ends_tag(italic)) {
                    text += gunichar(0xFDD1);
                    text += "*";
                    text += gunichar(0xFDD0);
                }
                if (begin.ends_tag(underline)) {
                    text += gunichar(0xFDD1);
                    text += "_";
                    text += gunichar(0xFDD0);
                }
                // add text
                text += begin.get_char();
            }

            Tox::instance().send_message(get_friend_nr(), text);

            // add to chat
            add_line(0, false, text);

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
    auto log = Tox::instance().get_log(nr);
    for (auto l : log) {
        if (l.sendtime != 0) {
            add_line(l.sendtime, false, l.data);
        } else {
            add_line(l.recvtime, true, l.data);
        }
    }
}

WidgetChat::~WidgetChat() {
}

void WidgetChat::focus() {
    m_input.grab_focus();
}

Tox::FriendNr WidgetChat::get_friend_nr() const {
    return m_nr;
}

void WidgetChat::add_line(Glib::ustring text) {
    m_output.add_line(text);
}

void WidgetChat::add_line(unsigned long long timestamp,
                          bool left_side,
                          const Glib::ustring& message) {
    // check if time i set, if not we will give it actual time
    if (timestamp == 0) {
        timestamp = Glib::DateTime::create_now_utc().to_unix();
    }
    auto new_time = Glib::DateTime::create_now_utc(timestamp);
    new_time = Glib::DateTime::create_utc(new_time.get_year(),
                                          new_time.get_month(),
                                          new_time.get_day_of_month(),
                                          0,
                                          0,
                                          0);
    decltype(new_time) last_time = Glib::DateTime::create_now_utc(0);

    bool action = message.find("/me ") == 0;

    // check last message blob
    std::vector<Gtk::Widget*> childs = m_vbox.get_children();
    if (!childs.empty()) {
        WidgetChatLine* item = dynamic_cast<WidgetChatLine*>(childs.back());
        if (item != nullptr) {
            last_time = Glib::DateTime::create_now_utc(item->last_timestamp());
            last_time = Glib::DateTime::create_utc(last_time.get_year(),
                                                   last_time.get_month(),
                                                   last_time.get_day_of_month(),
                                                   0,
                                                   0,
                                                   0);
            // check if blob is on same side
            if (!action && item->get_side() == left_side) {
                // check if it's same day month year
                if (last_time.compare(new_time) == 0) {
                    item->add_line(timestamp, message);
                    return;
                }
            }
        }
    }

    // check if we need to add a date-line
    if (last_time.compare(new_time) != 0) {
        // add a date message
        auto msg = Gtk::manage(new WidgetChatLabel() /*new Gtk::Label()*/);
        msg->set_text(Glib::DateTime::create_now_local(timestamp)
                          .format(_("DATE_FORMAT")));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();
        m_vbox.pack_start(*msg, false, false);
    }

    if (!action) {
        // add new line
        auto new_line = Gtk::manage(new WidgetChatLine(left_side));
        new_line->add_line(timestamp, message);
        new_line->show_all();
        m_vbox.pack_start(*new_line, false, false);
    } else {
        // TODO add a WidgetChatLineMe ..
        // add new action line
        auto msg = Gtk::manage(new WidgetChatLabel() /*new Gtk::Label()*/);
        auto name = left_side ? Tox::instance().get_name_or_address(m_nr)
                              : Tox::instance().get_name_or_address();
        msg->set_text(name + message.substr(Glib::ustring("/me").size()));
        msg->set_name("ChatTime");
        msg->set_halign(Gtk::ALIGN_CENTER);
        msg->show_all();
        m_vbox.pack_start(*msg, false, false);
    }
}
