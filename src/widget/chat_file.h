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
#ifndef WIDGETCHATFILERECV_H
#define WIDGETCHATFILERECV_H
#include "tox/types.h"
#include "utils/dispatcher.h"
#include <gtkmm.h>
#include "utils/builder.h"
#include <vector>
#include "widget/imagescaled.h"
#include "widget/videoplayer.h"
#include <thread>
#include "utils/debug.h"
#include "widget/chat_file_popover.h"
#include "config.h"

namespace widget {
    class file: public Gtk::Frame, public utils::debug::track_obj<file> {
        private:
            std::shared_ptr<toxmm::file> m_file;
            utils::dispatcher m_dispatcher;

            Gtk::Spinner*  m_spinner;
            Gtk::Label*    m_file_name;
            Gtk::Button*   m_file_info;
            Gtk::Button*   m_file_info_2;
            Gtk::Revealer* m_preview_revealer;
            Gtk::Revealer* m_info_revealer;
            Gtk::Box*      m_preview;
            Gtk::EventBox* m_eventbox;
            Gtk::Image*    m_net_icon;

            sigc::connection m_leave_timer;

            widget::imagescaled  m_preview_image;
            widget::videoplayer* m_preview_video;

            widget::chat_file_popover m_file_info_popover;

            std::vector<Glib::RefPtr<Glib::Binding>> m_bindings;

            std::thread m_preview_thread;

            bool m_display_inline;

        public:
            file(BaseObjectType* cobject,
                 utils::builder builder,
                 const std::shared_ptr<toxmm::file>& file,
                 const std::shared_ptr<class config>& config);
            virtual ~file();
            static utils::builder::ref<file> create(const std::shared_ptr<toxmm::file>& file,
                                                    const std::shared_ptr<class config>& config);
            static utils::builder::ref<file> create(const Glib::ustring& file_path,
                                                    const std::shared_ptr<class config>& config);

        protected:
            void update_complete();
    };

}
#endif
