/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#include "chat_input.h"
#include "tox/tox.h"

#ifndef SIGC_CPP11_HACK
#define SIGC_CPP11_HACK
namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}
#endif

using namespace widget;

chat_input::chat_input(BaseObjectType* cobject, utils::builder): Gtk::TextView(cobject) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_buffer = get_buffer();

    m_bold_tag = m_buffer->create_tag("bold");
    m_italic_tag = m_buffer->create_tag("italic");
    m_underline_tag = m_buffer->create_tag("underline");

    m_bold_tag->property_weight() = Pango::WEIGHT_BOLD;
    m_italic_tag->property_style() = Pango::STYLE_ITALIC;
    m_underline_tag->property_underline() = Pango::UNDERLINE_SINGLE;

    signal_key_press_event().connect([this](GdkEventKey* event) {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
        // text formating
        Gtk::TextBuffer::iterator begin;
        Gtk::TextBuffer::iterator end;
        if (m_buffer->get_selection_bounds(begin, end) && begin != end
            && event->state & GDK_CONTROL_MASK) {
            auto tag = m_bold_tag;
            switch (event->keyval) {
                case 'b':
                    // default
                    break;
                case 'i':
                    tag = m_italic_tag;
                    break;
                case 'u':
                    tag = m_underline_tag;
                    break;
                default:
                    return false;
            }
            // toggle text
            while (begin < end) {
                if (begin.has_tag(tag)) {
                    // remove
                    auto tag_end = begin;
                    tag_end.forward_to_tag_toggle(tag);
                    if (tag_end > end) {
                        tag_end = end;
                    }
                    m_buffer->remove_tag(tag, begin, tag_end);
                    begin = tag_end;
                } else {
                    // add
                    auto tag_start = begin;
                    do {
                        if (!tag_start.forward_to_tag_toggle(tag)) {
                            tag_start = end;
                            break;
                        }
                    } while (!tag_start.begins_tag(tag));
                    if (tag_start > end) {
                        tag_start = end;
                    }
                    m_buffer->apply_tag(tag, begin, tag_start);
                    begin = tag_start;
                }
            }
        }
        return false;
    }, false);

    m_buffer->signal_changed().connect(sigc::track_obj([this]() {
        // this could be pretty expensive for long texts
        auto iter = m_buffer->begin();
        bool reset_each_line = true;
        if (iter.get_chars_in_line() >= 4) {
            auto cmd_end = m_buffer->begin();
            cmd_end.forward_chars(4);
            if (m_buffer->get_text(iter, cmd_end) == "/me ") {
                reset_each_line = false;
            }
        }
        auto text_size = 0;
        auto total_size = 0;
        do {
            total_size += iter.get_bytes_in_line();
            if (total_size >= 10 * TOX_MAX_MESSAGE_LENGTH) {
                // yeah.. uhm.. jump to byte offset is
                // pretty much impossible ?
            }

            auto end_line = iter;
            if (!iter.ends_line()) {
                end_line.forward_to_line_end();
            }

            auto line = iter.get_text(end_line);
            if (!end_line.is_end()) {
                line += '\n';
            }

            text_size += line.bytes();
            if (text_size >= TOX_MAX_MESSAGE_LENGTH) {
                int counted_size = 0;
                do {
                    // this must be a performance killer for sure..
                    counted_size += Glib::ustring(1, iter.get_char()).bytes();
                    if (counted_size >= TOX_MAX_MESSAGE_LENGTH) {
                        break;
                    }
                } while (iter.forward_char());
                // remove everything after
                m_buffer->erase(iter, m_buffer->end());
                return;
            }
            if (reset_each_line) {
                text_size = 0;
            }
        } while (iter.forward_line());
    }, *this));
}

chat_input::~chat_input() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

std::vector<Glib::ustring> chat_input::get_text_new() {
    std::vector<Glib::ustring> result;
    Glib::ustring tmp;

    auto iter = m_buffer->begin();

    if (iter.get_chars_in_line() >= 4) {
        auto cmd_end = m_buffer->begin();
        cmd_end.forward_chars(4);
        if (m_buffer->get_text(iter, cmd_end) == "/me ") {
            result.push_back(m_buffer->get_text());
            return result;
        }
    }

    do {
        auto end_line = iter;
        if (!iter.ends_line()) {
            end_line.forward_to_line_end();
        }

        auto line = iter.get_text(end_line);
        if (!end_line.is_end()) {
            line += '\n';
        }

        if (tmp.bytes() + line.bytes() >= TOX_MAX_MESSAGE_LENGTH) {
            // remove last new line
            //tmp.erase(tmp.rbegin());
            auto rbegin = tmp.end();
            rbegin--;
            tmp.erase(rbegin);
            // add to result
            result.push_back(tmp);
            tmp.clear();
        }

        tmp += line;
    } while (iter.forward_line());

    if (!tmp.empty()) {
        result.push_back(tmp);
    }

    for (auto& t : result) {
        if (t.bytes() >= TOX_MAX_MESSAGE_LENGTH) {
            // what ?
            abort();
        }
    }

    return result;
}

Glib::ustring chat_input::get_serialized_text() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    // maybe for the future
    Glib::ustring text;
    auto begin = m_buffer->begin();
    auto end = m_buffer->end();

    bool b_open = false;
    bool i_open = false;
    bool u_open = false;

    //http://www.unicode.org/faq/private_use.html#nonchar4
    //used to work, not Gnome shows white boxes
    auto tag_open     = gunichar(0xFDD0);
    auto tag_close    = gunichar(0xFDD1);

    for (; begin != end; begin++) {
        // open
        if (begin.begins_tag(m_bold_tag)) {
            text += tag_open;
            text += "**";
            text += tag_close;
            b_open = true;
        }
        if (begin.begins_tag(m_italic_tag)) {
            text += tag_open;
            text += "*";
            text += tag_close;
            i_open = true;
        }
        if (begin.begins_tag(m_underline_tag)) {
            text += tag_open;
            text += "_";
            text += tag_close;
            u_open = true;
        }
        // close
        if (begin.ends_tag(m_underline_tag)) {
            text += tag_close;
            text += "_";
            text += tag_close;
            u_open = false;
        }
        if (begin.ends_tag(m_italic_tag)) {
            text += tag_close;
            text += "*";
            text += tag_open;
            i_open = false;
        }
        if (begin.ends_tag(m_bold_tag)) {
            text += tag_close;
            text += "**";
            text += tag_open;
            b_open = false;
        }

        // add text
        text += begin.get_char();
    }

    if (u_open) {
        text += tag_close;
        text += "_";
        text += tag_open;
    }
    if (i_open) {
        text += tag_close;
        text += "*";
        text += tag_open;
    }
    if (b_open) {
        text += tag_close;
        text += "**";
        text += tag_open;
    }

    return text;
}
