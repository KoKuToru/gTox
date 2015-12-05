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
#include "widget/chat_action.h"
#include "widget/chat_file.h"
#include "tox/contact/file/file.h"
#include "tox/contact/manager.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "tox/contact/call.h"
#include "widget/imagescaled.h"
#include "tox/exception.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace dialog;

chat::chat(std::shared_ptr<toxmm::core> core,
           std::shared_ptr<toxmm::contact> contact,
           std::shared_ptr<class config> config,
           detachable_window::type_slot_detachable_add slot_add_widget,
           detachable_window::type_slot_detachable_add slot_del_widget):
    Glib::ObjectBase(typeid(chat)),
    detachable_window(slot_add_widget, slot_del_widget),
    m_core(core),
    m_contact(contact),
    m_config(config) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { contact->property_name_or_addr().get_value().raw() });

    utils::builder builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_chat.ui"));

    builder.get_widget("chat_body", m_body);
    m_input = builder.get_widget_derived<widget::chat_input>("chat_input");
    builder.get_widget("chat_input_revealer", m_input_revealer);
    builder.get_widget("chat_input_format_revealer", m_input_format_revealer);
    builder.get_widget("chat_box", m_chat_box);
    builder.get_widget("eventbox", m_eventbox);
    builder.get_widget("scrolled", m_scrolled);
    builder.get_widget("viewport", m_viewport);
    builder.get_widget("av_call", m_av_call);
    builder.get_widget("headerbar_buttons", m_headerbar_buttons);

    m_image_webcam_local  = builder.get_widget_derived<widget::imagescaled>("image10");
    m_image_webcam_remote = builder.get_widget_derived<widget::imagescaled>("image_webcam_remote");

    property_body() = m_body;
    property_headerbar().get_value()->pack_end(*m_headerbar_buttons);

    m_binding_name = Glib::Binding::bind_property(m_contact->property_name_or_addr(),
                                                  property_headerbar_title(),
                                                  Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_status = Glib::Binding::bind_property(m_contact->property_status_message(),
                                                    property_headerbar_subtitle(),
                                                    Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE);

    m_binding_online = Glib::Binding::bind_property(m_contact->property_connection(),
                                 m_input_revealer->property_reveal_child(),
                                 Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                                 [](const TOX_CONNECTION& connection, bool& is_online) {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), { connection });
        is_online = connection != TOX_CONNECTION_NONE;
        return true;
    });

    m_input->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        utils::debug::scope_log log(DBG_LVL_3("gtox"), {});
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

    m_contact->signal_send_message().connect(sigc::track_obj([this](Glib::ustring message, std::shared_ptr<toxmm::receipt>) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { message.raw() });
        auto core = m_core.lock();
        if (core) {
            auto time = Glib::DateTime::create_now_utc();
            add_chat_line(
                LINE_APPEND_APPENDABLE,
                core,
                time,
                Gtk::manage(new widget::chat_message(core->property_name_or_addr(),
                                                     time,
                                                     message)));
        }
    }, *this));

    m_contact->signal_recv_message().connect(sigc::track_obj([this](Glib::ustring message) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { message.raw() });
        auto time = Glib::DateTime::create_now_utc();
        add_chat_line(
            LINE_APPEND_APPENDABLE,
            m_contact,
            time,
            Gtk::manage(new widget::chat_message(m_contact->property_name_or_addr(),
                                                 time,
                                                 message)));
    }, *this));

    m_contact->signal_send_action().connect(sigc::track_obj([this](Glib::ustring action, std::shared_ptr<toxmm::receipt>) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { action.raw() });
        auto core = m_core.lock();
        if (core) {
            auto time = Glib::DateTime::create_now_utc();
            add_chat_line(
                LINE_APPEND_APPENDABLE,
                core,
                time,
                Gtk::manage(new widget::chat_action(core->property_name_or_addr(),
                                                    time,
                                                    action)));
        }
    }, *this));

    m_contact->signal_recv_action().connect(sigc::track_obj([this](Glib::ustring action) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), { action.raw() });
        auto time = Glib::DateTime::create_now_utc();
        add_chat_line(
            LINE_NEW,
            m_contact,
            Glib::DateTime::create_now_utc(),
            Gtk::manage(new widget::chat_action(m_contact->property_name_or_addr(),
                                                time,
                                                action)));
    }, *this));

    m_contact->file_manager()->signal_recv_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm::file>& file) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (file->property_kind() != TOX_FILE_KIND_DATA) {
            return;
        }
        auto b_ref = widget::file::create(file, m_config);
        auto widget = b_ref.raw();
        add_chat_line(
            LINE_APPEND_APPENDABLE,
            m_contact,
            Glib::DateTime::create_now_utc(),
            Gtk::manage(widget));
    }, *this));
    m_contact->file_manager()->signal_send_file().connect(sigc::track_obj([this](std::shared_ptr<toxmm::file>& file) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (file->property_kind() != TOX_FILE_KIND_DATA) {
            return;
        }
        auto core = m_core.lock();
        if (core) {
            auto b_ref = widget::file::create(file, m_config);
            auto widget = b_ref.raw();
            add_chat_line(
                LINE_APPEND_APPENDABLE,
                core,
                Glib::DateTime::create_now_utc(),
                Gtk::manage(widget));
        }
    }, *this));

    //logic for text-selection
    m_eventbox->add_events(Gdk::BUTTON_PRESS_MASK |
                           Gdk::BUTTON_RELEASE_MASK |
                           Gdk::BUTTON1_MOTION_MASK |
                           Gdk::KEY_PRESS_MASK);
    m_eventbox->set_can_focus(true);
    m_eventbox->signal_button_press_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
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

        m_eventbox->grab_focus();

        return true;
    }, *this));
    m_eventbox->signal_button_release_event().connect(sigc::track_obj([this](GdkEventButton* event) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (event->button != 1) {
            return false;
        }
        from_x = -1;
        from_y = -1;
        return true;
    }, *this));
    m_eventbox->signal_motion_notify_event().connect(sigc::track_obj([this](GdkEventMotion* event) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (from_x < 0 && from_y < 0) {
            return false;
        }
        // update all children
        update_children(event, m_chat_box->get_children());
        return true;
    }, *this));
    m_eventbox->signal_key_press_event().connect(sigc::track_obj([this](GdkEventKey* event) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
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
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        // check if lowest position
        auto adj = m_scrolled->get_vadjustment();
        m_autoscroll = adj->get_upper() - adj->get_page_size()
                       == adj->get_value();
    }, *this));
    m_chat_box->signal_size_allocate()
            .connect_notify(sigc::track_obj([this](Gtk::Allocation&) {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
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
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
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

    // av support
    m_av_call->signal_clicked().connect(sigc::track_obj([this]() {
        auto ct = m_contact;
        std::clog << "call the contact" << std::endl;
        ct->call()->property_state() = toxmm::call::CALL_RESUME;
    }, *this));

    m_contact->call()->property_state().signal_changed().connect(sigc::track_obj([this]() {
        std::clog << "call state changed to ";
        switch (m_contact->call()->property_state().get_value()) {
            case toxmm::call::CALL_RESUME:
                std::clog << " RESUME" << std::endl;
                break;
            case toxmm::call::CALL_PAUSE:
                std::clog << " PAUSE" << std::endl;
                break;
            case toxmm::call::CALL_CANCEL:
                std::clog << " CANCEL" << std::endl;
                break;
        }
    }, *this));

    m_contact->call()->property_remote_state().signal_changed().connect(sigc::track_obj([this]() {
        std::clog << "remote call state changed to ";
        switch (m_contact->call()->property_remote_state().get_value()) {
            case toxmm::call::CALL_RESUME:
                std::clog << " RESUME" << std::endl;
                break;
            case toxmm::call::CALL_PAUSE:
                std::clog << " PAUSE" << std::endl;
                break;
            case toxmm::call::CALL_CANCEL:
                std::clog << " CANCEL" << std::endl;
                break;
        }
    }, *this));

    auto update_webcam_state = sigc::track_obj([this]() {
        std::clog << "update webcame state" << std::endl;
        auto remote_state = m_contact->call()->property_remote_state().get_value();
        auto state = m_contact->call()->property_state().get_value();

        if (remote_state == toxmm::call::CALL_CANCEL  ||
            state == toxmm::call::CALL_CANCEL) {
            std::clog << "stop webcam" << std::endl;
            m_webcam.property_state() = Gst::STATE_NULL;
            return;
        }
        std::clog << "start webcam" << std::endl;
        m_webcam.property_state() = Gst::STATE_PLAYING;
    }, *this);

    m_webcam.property_device() = m_webcam.get_webcam_devices().at(1);
    m_webcam.signal_error().connect([](Glib::ustring msg) {
        std::clog << "WEBCAM ERROR ! " << msg << std::endl;
    });

    m_contact->call()->property_state().signal_changed().connect(update_webcam_state);
    m_contact->call()->property_remote_state().signal_changed().connect(update_webcam_state);

    m_contact->call()->signal_incoming_call().connect(sigc::track_obj([this]() {
        std::clog << "new incoming call" << std::endl;
    }, *this));

    m_contact->call()->signal_finish().connect(sigc::track_obj([this]() {
        std::clog << "call ended" << std::endl;
    }, *this));

    m_contact->call()->signal_suggestion_updated().connect(sigc::track_obj([this]() {
        std::clog << "suggested bitrate updated to V: " <<
                     m_contact->call()->property_suggested_video_kilobitrate() <<
                     " A: " <<
                     m_contact->call()->property_suggested_audio_kilobitrate() <<
                     std::endl;
    }, *this));

    m_webcam_bind_preview = Glib::Binding::bind_property(m_webcam.property_pixbuf(),
                                                         m_contact->call()->property_video_frame(),
                                                         Glib::BINDING_DEFAULT,
                                                         [this](const Glib::RefPtr<Gdk::Pixbuf>& in, toxmm::av::image& out) {
        std::clog << "got a frame for sending" << std::endl;
        if (in) {
            int n = in->property_n_channels();
            uint8_t* pixels = in->get_pixels();

            std::clog << in->get_width() << "x" << in->get_height() << std::endl;
            size_t size = in->get_width() * in->get_height();
            out = toxmm::av::image(in->get_width(),
                                   in->get_height());
            for (size_t i = 0; i < size; ++i) {
                out.data()[i] = toxmm::av::pixel(
                                    pixels[i*n + 0],
                                    pixels[i*n + 1],
                                    pixels[i*n + 2]);
            }
        }
        return true;
    });

    m_webcam_bind_preview_2 = Glib::Binding::bind_property(m_webcam.property_pixbuf(),
                                                           m_image_webcam_local->property_pixbuf(),
                                                           Glib::BINDING_DEFAULT);

    m_webcam_bind_preview_test = Glib::Binding::bind_property(m_contact->call()->property_remote_video_frame(),
                                                              m_image_webcam_remote->property_pixbuf(),
                                                              Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                                                              [](const toxmm::av::image& in, Glib::RefPtr<Gdk::Pixbuf>& out) {
        if (in.size() > 0) {
            auto mem = new uint8_t[in.size() * 3];
            for (size_t i = 0; i < in.size(); ++i) {
                mem[i*3 + 0] = in.data()[i].red();
                mem[i*3 + 1] = in.data()[i].green();
                mem[i*3 + 2] = in.data()[i].blue();
            }
            out = Gdk::Pixbuf::create_from_data(mem,
                                                Gdk::COLORSPACE_RGB,
                                                false,
                                                8,
                                                in.width(),
                                                in.height(),
                                                in.width() * 3,
                                                [](const guint8* data) {
                delete[] data;
            });
        }
        return true;
    });

    load_log();
}

chat::~chat() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    delete m_body;
}

void chat::update_children(GdkEventMotion* event,
                           std::vector<Gtk::Widget*> children) {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
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
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
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

void chat::load_log() {
    utils::debug::scope_log lo(DBG_LVL_1("gtox"), {});
    auto c = m_core.lock();
    if (!c) {
        return;
    }
    auto cm = c->contact_manager();
    if (!cm) {
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

        if (!last_data || date != last_date) {
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
        auto time = Glib::DateTime::create_now_utc(item->timestamp());
        auto sender = std::string(item->sender()->c_str());
        switch (item->data_type()) {
            case flatbuffers::Log::Data::Message: {
                auto f = reinterpret_cast<const flatbuffers::Log::Message*>(
                             item->data());
                auto contact = cm->find(toxmm::contactAddrPublic(
                                            sender));
                auto message = std::string(f->message()->c_str());
                if (contact) {
                    add_chat_line(LINE_APPEND_APPENDABLE,
                                  contact,
                                  time,
                                  Gtk::manage(new widget::chat_message(
                                                  contact->property_name_or_addr(),
                                                  time,
                                                  message)));
                } else if (c->property_addr_public().get_value() == sender) {
                    add_chat_line(LINE_APPEND_APPENDABLE,
                                  c,
                                  time,
                                  Gtk::manage(new widget::chat_message(
                                                  c->property_name_or_addr(),
                                                  time,
                                                  message)));
                } else {
                    //not found
                    //TODO: will probably need this for group chat
                }
            } break;
            case flatbuffers::Log::Data::Action: {
                auto f = reinterpret_cast<const flatbuffers::Log::Action*>(
                             item->data());
                auto contact = cm->find(toxmm::contactAddrPublic(sender));
                auto action = std::string(f->action()->begin(),
                                          f->action()->end());
                if (contact) {
                    add_chat_line(LINE_NEW,
                                  contact,
                                  time,
                                  Gtk::manage(new widget::chat_action(
                                                  contact->property_name_or_addr(),
                                                  time,
                                                  action)));
                } else if (c->property_addr_public().get_value() == sender) {
                    add_chat_line(LINE_NEW,
                                  c,
                                  time,
                                  Gtk::manage(new widget::chat_action(
                                                  c->property_name_or_addr(),
                                                  time,
                                                  action)));
                } else {
                    //not found
                    //TODO: will probably need this for group chat
                }
            } break;
            case flatbuffers::Log::Data::File: {
                auto f = reinterpret_cast<const flatbuffers::Log::File*>(
                             item->data());
                auto contact = cm->find(toxmm::contactAddrPublic(sender));
                auto receiver = std::string(f->receiver()->c_str());
                if (!contact) {
                    contact = cm->find(toxmm::contactAddrPublic(receiver));
                }
                if (contact) {
                    //search the file
                    auto fm = contact->file_manager();
                    if (!fm) {
                        break;
                    }

                    auto file = fm->find(toxmm::uniqueId(std::string(f->uuid()->c_str())));

                    auto b_ref = file
                                 ? widget::file::create(file, m_config)
                                 : widget::file::create(f->path()->c_str(), m_config);

                    auto widget = Gtk::manage(b_ref.raw());

                    if (sender != std::string(c->property_addr_public().get_value())) {
                        add_chat_line(LINE_APPEND_APPENDABLE,
                                      contact,
                                      time,
                                      Gtk::manage(widget));
                    } else {
                        add_chat_line(LINE_APPEND_APPENDABLE,
                                      c,
                                      time,
                                      Gtk::manage(widget));
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

void chat::add_chat_line(AppendMode append_mode,
                   std::shared_ptr<toxmm::contact> contact,
                   Glib::DateTime time,
                   Gtk::Widget* widget) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    append_mode,
                                    contact->property_name_or_addr().get_value().raw(),
                                    time.format("%c").raw()
                                });
    auto side = SIDE::NONE;
    bool append = append_mode == LINE_APPEND_APPENDABLE || append_mode == LINE_APPEND;
    if (append) {
        side = SIDE::OTHER;
    }

    auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);

    //check if same day
    if (append  && bubble != nullptr && m_last_bubble.side == side) {
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
        }
    }

    //need a new bubble
    bool appendable = append_mode == LINE_APPEND_APPENDABLE || append_mode == LINE_NEW_APPENDABLE;
    auto bubble_builder  = widget::chat_bubble::create(contact, time);
    bubble               = bubble_builder.raw();
    m_last_bubble.side   = appendable?side:SIDE::NONE;
    m_last_bubble.widget = Gtk::manage(bubble);
    m_last_bubble.time   = time;

    bubble->add_row(*widget);
    m_chat_box->add(*m_last_bubble.widget);
}

void chat::add_chat_line(AppendMode append_mode,
                   std::shared_ptr<toxmm::core> contact,
                   Glib::DateTime time,
                   Gtk::Widget* widget) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    append_mode,
                                    contact->property_name_or_addr().get_value().raw(),
                                    time.format("%c").raw()
                                });
    auto side = SIDE::NONE;
    bool append = append_mode == LINE_APPEND_APPENDABLE || append_mode == LINE_APPEND;
    if (append) {
        side = SIDE::OWN;
    }

    auto bubble = dynamic_cast<widget::chat_bubble*>(m_last_bubble.widget);

    //check if same day
    if (append  && bubble != nullptr && m_last_bubble.side == side) {
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
        }
    }

    //need a new bubble
    bool appendable = append_mode == LINE_APPEND_APPENDABLE || append_mode == LINE_NEW_APPENDABLE;
    auto bubble_builder  = widget::chat_bubble::create(contact, time);
    bubble               = bubble_builder.raw();
    m_last_bubble.side   = appendable?side:SIDE::NONE;
    m_last_bubble.widget = Gtk::manage(bubble);
    m_last_bubble.time   = time;

    bubble->add_row(*widget);
    m_chat_box->add(*m_last_bubble.widget);
}

