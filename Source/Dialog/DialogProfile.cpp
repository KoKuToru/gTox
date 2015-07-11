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
#include "glib/gthread.h"
#include "Widget/WidgetAvatar.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

DialogProfile::DialogProfile(BaseObjectType* cobject, gToxBuilder builder, const std::vector<std::string>& accounts):
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

    Gtk::ListBox* profile_list   = builder.get_widget<Gtk::ListBox>("profile_list");
    Gtk::Button*  profile_new    = builder.get_widget<Gtk::Button>("profile_new");
    Gtk::Button*  profile_select = builder.get_widget<Gtk::Button>("profile_select");

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

    profile_new->signal_clicked().connect([this](){
        m_abort = false;
        quit();
    });

    profile_list->signal_button_press_event().connect_notify([this, profile_list](GdkEventButton* event) {
        if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
            auto item = profile_list->get_row_at_y(event->y);
            if (item) {
                profile_list->select_row(*item);
                m_popup_menu.popup(event->button, event->time);
            }
        }
    });

    profile_select->set_sensitive(false);
    profile_select->signal_clicked().connect([this, profile_list](){
        m_abort = false;

        auto row = profile_list->get_selected_row();
        if (row) {
            m_selected_path = m_accounts[row->get_index()];
        }

        quit();
    });

    auto open_in_fm = Gtk::manage(new Gtk::MenuItem(_("PROFILE_MENU_OPEN_IN_FILEMANAGER"), true));

    m_popup_menu.append(*open_in_fm);

    m_popup_menu.accelerate(*this);
    m_popup_menu.show_all();

    open_in_fm->signal_activate().connect([this, profile_list](){
        auto row = profile_list->get_selected_row();
        if (row) {
            auto filemanager =
                Gio::AppInfo::get_default_for_type("inode/directory", true);

            // TODO Show user error if no filemanager
            if (filemanager)
                filemanager->launch_uri(
                        Glib::filename_to_uri(m_accounts[row->get_index()]));
        }
    });

    set_accounts(accounts);
}

void DialogProfile::set_accounts(const std::vector<std::string>& accounts) {
    m_accounts = accounts;

    m_thread = Glib::Thread::create([this, accounts](){
        Glib::ustring tox_name;
        Glib::ustring tox_status;
        Toxmm::FriendAddr tox_addr;

        for (auto acc : accounts) {
            int tox_error = 0;
            bool can_write = false;

            //TRY TO LOAD TOX DATA
            try {
                Toxmm instance;
                instance.open(acc, false);

                tox_name = instance.get_name_or_address();
                tox_status = instance.get_status_message();
                tox_addr = instance.get_address();
                can_write = instance.profile().can_write();
            } catch (ToxException exception) {
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
                tox_error = 3;
            }

            auto avatar_path = Glib::build_filename(Glib::get_user_config_dir(),
                                                    "tox", "avatars",
                                                    Toxmm::to_hex(tox_addr.data(), TOX_PUBLIC_KEY_SIZE) + ".png");

            m_dispatcher.emit([this, tox_name, acc, tox_status, tox_error, can_write, tox_addr, avatar_path](){
                gToxBuilder builder = Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_profile.ui");
                Gtk::ListBoxRow* row = nullptr;
                builder->get_widget("pofile_list_item", row);
                if (row) {
                    Gtk::Label* name = nullptr;
                    Gtk::Label* status = nullptr;
                    Gtk::Label* path = nullptr;
                    WidgetAvatar* avatar = nullptr;
                    builder.get_widget("name", name);
                    builder.get_widget("status", status);
                    builder.get_widget("path", path);
                    avatar = builder.get_widget_derived<WidgetAvatar>("avatar", avatar_path);

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

gToxBuilderRef<DialogProfile> DialogProfile::create(const std::vector<std::string>& accounts) {
    return gToxBuilder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_profile.ui"))
            .get_widget_derived<DialogProfile>("dialog_profile", accounts);
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
