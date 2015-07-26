/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
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
#include "chat.h"
#include "main.h"
#include "tox/contact/contact.h"
#include "tox/contact/file/manager.h"
#include "widget/chat_input.h"
#include "widget/chat_bubble.h"
#include "widget/chat_message.h"
#include "widget/chat_file.h"
#include "tox/contact/file/file.h"
#include "tox/contact/manager.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace dialog;

chat::chat(Glib::RefPtr<dialog::main> main, std::shared_ptr<toxmm2::contact> contact):
    m_contact(contact),
    m_main(main),
    m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_chat.ui")) {

    m_builder.get_widget("chat_headerbar_attached", m_headerbar_attached);
    m_builder.get_widget("chat_headerbar_detached", m_headerbar_detached);
    m_builder.get_widget("chat_body", m_body);
    m_input = m_builder.get_widget_derived<widget::chat_input>("chat_input");
    m_builder.get_widget("chat_input_revealer", m_input_revealer);
    m_builder.get_widget("chat_input_format_revealer", m_input_format_revealer);
    m_builder.get_widget("btn_prev", m_btn_prev);
    m_builder.get_widget("btn_next", m_btn_next);
    m_builder.get_widget("chat_attach", m_btn_attach);
    m_builder.get_widget("chat_detach", m_btn_detach);
    m_builder.get_widget("close_btn_attached", m_btn_close_attached);
    m_builder.get_widget("close_btn_detached", m_btn_close_detached);
    m_builder.get_widget("chat_box", m_chat_box);
    m_builder.get_widget("eventbox", m_eventbox);
    m_builder.get_widget("scrolled", m_scrolled);
    m_builder.get_widget("viewport", m_viewport);

    m_main->chat_add(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);

    m_binding_name[0] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_attached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);
    m_binding_name[1] = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                     m_headerbar_detached->property_title(),
                                                     Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_status[0] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                       m_headerbar_attached->property_subtitle(),
                                                       Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);
    m_binding_status[1] = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                       m_headerbar_detached->property_subtitle(),
                                                       Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_online = Glib::Binding::bind_property(m_contact->property_connection(),
                                 m_input_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                                 [](const TOX_CONNECTION& connection, bool& is_online) {
        is_online = connection != TOX_CONNECTION_NONE;
        return true;
    });

    /*m_binding_focus = Glib::Binding::bind_property(m_input->property_has_focus(),
                                 m_input_format_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);*/

    set_titlebar(*m_headerbar_detached);
    m_btn_detach->signal_clicked().connect(sigc::track_obj([this]() {
        //TODO: take position w/h from main-window
        m_main->chat_remove(*m_headerbar_attached, *m_body);
        add(*m_body);
        show();
    }, *this));
    m_btn_attach->signal_clicked().connect(sigc::track_obj([this]() {
        remove();
        hide();
        m_main->chat_add(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);
    }, *this));
    m_btn_close_attached->signal_clicked().connect(sigc::track_obj([this]() {
        m_main->chat_remove(*m_headerbar_attached, *m_body);
    }, *this));
    m_btn_close_detached->signal_clicked().connect(sigc::track_obj([this]() {
        remove();
        hide();
    }, *this));

    m_input->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        auto text_buffer = m_input->get_buffer();
        if (event->keyval == GDK_KEY_Return && !(event->state & GDK_SHIFT_MASK)) {
            if (text_buffer->begin() != text_buffer->end()) {
                 bool allow_send = text_buffer->get_text().find_first_not_of(" \t\n\v\f\r") != std::string::npos;
                 if (allow_send) {
                     if (Glib::str_has_prefix(text_buffer->get_text(), "/me ")) {
                         m_contact->send_action(m_input->get_serialized_text().substr(4));
                     } else {
                         m_contact->send_message(m_input->get_serialized_text());
                     }
                     text_buffer->set_text("");
                     return true;
                 }
            }
        }
        return false;
    }, *this), false);

    m_contact->signal_send_message().connect(sigc::track_obj([this](Glib::ustring message, std::shared_ptr<toxmm2::receipt>) {
        add_chat_line(
            true,
            m_main->tox(),
            Glib::DateTime::create_now_utc(),
            Gtk::manage(new widget::chat_message(message)));
    }, *this));

    m_contact->signal_recv_message().connect(sigc::track_obj([this](Glib::ustring message) {
        add_chat_line(
            true,
            m_contact,
            Glib::DateTime::create_now_utc(),
            Gtk::manage(new widget::chat_message(message)));
    }, *this));

    m_contact->signal_send_action().connect(sigc::track_obj([this](Glib::ustring action, std::shared_ptr<toxmm2::receipt>) {
        add_chat_line(
            false,
            m_main->tox(),
            Glib::DateTime::create_now_utc(),
            Gtk::manage(new widget::chat_message(m_main->tox()->property_name_or_addr() + " " + action)));
    }, *this));

    m_contact->signal_recv_action().connect(sigc::track_obj([this](Glib::ustring action) {
        add_chat_line(
            false,
            m_contact,
            Glib::DateTime::create_now_utc(),
            Gtk::manage(new widget::chat_message(m_contact->property_name_or_addr() + " " + action)));
    }, *this));

    m_contact->file_manager()->signal_recv_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm2::file>& file) {
        if (file->property_kind() != TOX_FILE_KIND_DATA) {
            return;
        }
        auto b_ref = widget::file::create(file);
        auto widget = b_ref.raw();
        add_chat_line(
            true,
            m_contact,
            Glib::DateTime::create_now_utc(),
            Gtk::manage(widget));
    }, *this));
    m_contact->file_manager()->signal_send_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm2::file>& file) {
        if (file->property_kind() != TOX_FILE_KIND_DATA) {
            return;
        }
        auto b_ref = widget::file::create(file);
        auto widget = b_ref.raw();
        add_chat_line(
            true,
            m_main->tox(),
            Glib::DateTime::create_now_utc(),
            Gtk::manage(widget));
    }, *this));

    //logic for text-selection
    m_eventbox->add_events(Gdk::BUTTON_PRESS_MASK |
                           Gdk::BUTTON_RELEASE_MASK |
                           Gdk::BUTTON1_MOTION_MASK |
                           Gdk::KEY_PRESS_MASK);
    m_eventbox->set_can_focus(true);
    m_eventbox->signal_button_press_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        if (event->button != 1) {
            return false;
        }

        from_x = event->x;
        from_y = event->y;

        // update all children, reset selection
        GdkEventMotion dummy_event;
        dummy_event.x = from_x;
        dummy_event.y = from_y;
        update_children(&dummy_event, m_chat_box->get_children());

        grab_focus();

        return true;
    }, *this));
    m_eventbox->signal_button_release_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        if (event->button != 1) {
            return false;
        }
        from_x = -1;
        from_y = -1;
        return true;
    }, *this));
    m_eventbox->signal_motion_notify_event().connect(sigc::track_obj([this](GdkEventMotion* event) {
        if (from_x < 0 && from_y < 0) {
            return false;
        }
        // update all children
        update_children(event, m_chat_box->get_children());
        return true;
    }, *this));
    m_eventbox->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        if (!(event->state & GDK_CONTROL_MASK)) {
            return false;
        }
        if (event->keyval != 'c') {
            return false;
        }

        // copy to clipboard
        auto data = get_children_selection(m_chat_box->get_children());
        Gtk::Clipboard::get()->set_text(data);
        return true;
    }, *this));

    //auto scroll
    m_scrolled->get_vadjustment()->signal_value_changed()
            .connect_notify(sigc::track_obj([this]() {
        // check if lowest position
        auto adj = m_scrolled->get_vadjustment();
        m_autoscroll = adj->get_upper() - adj->get_page_size()
                       == adj->get_value();
    }, *this));
    m_chat_box->signal_size_allocate()
            .connect_notify(sigc::track_obj([this](Gtk::Allocation&) {
        // auto scroll:
        if (m_autoscroll) {
            auto adj = m_scrolled->get_vadjustment();
            adj->set_value(adj->get_upper() - adj->get_page_size());
        }
    }, *this));
    m_scrolled->signal_size_allocate()
            .connect_notify(sigc::track_obj([this](Gtk::Allocation&) {
        // auto scroll:
        if (m_autoscroll) {
            auto adj = m_scrolled->get_vadjustment();
            adj->set_value(adj->get_upper() - adj->get_page_size());
        }
    }, *this));

    // Disable auto scroll to focused child
    auto dummy_adj = Gtk::Adjustment::create(0, 0, 0);
    m_viewport->set_focus_hadjustment(dummy_adj);
    m_viewport->set_focus_vadjustment(dummy_adj);

    //drag and drop
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("text/uri-list"));
    m_scrolled->drag_dest_set(targets,
                              Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP,
                              Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
    m_scrolled->signal_drag_data_received().connect(sigc::track_obj([this](const Glib::RefPtr<Gdk::DragContext>&, int, int, const Gtk::SelectionData& data, guint, guint) {
        auto ct = m_contact; //.lock();
        auto fmng = ct->file_manager();
        if (!ct | !fmng) {
            return;
        }
        if (ct->property_connection() == TOX_CONNECTION_NONE) {
            //TODO: error message, user offline
            return;
        }
        for (auto uri : data.get_uris()) {
            //do something with the files
            fmng->send_file(Glib::filename_from_uri(uri));
        }
    }, *this));
}

