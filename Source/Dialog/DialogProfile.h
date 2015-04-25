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
#ifndef DIALOGPROFILE_H
#define DIALOGPROFILE_H

#include <gtkmm.h>

// Single chat window
class DialogProfile : public Gtk::Window {
  private:
    const Glib::RefPtr<Gtk::Builder> m_builder;
    std::vector<std::string> m_accounts;
    bool m_abort;
    bool m_quited;
    std::string m_selected_path;
    Glib::Thread* m_thread = nullptr;
    std::list<sigc::connection> m_events;

    void quit();

    void set_accounts(const std::vector<std::string>& accounts);

  public:
    DialogProfile(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    static DialogProfile* create(const std::vector<std::string>& accounts);

    ~DialogProfile();

    bool is_aborted();
    std::string get_path();
};

#endif
