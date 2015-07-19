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

namespace widget {
    class file: public Gtk::Frame {
        private:
            std::shared_ptr<toxmm2::file> m_file;
            utils::dispatcher m_dispatcher;

            Gtk::ToggleButton* m_file_resume;
            Gtk::ToggleButton* m_file_cancel;
            Gtk::ToggleButton* m_file_pause;
            Gtk::ProgressBar*  m_file_progress;
            Gtk::Revealer*     m_revealer_download;
            Gtk::Spinner*      m_spinner;
            Gtk::Widget*       m_file_open_bar;
            Gtk::Label*        m_file_speed;
            Gtk::Label*        m_file_size;
            Gtk::Label*        m_file_time;
            Gtk::Label*        m_file_name;
            Gtk::Box*          m_widget_list;
            Gtk::Widget*       m_file_info_box_1;
            Gtk::Widget*       m_file_info_box_2;
            Gtk::Button*       m_file_dir;
            Gtk::Button*       m_file_open;

            std::vector<Glib::RefPtr<Glib::Binding>> m_bindings;
            Glib::RefPtr<Gio::FileMonitor> m_monitor;

        public:
            file(BaseObjectType* cobject,
                 utils::builder builder,
                 const std::shared_ptr<toxmm2::file>& file);
            static utils::builder::ref<file> create(const std::shared_ptr<toxmm2::file>& file);
    };

}
#endif