chat::~chat() {
    m_main->chat_remove(*m_headerbar_attached, *m_body);
}

void chat::activated() {
    if (is_visible()) {
        present();
    } else {
        m_main->chat_show(*m_headerbar_attached, *m_body, *m_btn_prev, *m_btn_next);
    }
}

void chat::update_children(GdkEventMotion* event,
                           std::vector<Gtk::Widget*> children) {
    for (auto c : children) {
        // check if it's a container
        auto c_container = dynamic_cast<Gtk::Container*>(c);
        if (c_container) {
            update_children(event, c_container->get_children());
            continue;
        }
        // check if it's WidgetChatMessage
        auto c_message = dynamic_cast<widget::label*>(c);
        if (!c_message) {
            //
            continue;
        }
        // correct x,y to widget x,y
        int w_from_x, w_from_y, w_to_x, w_to_y;
        int to_x = event->x;
        int to_y = event->y;
        if (m_chat_box->translate_coordinates(*c_message, from_x, from_y, w_from_x, w_from_y) &&
            m_chat_box->translate_coordinates(*c_message, to_x, to_y, w_to_x, w_to_y)) {

            // fix order
            if (w_to_y < w_from_y) {
                std::swap(w_to_y, w_from_y);
                std::swap(w_to_x, w_from_x);
            }

            // check if within
            if (w_from_y < c_message->get_allocated_height() && w_to_y > 0) {
                c_message->on_selection(w_from_x, w_from_y, w_to_x, w_to_y);
            } else {
                c_message->on_selection(0, 0, 0, 0);
            }
        }
    }
}

