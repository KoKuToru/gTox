/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
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

#include "FirstStartAssistant.h"
#include "Tox/Tox.h"
#include <glibmm/i18n.h>

FirstStartAssistant::FirstStartAssistant(Glib::ustring path):
path(path),
aborted(false){
    set_default_size(800, 600);

    import.pack_start(*Gtk::manage(new Gtk::Label(_("Not implemented yet"))));
    finish.pack_start(*Gtk::manage(new Gtk::Label(_("You are done.\n Have fun using gTox!"))));

    append_page(welcome);
    append_page(account);
    append_page(create);
    append_page(import);
    append_page(finish);

    set_page_complete(welcome, true);
    set_page_complete(account, true);
    set_page_complete(create, false);
    set_page_complete(import, false);
    set_page_complete(finish, false);

    set_page_type(welcome, Gtk::ASSISTANT_PAGE_INTRO);
    set_page_type(account, Gtk::ASSISTANT_PAGE_CONTENT);
    set_page_type(create, Gtk::ASSISTANT_PAGE_CONTENT);
    set_page_type(import, Gtk::ASSISTANT_PAGE_CONTENT);
    set_page_type(finish, Gtk::ASSISTANT_PAGE_SUMMARY);

    set_page_title(*get_nth_page(0), _("Welcome"));
    set_page_title(*get_nth_page(1), _("Account"));
    set_page_title(*get_nth_page(2), _("New account"));
    set_page_title(*get_nth_page(3), _("Import"));
    set_page_title(*get_nth_page(4), _("Finish"));

    set_forward_page_func(sigc::mem_fun(this, &FirstStartAssistant::on_next));


    create.getEntryName().signal_changed().connect([this](){
        set_page_complete(create, !create.getEntryName().get_text().empty());
    });

    show_all_children();
}

FirstStartAssistant::~FirstStartAssistant(){
}

int FirstStartAssistant::getPage(Gtk::Widget& widget){
    for(int i = 0; i < get_n_pages(); ++i)
        if(get_nth_page(i) == (Gtk::Widget*)&widget)
            return i;
    return -1;
}

int FirstStartAssistant::on_next(int page){
    if(page == getPage(account)){
        if(account.getRbImport().get_active())
            return getPage(import);
    }
    if(page == getPage(create)){
        return getPage(finish);
    }
    return page+1;
}

void FirstStartAssistant::on_cancel(){
    int status = Gtk::MessageDialog(_("Do you really want to guit the assistant?"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true).run();

    if(status == Gtk::RESPONSE_YES){
        aborted = true;
        Gtk::Main::quit();
    }
}

void FirstStartAssistant::on_close(){
    if(!aborted){
        Tox::instance().init();
        Tox::FriendAddr myAddr = Tox::instance().get_address();

        Glib::ustring sMyAddr = Tox::to_hex(myAddr.data(), 32);
        Tox::instance().set_name(create.getEntryName().get_text());
        Tox::instance().set_status_message("Powered by gTox");
        path = Glib::build_filename(path, sMyAddr + ".state");
        Tox::instance().save(path);
    }
    Gtk::Main::quit();
}