void chat::add_log(std::shared_ptr<toxmm::storage> storage,
                   std::shared_ptr<toxmm::contact> contact,
                   std::function<flatbuffers::Offset<flatbuffers::Log::Item>(flatbuffers::FlatBufferBuilder&)> create_func) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                    contact->property_name_or_addr().get_value().raw(),
                                });
    if (!storage || !contact) {
        return;
    }

    auto  date = Glib::DateTime::create_now_utc();

    std::initializer_list<std::string> key = {
        contact->property_addr_public().get_value(),
        "log",
        date.format("%Y-%m-%d")
    };

    //try to load old log
    std::vector<uint8_t> content;
    storage->load(key, content);

    if (content.empty()) {
        //create empty collection
        flatbuffers::FlatBufferBuilder fbb;
        flatbuffers::Log::FinishCollectionBuffer(
                    fbb,
                    flatbuffers::Log::CreateCollection(
                        fbb,
                        fbb.CreateVector(std::vector<flatbuffers::Offset<flatbuffers::Log::Item>>())));

        content.clear();
        content.insert(content.end(),
                       fbb.GetBufferPointer(),
                       fbb.GetBufferPointer() + fbb.GetSize());
    }

    auto verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::Log::VerifyCollectionBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Log::VerifyCollectionBuffer failed");
    }
    auto data = flatbuffers::Log::GetMutableCollection(content.data());

    auto schema_binary = Gio::Resource::lookup_data_global("/org/gtox/flatbuffers/Log.bfbs");
    gsize schmea_binary_size;
    auto schema_binary_ptr = schema_binary->get_data(schmea_binary_size);
    auto& schema = *reflection::GetSchema(schema_binary_ptr); //doesn't this leak memory ?

    flatbuffers::ResizeVector<flatbuffers::Offset<flatbuffers::Log::Item>>(
                schema,
                data->items()->size() + 1,
                0,
                data->items(),
                &content);

    //reload (otherwise it will SEGFAULT)
    data = flatbuffers::Log::GetMutableCollection(content.data());

    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(create_func(fbb));
    auto new_item = flatbuffers::AddFlatBuffer(content,
                                               fbb.GetBufferPointer(),
                                               fbb.GetSize());

    //reload (otherwise it will SEGFAULT)
    data = flatbuffers::Log::GetMutableCollection(content.data());

    data->mutable_items()->MutateOffset(
                data->items()->size() - 1,
                new_item);

    verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::Log::VerifyCollectionBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Log::VerifyCollectionBuffer failed");
    }
    storage->save(key, content);
}