Glib::ustring chat::get_children_selection(
    std::vector<Gtk::Widget*> children) {
    Glib::ustring res;
    Glib::ustring tmp;
    for (auto c : children) {
        tmp.clear();

        // check if it's a container
        auto c_container = dynamic_cast<Gtk::Container*>(c);
        if (c_container) {
            tmp = get_children_selection(c_container->get_children());
        }
        // check if it's WidgetChatMessage
        auto c_message = dynamic_cast<widget::label*>(c);
        if (c_message) {
            tmp = c_message->get_selection();
        }

        // add to result
        if (!tmp.empty()) {
            if (!res.empty()) {
                res += '\n';
            }
            res += tmp;
        }
    }
    return res;
}

void chat::save_log() {
    auto c = m_main->tox();
    if (!c) {
        return;
    }

    std::vector<uint8_t> content;
    for (auto pending: pending_log) {
        auto  date  = pending.first;
        auto& fbb   = pending.second->first;
        auto& items = pending.second->second;

        std::initializer_list<std::string> key = {
            m_contact->property_addr_public().get_value(),
            "log",
            date.format_string("%Y-%m-%d")
        };

        //try to load old log
        content.clear();
        c->storage()->load(key, content);

        if (!content.empty()) {
            auto verify = flatbuffers::Verifier(content.data(), content.size());
            if (!flatbuffers::Log::VerifyCollectionBuffer(verify)) {
                throw std::runtime_error("flatbuffers::Log::VerifyCollectionBuffer failed");
            }
            auto data = flatbuffers::Log::GetCollection(content.data());
            //add all log entrys to the pending one
            typename std::remove_reference<decltype(items)>::type full_log;
            for (auto item : *data->items()) {
                flatbuffers::Offset<void> subdata;
                //is there a more efficent way to copy a flatbuffer directly ?
                switch (item->data_type()) {
                    case flatbuffers::Log::Data_Message: {
                        auto f = reinterpret_cast<const flatbuffers::Log::Message*>(item->data());
                        subdata = flatbuffers::Log::CreateMessage(
                                      fbb,
                                      fbb.CreateString(f->message()),
                                      f->status()).Union();
                    } break;
                    case flatbuffers::Log::Data_Action: {
                        auto f = reinterpret_cast<const flatbuffers::Log::Action*>(item->data());
                        subdata = flatbuffers::Log::CreateAction(
                                      fbb,
                                      fbb.CreateString(f->action()),
                                      f->status()).Union();
                    } break;
                    case flatbuffers::Log::Data_File: {
                        auto f = reinterpret_cast<const flatbuffers::Log::File*>(item->data());
                        subdata = flatbuffers::Log::CreateFile(
                                      fbb,
                                      fbb.CreateString(f->uuid()),
                                      fbb.CreateString(f->name()),
                                      fbb.CreateString(f->path()),
                                      f->status(),
                                      fbb.CreateString(f->receiver())).Union();
                    } break;
                    default:
                        //TODO: What should we do ?
                        break;
                }
                auto new_item = flatbuffers::Log::CreateItem(
                                    fbb,
                                    fbb.CreateString(item->sender()),
                                    item->timestamp(),
                                    item->data_type(),
                                    subdata);
                full_log.push_back(new_item);
            }
            full_log.insert(full_log.end(), items.begin(), items.end());
            items = full_log;
        }

        flatbuffers::Log::FinishCollectionBuffer(
                    fbb,
                    flatbuffers::Log::CreateCollection(
                        fbb,
                        fbb.CreateVector(items)));

        content.clear();
        content.insert(content.end(),
                       fbb.GetBufferPointer(),
                       fbb.GetBufferPointer() + fbb.GetSize());
        c->storage()->save(key, content);
    }

    //remove pending items
    pending_log.clear();
}

