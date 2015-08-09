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
#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include "tox/types.h"
#include "tox/storage.h"
#include <memory>
#include "resources/flatbuffers/generated/Log_generated.h"

namespace widget {
    class chat_input;
}

namespace dialog {
    class main;
    class chat : public Gtk::Window {
        private:
            std::shared_ptr<toxmm::contact> m_contact;

            main& m_main;
            utils::dispatcher m_dispatcher;
            utils::builder    m_builder;

            Gtk::HeaderBar* m_headerbar_attached;
            Gtk::HeaderBar* m_headerbar_detached;
            Gtk::Widget* m_body;

            Gtk::Button* m_btn_attach;
            Gtk::Button* m_btn_detach;
            Gtk::Button* m_btn_prev;
            Gtk::Button* m_btn_next;
            Gtk::Button* m_btn_close_attached;
            Gtk::Button* m_btn_close_detached;

            Gtk::EventBox* m_eventbox;

            Gtk::ScrolledWindow* m_scrolled;
            Gtk::Viewport*       m_viewport;

            widget::chat_input* m_input;
            Gtk::Revealer* m_input_revealer;
            Gtk::Revealer* m_input_format_revealer;

            Gtk::Box* m_chat_box;

            Glib::RefPtr<Glib::Binding> m_binding_name[2];
            Glib::RefPtr<Glib::Binding> m_binding_status[2];
            Glib::RefPtr<Glib::Binding> m_binding_online;
            Glib::RefPtr<Glib::Binding> m_binding_focus;

            enum class SIDE {
                NONE,
                OWN,
                OTHER
            };

            struct {
                    uint64_t timestamp = 0;
                    Glib::DateTime time;
                    SIDE side = SIDE::NONE;
                    Gtk::Widget* widget = nullptr;

            } m_last_bubble;

            int from_x = -1;
            int from_y = -1;

            bool m_autoscroll = true;

            void update_children(GdkEventMotion* event,
                                 std::vector<Gtk::Widget*> children);
            Glib::ustring get_children_selection(std::vector<Gtk::Widget*> children);

            void load_log();

            enum AppendMode {
                //adds to previouse bubble
                LINE_APPEND_APPENDABLE,
                //adds to previouse bubble but following lines can't add to it
                LINE_APPEND,
                //start a new bubble
                LINE_NEW_APPENDABLE,
                LINE_NEW
            };

            void add_chat_line(AppendMode append_mode,
                               std::shared_ptr<toxmm::contact> contact,
                               Glib::DateTime time,
                               Gtk::Widget* widget);
            void add_chat_line(AppendMode append_mode,
                               std::shared_ptr<toxmm::core>,
                               Glib::DateTime time,
                               Gtk::Widget* widget);
        public:
            chat(main& main, std::shared_ptr<toxmm::contact> contact);
            ~chat();

            static void add_log(std::shared_ptr<toxmm::storage> storage,
                                std::shared_ptr<toxmm::contact> contact,
                                std::function<flatbuffers::Offset<flatbuffers::Log::Item>(flatbuffers::FlatBufferBuilder&)> create_func);

            void activated();
    };
}

#endif
