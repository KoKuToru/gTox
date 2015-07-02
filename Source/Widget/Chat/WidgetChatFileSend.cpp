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
#include "WidgetChatFileSend.h"
#include <iomanip>
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

static Glib::ustring s_units[] = {"Bytes", "KiB", "MiB", "GiB"};

void WidgetChatFileSend::init(gToxBuilder builder) {
    auto preview_builder = WidgetChatFilePreview::create(Glib::filename_to_uri(m_path));
    m_preview = Gtk::manage(preview_builder.raw());

    builder.get_widget<Gtk::Box>("widget_list")->add(*m_preview);

    builder.get_widget<Gtk::Label>("file_name")->set_text(m_path);
    double size  = m_file_size;
    int unit = 0;
    while (size > 1024 && unit < 4) {
        size /= 1024;
        unit += 1;
    }
    auto file_size = Glib::ustring::format(std::fixed, std::setprecision(2), size)
                     + " " + s_units[unit];
    builder.get_widget<Gtk::Label>("file_size")->set_text(file_size);

    builder.get_widget("file_resume", m_file_resume);
    builder.get_widget("file_cancel", m_file_cancel);
    builder.get_widget("file_pause", m_file_pause);
    builder.get_widget("revealer_download", m_revealer_download);
    builder.get_widget("spinner", m_spinner);
    builder.get_widget("file_open_bar", m_file_open_bar);

    m_file_pause->set_active(true);

    m_file_resume->signal_toggled().connect([this](){
        if (m_file_resume->get_active()) {
            if (m_file_cancel->get_active()) {
                m_file_cancel->set_active(false);
            }
            if (m_file_pause->get_active()) {
                m_file_pause->set_active(false);
            }
            m_file_speed->show();
            m_file_time->show();
            m_file_resume->set_sensitive(false);
            try {
                if (!m_send_skip_control) {
                    m_send.resume();
                }
            } catch (...) {
                m_file_cancel->clicked();
            }
        } else {
            m_file_resume->set_sensitive(true);
        }
    });
    auto file_info_box_1 = builder.get_widget<Gtk::Widget>("file_info_box_1");
    auto file_info_box_2 = builder.get_widget<Gtk::Widget>("file_info_box_2");
    m_file_cancel->signal_toggled().connect([this, file_info_box_1, file_info_box_2](){
        if (m_file_cancel->get_active()) {
            if (m_file_resume->get_active()) {
                m_file_resume->set_active(false);
            }
            if (m_file_pause->get_active()) {
                m_file_pause->set_active(false);
            }
            try {
                if (!m_send_skip_control) {
                    m_send.cancel();
                }
            } catch (...) {
                //TODO ?
            }
            file_info_box_1->hide();
            file_info_box_2->hide();
            m_file_cancel->set_sensitive(false);
            m_update_interval.disconnect();
            m_file_progress->hide();
        } else {
            m_file_cancel->set_sensitive(true);
        }
    });
    m_file_pause->signal_toggled().connect([this](){
        if (m_file_pause->get_active()) {
            if (m_file_cancel->get_active()) {
                m_file_cancel->set_active(false);
            }
            if (m_file_resume->get_active()) {
                m_file_resume->set_active(false);
            }
            m_file_speed->hide();
            m_file_time->hide();
            m_file_pause->set_sensitive(false);
            try {
                if (!m_send_skip_control) {
                    m_send.pause();
                }
            } catch (...) {
                m_file_cancel->clicked();
            }
        } else {
            m_file_pause->set_sensitive(true);
        }
    });

    builder.get_widget("file_progress", m_file_progress);

    builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(true);

    builder.get_widget("file_speed", m_file_speed);
    builder.get_widget("file_time", m_file_time);
    size_t dummy;
    m_send.get_progress(m_last_position, dummy);
    m_update_interval = Glib::signal_timeout().connect([this](){
        size_t position, size;
        m_send.get_progress(position, size);
        size_t diff = position - m_last_position;
        m_last_position = position;

        m_file_progress->set_fraction(position / (double)size);

        if (position == size) {
            //finish !
            m_revealer_download->set_reveal_child(false);
            return false;
        }

        double s  = diff / 0.5; //each 500 ms

        double sf = s;
        int unit = 0;
        while (sf > 1024 && unit < 4) {
            sf /= 1024;
            unit += 1;
        }

        m_file_speed->set_text(Glib::ustring::format(std::fixed, std::setprecision(2), sf)
                               + " " + s_units[unit] + "/s");

        if (s < 1) { //1 Bytes/s
            m_file_time->set_text(_("FILE_RECV_LESS_THAN_1_BYTES_EACH_SEC"));
            return true;
        }

        //calculate time it takes to download rest
        s = (size - position) / s;

        int sec = int(s) % 60;
        int min = (int(s) / 60) % 60;
        int h   = (int(s) / 60) / 60;

        m_file_time->set_text(
                    Glib::ustring::format(std::setw(3), std::setfill(L'0'), std::right, h) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, min) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, sec));

        return true;
    }, 500);

    builder.get_widget<Gtk::Button>("file_dir")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::get_default_for_type("inode/directory", true)->launch_uri(Glib::filename_to_uri(m_path));
        } catch (...) {
            //TODO: display error !
        }
    });

    builder.get_widget<Gtk::Button>("file_open")->signal_clicked().connect([this](){
        try {
            Gio::AppInfo::launch_default_for_uri(Glib::filename_to_uri(m_path));
        } catch (...) {
            //TODO: display error !
        }
    });

    m_loaded = m_preview->signal_loaded().connect([this](bool success) {
        m_spinner->hide();
    });
    m_preview->start_loading();
    m_send.emit_progress();
}