void chat::load_log() {
    auto c = m_main->tox();
    auto cm = c->contact_manager();
    if (!c || !cm) {
        return;
    }

    std::vector<uint8_t> content;

    //TODO: storage will need a way to list keys..

    //search logs to load..
    std::deque<std::pair<Glib::Date, int>> log;
    constexpr int max_lines = 100;
    for (int d = 0; d <= 10 && log.size() < max_lines; ++d) {
        auto date = Glib::Date();
        date.set_time_current();
        date.subtract_days(d);

        std::initializer_list<std::string> key = {
            m_contact->property_addr_public().get_value(),
            "log",
            date.format_string("%Y-%m-%d")
        };

        content.clear();
        c->storage()->load(key, content);

        if (content.empty()) {
            continue;
        }

        auto verify = flatbuffers::Verifier(content.data(), content.size());
        if (!flatbuffers::Log::VerifyCollectionBuffer(verify)) {
            throw std::runtime_error("flatbuffers::Log::VerifyCollectionBuffer failed");
        }
        auto data = flatbuffers::Log::GetCollection(content.data());

        //add all messages for this day to the chat
        for (int i = data->items()->size(); i > 0 ; --i) {
            auto index = i - 1;
            log.push_front({date, index});
            if (log.size() >= max_lines) {
                break;
            }
        }
    }

    //load the selected lines
    Glib::Date last_date;
    const flatbuffers::Log::Collection* last_data = nullptr;

    for (auto l : log) {
        auto& date  = l.first;
        auto& index = l.second;

        if (date != last_date) {
            last_date = date;

            std::initializer_list<std::string> key = {
                m_contact->property_addr_public().get_value(),
                "log",
                date.format_string("%Y-%m-%d")
            };

            content.clear();
            c->storage()->load(key, content);

            if (content.empty()) {
                throw std::runtime_error("Log content is empty now..");
            }

            auto verify = flatbuffers::Verifier(content.data(), content.size());
            if (!flatbuffers::Log::VerifyCollectionBuffer(verify)) {
                throw std::runtime_error("flatbuffers::Log::VerifyCollectionBuffer failed");
            }

            last_data = flatbuffers::Log::GetCollection(content.data());
        }

        auto item = last_data->items()->Get(index);

        //add message to chat
        switch (item->data_type()) {
            case flatbuffers::Log::Data_Message: {
                auto f = reinterpret_cast<const flatbuffers::Log::Message*>(
                             item->data());
                auto contact = cm->find(toxmm2::contactAddrPublic(
                                            item->sender()->str()));
                auto widget = new widget::chat_message(
                                  f->message()->str());
                if (contact) {
                    add_chat_line(true,
                                  contact,
                                  Glib::DateTime::create_now_utc(
                                      item->timestamp()),
                                  Gtk::manage(widget));
                } else if (c->property_addr_public().get_value()
                           == item->sender()->str()) {
                    add_chat_line(true,
                                  c,
                                  Glib::DateTime::create_now_utc(
                                      item->timestamp()),
                                  Gtk::manage(widget));
                } else {
                    //not found
                    //TODO: will probably need this for group chat
                }
            } break;
            case flatbuffers::Log::Data_Action: {
                auto f = reinterpret_cast<const flatbuffers::Log::Action*>(
                             item->data());
                auto contact = cm->find(toxmm2::contactAddrPublic(
                                            item->sender()->str()));
                auto widget = new widget::chat_message(
                                  contact->property_name_or_addr() + " " +
                                  f->action()->str());
                if (contact) {
                    add_chat_line(false,
                                  contact,
                                  Glib::DateTime::create_now_utc(
                                      item->timestamp()),
                                  Gtk::manage(widget));
                } else if (c->property_addr_public().get_value()
                                 == item->sender()->str()) {
                    add_chat_line(false,
                                  c,
                                  Glib::DateTime::create_now_utc(
                                      item->timestamp()),
                                  Gtk::manage(widget));
                } else {
                    //not found
                    //TODO: will probably need this for group chat
                }
            } break;
            case flatbuffers::Log::Data_File: {
                auto f = reinterpret_cast<const flatbuffers::Log::File*>(
                             item->data());
                auto contact = cm->find(toxmm2::contactAddrPublic(
                                            item->sender()->str()));
                if (!contact) {
                    contact = cm->find(toxmm2::contactAddrPublic(
                                           f->receiver()->str()));
                }
                if (contact) {
                    //search the file
                    auto fm = contact->file_manager();
                    if (!fm) {
                        break;
                    }

                    auto file = fm->find(toxmm2::uniqueId(f->uuid()->str()));

                    if (file) {
                        auto b_ref = widget::file::create(file);
                        auto widget = Gtk::manage(b_ref.raw());

                        add_chat_line(true,
                                      contact,
                                      Glib::DateTime::create_now_utc(
                                          item->timestamp()),
                                      Gtk::manage(widget));
                    } else {
                        //TODO: file preview only
                    }
                } else {
                    //not found
                }
            } break;
            default:
                //TODO: What should we do ?
                break;
        }
    }
}

