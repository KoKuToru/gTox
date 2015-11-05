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
#ifndef WIDGETPOPOVERFILE_H
#define WIDGETPOPOVERFILE_H

#include <gtkmm.h>
#include "utils/debug.h"
#include "tox/types.h"

namespace widget {
    class chat_file_popover: public Gtk::Popover, public utils::debug::track_obj<chat_file_popover> {
        private:
            std::shared_ptr<toxmm::file> m_file;

            Gtk::Box* m_body;
            Gtk::ToggleButton* m_file_resume;
            Gtk::ToggleButton* m_file_cancel;
            Gtk::ToggleButton* m_file_pause;
            Gtk::ProgressBar*  m_file_progress;
            Gtk::Widget*       m_file_open_bar;
            Gtk::Label*        m_file_speed;
            Gtk::Label*        m_file_size;
            Gtk::Label*        m_file_time;
            Gtk::Label*        m_file_name;
            Gtk::Widget*       m_file_control;
            Gtk::Button*       m_file_dir;
            Gtk::Button*       m_file_open;
            Gtk::Widget*       m_file_info_box_1;
            Gtk::Widget*       m_file_info_box_2;

            std::vector<Glib::RefPtr<Glib::Binding>> m_bindings;

        public:
            chat_file_popover(const std::shared_ptr<toxmm::file>& file);
            ~chat_file_popover();

        protected:
            void update_complete();
    };
}
#endif
