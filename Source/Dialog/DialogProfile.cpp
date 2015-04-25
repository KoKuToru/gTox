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
#include "DialogProfile.h"
#include <glibmm/i18n.h>
#include <iostream>
#include <Tox/Toxmm.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

DialogProfile::DialogProfile(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::Window(cobject),
    m_builder(builder),
    m_abort(true),
    m_quited(false) {

    set_title(_("PROFILE_TITLE"));
    set_default_geometry(300, 300);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    Gtk::HeaderBar* hbar = nullptr;
    builder->get_widget("headerbar", hbar);
    if (hbar) {
        set_titlebar(*hbar);
        hbar->set_title(_("PROFILE_TITLE"));
        hbar->set_subtitle(_("PROFILE_SUBTITLE"));
    }

    Gtk::ListBox* profile_list = nullptr;
    Gtk::Button* profile_new = nullptr;
    Gtk::Button* profile_select = nullptr;
    Gtk::Button* profile_abort = nullptr;
    builder->get_widget("profile_list", profile_list);
    builder->get_widget("profile_new", profile_new);
    builder->get_widget("profile_select", profile_select);
    builder->get_widget("profile_abort", profile_abort);

    if (profile_list) {
        profile_list->signal_row_selected().connect([this, profile_select](Gtk::ListBoxRow* row) {
                if (m_quited) {
                    return;
                }
                if (profile_select) {
                    profile_select->set_sensitive(row != nullptr);
                }
            });
        profile_list->signal_row_activated().connect([this, profile_select](Gtk::ListBoxRow*) {
            if (profile_select) {
                profile_select->clicked();
            }
        });
    }

    if (profile_new) {
        profile_new->signal_clicked().connect([this](){
            m_abort = false;
            quit();
        });
    }

    if (profile_select) {
        profile_select->set_sensitive(false);
        profile_select->signal_clicked().connect([this, profile_list](){
            m_abort = false;
            if (profile_list) {
                auto row = profile_list->get_selected_row();
                if (row) {
                    m_selected_path = m_accounts[row->get_index()];
                }
            }
            quit();
        });
    }

    if (profile_abort) {
        profile_abort->signal_clicked().connect([this](){
            quit();
        });
    }
}

#include "glib/gthread.h"

void DialogProfile::set_accounts(const std::vector<std::string>& accounts) {
    m_accounts = accounts;
    Gtk::ListBox* list = nullptr;
    m_builder->get_widget("profile_list", list);
    if (list) {
        m_thread = Glib::Thread::create([this, list, accounts](){
            Glib::ustring tox_name;
            Glib::ustring tox_status;
            bool tox_okay = false;

            for (auto acc : accounts) {
                //TRY TO LOAD TOX DATA
                try {
                    Toxmm::instance().init(acc);
                    tox_name = Toxmm::instance().get_name_or_address();
                    tox_status = Toxmm::instance().get_status_message();
                    tox_okay = true;
                } catch (...) {
                    tox_okay = false;
                }
                Toxmm::destroy();

                m_events.push_back(Glib::signal_idle().connect([list, tox_name, acc, tox_status, tox_okay](){
                    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_profile.ui");
                    Gtk::ListBoxRow* row = nullptr;
                    builder->get_widget("pofile_list_item", row);
                    if (row) {
                        Gtk::Label* name = nullptr;
                        Gtk::Label* status = nullptr;
                        Gtk::Label* path = nullptr;
                        Gtk::Image* avatar = nullptr;
                        builder->get_widget("name", name);
                        builder->get_widget("status", status);
                        builder->get_widget("path", path);
                        builder->get_widget("avatar", avatar);
                        if (name && status && path && avatar) {
                            avatar->set(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg")->scale_simple(
                                           72,
                                           72,
                                           Gdk::INTERP_BILINEAR));
                            path->set_text(acc);
                            if (tox_okay) {
                                name->set_text(tox_name);
                                status->set_text(tox_status);
                            } else {
                                row->set_sensitive(false);
                            }
                        }
                        row->show();
                        list->add(*row);
                    }

                    return false;
                }));
            }

            Gtk::Revealer* revealer;
            m_builder->get_widget("revealer", revealer);
            revealer->set_reveal_child(false);
        }, true);
    }
}

DialogProfile* DialogProfile::create(const std::vector<std::string>& accounts) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_profile.ui");
    DialogProfile* tmp = nullptr;
    builder->get_widget_derived("dialog_profile", tmp);
    tmp->set_accounts(accounts);
    return tmp;
}

void DialogProfile::quit() {
    m_quited = true;
    hide();
}

DialogProfile::~DialogProfile() {
    m_quited = true;
    if (m_thread != nullptr) {
        //wait for thread :D
        m_thread->join();
    }
}

bool DialogProfile::is_aborted() {
    return m_abort;
}

std::string DialogProfile::get_path() {
    return m_selected_path;
}
