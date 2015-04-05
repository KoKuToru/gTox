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
#include "Generated/icon.h"
#include "Generated/theme.h"
#include <iostream>
#include <Tox/Tox.h>

DialogProfile::DialogProfile(const std::vector<std::string>& accounts):
    m_accounts(accounts), m_abort(true), m_quited(false) {

    auto css = Gtk::CssProvider::create();
    if (!css->load_from_data(THEME::main)) {
        std::cerr << _("LOADING_THEME_FAILED") << std::endl;
    } else {
        auto screen = Gdk::Screen::get_default();
        auto ctx = get_style_context();
        ctx->add_provider_for_screen(
            screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    set_title(_("PROFILE_TITLE"));
    set_default_geometry(300, 300);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    auto hbar = Gtk::manage(new Gtk::HeaderBar());
    set_titlebar(*hbar);
    hbar->set_title(_("PROFILE_TITLE"));
    hbar->set_subtitle(_("PROFILE_SUBTITLE"));
    hbar->show();

    auto abort = Gtk::manage(new Gtk::Button(_("PROFILE_ABORT")));
    hbar->pack_start(*abort);
    abort->show();

    auto rbox = Gtk::manage(new Gtk::Box());
    rbox->get_style_context()->add_class("linked");
    hbar->pack_end(*rbox);
    rbox->show();

    auto badd = Gtk::manage(new Gtk::Button(_("PROFILE_NEW")));
    rbox->pack_start(*badd);
    badd->show();

    auto select = Gtk::manage(new Gtk::Button(_("PROFILE_SELECT")));
    rbox->add(*select);
    select->get_style_context()->add_class("suggested-action");
    select->show();

    auto box = Gtk::manage(new Gtk::ListBox());
    add(*box);
    box->show();

    bool loaded = false;
    for (auto path : accounts) {
        auto row = Gtk::manage(new Gtk::ListBoxRow());
        row->set_name("WidgetContactListItem");
        auto layout = Gtk::manage(new Gtk::Grid());
        auto avatar = Gtk::manage(new Gtk::Image());
        avatar->set(ICON::load_icon(ICON::avatar)->scale_simple(
                       72,
                       72,
                       Gdk::INTERP_BILINEAR));

        auto name   = Gtk::manage(new Gtk::Label(_("PROFILE_CORRUPTED_TITLE"), 1.0, 0.5));
        name->set_line_wrap(false);
        name->set_ellipsize(Pango::ELLIPSIZE_END);
        name->set_hexpand(true);
        name->set_name("Name");
        auto status = Gtk::manage(new Gtk::Label(_("PROFILE_CORRUPTED"), 1.0, 0.5));
        status->set_line_wrap(false);
        status->set_ellipsize(Pango::ELLIPSIZE_END);
        status->set_hexpand(true);
        status->set_name("Status");
        auto wpath   = Gtk::manage(new Gtk::Label(path, 0, 0.5));
        wpath->set_line_wrap(false);
        wpath->set_ellipsize(Pango::ELLIPSIZE_END);
        wpath->set_hexpand(true);
        wpath->set_name("Status");

        layout->attach(*name, 0, 0, 1, 1);
        layout->attach(*status, 0, 1, 1, 1);
        layout->attach(*wpath, 0, 2, 1, 1);
        layout->attach(*avatar, 1, 0, 1, 3);
        row->add(*layout);
        row->show_all();

        box->add(*row);

        //TRY TO LOAD TOX DATA
        try {
            Tox::instance().init(path);
            auto sname = Tox::instance().get_name_or_address();
            auto sstatus = Tox::instance().get_status_message();
            name->set_text(sname);
            status->set_text(sstatus);
            loaded = true;
        } catch (...) {
            row->set_sensitive(false);
        }
        Tox::destroy();
    }

    box->set_activate_on_single_click(false);
    box->signal_row_activated().connect([this, box](Gtk::ListBoxRow* row) {
        m_abort = false;
        m_selected_path = m_accounts[row->get_index()];
        quit();
    });
    box->signal_row_selected().connect([this, select](Gtk::ListBoxRow* row) {
        if (m_quited) {
            return;
        }
        select->set_sensitive(row != nullptr);
    });

    select->set_sensitive(false);
    select->signal_clicked().connect([this, box]() {
        m_abort = false;
        auto row = box->get_selected_row();
        m_selected_path = m_accounts[row->get_index()];
        quit();
    });

    abort->signal_clicked().connect([this]() {
        quit();
    });

    badd->signal_clicked().connect([this]() {
        m_abort = false;
        quit();
    });

    //quick check ..
    if (accounts.size() == 1 && loaded) {
        m_abort = false;
        m_selected_path = m_accounts[0];
        quit();
    }
}

void DialogProfile::quit() {
    m_quited = true;
    Gtk::Main::quit();
}

DialogProfile::~DialogProfile() {
    //
}

bool DialogProfile::is_aborted() {
    return m_abort;
}

std::string DialogProfile::get_path() {
    return m_selected_path;
}