WidgetChatFileSend::WidgetChatFileSend(BaseObjectType* cobject,
                                       gToxBuilder builder,
                                       gToxObservable* observable,
                                       Toxmm::FriendNr nr,
                                       Glib::ustring path):
    Gtk::Frame(cobject),
    gToxObserver(observable),
    m_send(observable, nr, TOX_FILE_KIND_DATA, path),
    m_friend_nr(nr),
    m_path(path) {

    //New transfer
    m_file_size = Gio::File::create_for_path(path)->query_info()->get_size();
    init(builder);
}

WidgetChatFileSend::WidgetChatFileSend(BaseObjectType* cobject,
                                       gToxBuilder builder,
                                       gToxObservable* observable,
                                       Toxmm::FriendNr nr,
                                       ToxLogEntity log):
    Gtk::Frame(cobject),
    gToxObserver(observable),
    m_send(observable, nr, TOX_FILE_KIND_DATA, log),
    m_friend_nr(nr),
    m_path(log.data),
    m_file_size(log.filesize) {

    //Resume transfer
    init(builder);
}


void WidgetChatFileSend::observer_handle(const ToxEvent& ev) {
    if (ev.type() == typeid(Toxmm::EventFileControl)) {
        auto data = ev.get<Toxmm::EventFileControl>();
        if (data.nr != m_friend_nr || data.file_number != m_file_number) {
            return;
        }

        m_send_skip_control = true;

        m_file_resume->set_active(false);
        m_file_cancel->set_active(false);
        m_file_pause->set_active(false);

        switch (data.control) {
            case TOX_FILE_CONTROL_RESUME:
                m_file_resume->set_active(true);
                break;
            case TOX_FILE_CONTROL_PAUSE:
                m_file_pause->set_active(true);
                break;
            case TOX_FILE_CONTROL_CANCEL:
                m_file_cancel->set_active(true);
                break;
        }

        m_send_skip_control = false;
    }
}

gToxBuilderRef<WidgetChatFileSend> WidgetChatFileSend::create(gToxObservable* instance,
                                                              Toxmm::FriendNr nr,
                                                              Glib::ustring uri) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetChatFileSend>("chat_filerecv",
                                                    instance,
                                                    nr,
                                                    uri);
}

gToxBuilderRef<WidgetChatFileSend> WidgetChatFileSend::create(gToxObservable* instance,
                                                              Toxmm::FriendNr nr,
                                                              ToxLogEntity log) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetChatFileSend>("chat_filerecv",
                                                    instance,
                                                    nr,
                                                    log);
}

WidgetChatFileSend::~WidgetChatFileSend() {
    before_deconstructor();
}

void WidgetChatFileSend::before_deconstructor() {
    m_update_interval.disconnect();
    m_loaded.disconnect();
    m_preview->before_deconstructor();
}
