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
#include "profile_selection.h"
#include <glibmm/i18n.h>
#include <iostream>
#include "tox/core.h"
#include "tox/exception.h"
#include "glib/gthread.h"
#include "widget/avatar.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace dialog;

profile_selection::profile_selection(BaseObjectType* cobject, utils::builder builder, const std::vector<std::string>& accounts):
    Gtk::Window(cobject),
    m_abort(true),
    m_quited(false) {

    builder->get_widget("profile_list", m_profile_list);
    builder->get_widget("revealer", m_revealer);

    set_title(_("PROFILE_TITLE"));
    set_default_geometry(300, 300);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    auto hbar = builder.get_widget<Gtk::HeaderBar>("headerbar");
    hbar->set_title(_("PROFILE_TITLE"));
    hbar->set_subtitle(_("PROFILE_SUBTITLE"));

    Gtk::ListBox* profile_selection_list   = builder.get_widget<Gtk::ListBox>("profile_list");
    Gtk::Button*  profile_selection_new    = builder.get_widget<Gtk::Button>("profile_new");
    Gtk::Button*  profile_selection_select = builder.get_widget<Gtk::Button>("profile_select");

    profile_selection_list->signal_row_selected().connect([this, profile_selection_select](Gtk::ListBoxRow* row) {
        if (m_quited) {
            return;
        }
        if (profile_selection_select) {
            profile_selection_select->set_sensitive(row != nullptr);
        }
    });
    profile_selection_list->signal_row_activated().connect([this, profile_selection_select](Gtk::ListBoxRow*) {
        if (profile_selection_select) {
            profile_selection_select->clicked();
        }
    });

    profile_selection_new->signal_clicked().connect([this](){
        m_abort = false;
        quit();
    });

    profile_selection_list->signal_button_press_event().connect_notify([this, profile_selection_list](GdkEventButton* event) {
        if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
            auto item = profile_selection_list->get_row_at_y(event->y);
            if (item) {
                profile_selection_list->select_row(*item);
                m_popup_menu.popup(event->button, event->time);
            }
        }
    });

    profile_selection_select->set_sensitive(false);
    profile_selection_select->signal_clicked().connect([this, profile_selection_list](){
        m_abort = false;

        auto row = profile_selection_list->get_selected_row();
        if (row) {
            m_selected_path = m_accounts[row->get_index()];
        }

        quit();
    });

    auto open_in_fm = Gtk::manage(new Gtk::MenuItem(_("PROFILE_MENU_OPEN_IN_FILEMANAGER"), true));

    m_popup_menu.append(*open_in_fm);

    m_popup_menu.accelerate(*this);
    m_popup_menu.show_all();

    open_in_fm->signal_activate().connect([this, profile_selection_list](){
        auto row = profile_selection_list->get_selected_row();
        if (row) {
            try {
                Gio::AppInfo::get_default_for_type("inode/directory", true)->launch_uri(Glib::filename_to_uri(m_accounts[row->get_index()]));
            } catch (...) {
                //TODO: display error !
            }
        }
    });

    set_accounts(accounts);
}

void profile_selection::set_accounts(const std::vector<std::string>& accounts) {
    m_accounts = accounts;

    m_thread = Glib::Thread::create([this, accounts](){
        Glib::ustring tox_name;
        Glib::ustring tox_status;
        toxmm::contactAddrPublic tox_addr;

        for (auto acc : accounts) {
            int tox_error = 0;
            bool can_write = false;

            //TRY TO LOAD TOX DATA
            try {
                toxmm::core::try_load(acc, tox_name, tox_status, tox_addr, can_write);
                if (tox_name.empty()) {
                    tox_name = tox_addr;
                }
            } catch (toxmm::exception exception) {
                if (exception.type() == typeid(TOX_ERR_NEW)) {
                    switch (exception.what_id()) {
                        case TOX_ERR_NEW_LOAD_BAD_FORMAT:
                            tox_error = 1;
                            break;
                        case TOX_ERR_NEW_LOAD_ENCRYPTED:
                            tox_error = 2;
                            break;
                        default:
                            tox_error = 3;
                            break;
                    }
                } else {
                    tox_error = 3;
                }
            } catch (...) {
                throw;
            }

            m_dispatcher.emit([this, tox_name, acc, tox_status, tox_error, can_write, tox_addr](){
                utils::builder builder = Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_profile.ui");
                Gtk::ListBoxRow* row = nullptr;
                builder->get_widget("pofile_list_item", row);
                if (row) {
                    Gtk::Label* name = nullptr;
                    Gtk::Label* status = nullptr;
                    Gtk::Label* path = nullptr;
                    builder.get_widget("name", name);
                    builder.get_widget("status", status);
                    builder.get_widget("path", path);
                    builder.get_widget_derived<widget::avatar>("avatar", tox_addr);

                    path->set_text(acc);
                    row->set_sensitive(false);
                    switch (tox_error) {
                        case 0:
                            name->set_text(tox_name);
                            status->set_text(tox_status);
                            if (can_write) {
                                row->set_sensitive(true);
                            }
                            break;
                        case 1:
                            name->set_text(_("PROFILE_CORRUPTED_TITLE"));
                            status->set_text(_("PROFILE_CORRUPTED"));
                            break;
                        case 2:
                            name->set_text(_("PROFILE_CRYPTED_TITLE"));
                            status->set_text(_("PROFILE_CRYPTED"));
                            break;
                        default:
                            name->set_text(_("PROFILE_UNEXPECTED_TITLE"));
                            status->set_text(_("PROFILE_UNEXPECTED"));
                            break;
                    }

                    row->show();
                    m_profile_list->add(*row);

                    //reveale profil
                    Gtk::Revealer* revealer;
                    builder.get_widget("revealer", revealer);
                    revealer->reference();
                    Glib::signal_idle().connect_once([revealer](){
                        revealer->set_reveal_child(true);
                        revealer->unreference();
                    });
                }

                return false;
            });
        }

        m_revealer->set_reveal_child(false);
    }, true);

}

utils::builder::ref<profile_selection> profile_selection::create(const std::vector<std::string>& accounts) {
    return utils::builder::create_ref<profile_selection>(
                "/org/gtox/ui/dialog_profile.ui",
                "dialog_profile",
                accounts);
}

void profile_selection::quit() {
    m_quited = true;
    hide();
}

profile_selection::~profile_selection() {
    m_quited = true;
    if (m_thread != nullptr) {
        //wait for thread :D
        m_thread->join();
    }
}

bool profile_selection::is_aborted() {
    return m_abort;
}

std::string profile_selection::get_path() {
    return m_selected_path;
}