void chat::add_chat_line(bool append_bubble,
                   std::shared_ptr<toxmm2::contact> contact,
                   Glib::DateTime time,
                   Gtk::Widget* widget) {

    auto side = SIDE::NONE;
    if (append_bubble) {
        side = SIDE::OTHER;
    }

    auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);

    //check if same day
    if (append_bubble  && bubble != nullptr && m_last_bubble.side == side) {
        auto a_time = m_last_bubble.time;
        auto b_time = time;

        bool same_day = a_time.get_year() == b_time.get_year() &&
                        a_time.get_month() == b_time.get_month() &&
                        a_time.get_day_of_month() == b_time.get_day_of_month();

        if (same_day) {
            //check if same time
            bool same_time = a_time.get_hour() == b_time.get_hour() &&
                             a_time.get_minute() == b_time.get_minute();

            if (same_time) {
                bubble->add_row(*widget);
                return;
            }
        } else {
            //TODO: display date line
        }
    }

    //need a new bubble
    auto bubble_builder  = widget::chat_bubble::create(contact);
    bubble               = bubble_builder.raw();
    m_last_bubble.side   = SIDE::OTHER;
    m_last_bubble.widget = Gtk::manage(bubble);
    m_last_bubble.time   = time;

    bubble->add_row(*widget);
    m_chat_box->add(*m_last_bubble.widget);
}

void chat::add_chat_line(bool append_bubble,
                   std::shared_ptr<toxmm2::core> contact,
                   Glib::DateTime time,
                   Gtk::Widget* widget) {

    auto side = SIDE::NONE;
    if (append_bubble) {
        side = SIDE::OTHER;
    }

    auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);

    //check if same day
    if (append_bubble  && bubble != nullptr && m_last_bubble.side == side) {
        auto a_time = m_last_bubble.time;
        auto b_time = time;

        bool same_day = a_time.get_year() != b_time.get_year() ||
                        a_time.get_month() != b_time.get_month() ||
                        a_time.get_day_of_month() != b_time.get_day_of_month();

        if (same_day) {

            //check if same time
            bool same_time = a_time.get_hour() != b_time.get_hour() ||
                             a_time.get_minute() != b_time.get_minute();

            if (same_time) {
                bubble->add_row(*widget);
                return;
            }
        } else {
            //TODO: display date line
        }
    }

    //need a new bubble
    auto bubble_builder  = widget::chat_bubble::create(contact);
    bubble               = bubble_builder.raw();
    m_last_bubble.side   = side;
    m_last_bubble.widget = Gtk::manage(bubble);
    m_last_bubble.time   = time;

    bubble->add_row(*widget);
    m_chat_box->add(*m_last_bubble.widget);
}
