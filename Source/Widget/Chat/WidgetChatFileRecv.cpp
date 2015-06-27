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
#include "WidgetChatFileRecv.h"
#include <iomanip>
#include <glibmm/i18n.h>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

static Glib::ustring s_units[] = {"Bytes", "KiB", "MiB", "GiB"};

WidgetChatFileRecv::WidgetChatFileRecv(BaseObjectType* cobject,
                                       gToxBuilder builder,
                                       gToxObservable* observable,
                                       Toxmm::EventFileRecv file):
    Gtk::Frame(cobject),
    gToxObserver(observable),
    m_builder(builder),
    m_recv(observable, file) {

    m_file_number = file.file_number;

    m_builder.get_widget<Gtk::Label>("file_name")->set_text(file.filename);
    double size  = file.file_size;
    int unit = 0;
    while (size > 1024 && unit < 4) {
        size /= 1024;
        unit += 1;
    }
    auto file_size = Glib::ustring::format(std::fixed, std::setprecision(2), size)
                     + " " + s_units[unit];
    m_builder.get_widget<Gtk::Label>("file_size")->set_text(file_size);

    m_builder.get_widget("file_resume", m_file_resume);
    m_builder.get_widget("file_cancel", m_file_cancel);
    m_builder.get_widget("file_pause", m_file_pause);

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
            m_recv.resume();
        }
    });
    m_file_cancel->signal_toggled().connect([this](){
        if (m_file_cancel->get_active()) {
            if (m_file_resume->get_active()) {
                m_file_resume->set_active(false);
            }
            if (m_file_pause->get_active()) {
                m_file_pause->set_active(false);
            }
            m_recv.cancel();
            m_builder.get_widget<Gtk::Widget>("file_info_box_1")->hide();
            m_builder.get_widget<Gtk::Widget>("file_info_box_2")->hide();
            m_update_interval.disconnect();
            m_file_progress->hide();
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
            m_recv.pause();
        }
    });

    m_builder.get_widget("file_progress", m_file_progress);

    m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(true);
    m_builder.get_widget<Gtk::Revealer>("revealer_preview")->set_reveal_child(false);

    m_set_image_connection = signal_set_image().connect([this](Glib::RefPtr<Gdk::Pixbuf> image) {
        m_builder.get_widget<Gtk::Image>("image")->property_pixbuf() = image;
        m_builder.get_widget<Gtk::Revealer>("revealer_loading")->set_reveal_child(false);
        m_builder.get_widget<Gtk::Revealer>("revealer_image")->set_reveal_child(true);
    });

    m_builder.get_widget("file_speed", m_file_speed);
    m_builder.get_widget("file_time", m_file_time);
    size_t dummy;
    m_recv.get_progress(m_last_position, dummy);
    m_update_interval = Glib::signal_timeout().connect([this](){
        size_t position, size;
        m_recv.get_progress(position, size);
        size_t diff = position - m_last_position;
        m_last_position = position;

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
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(h)) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(min)) + ":" +
                    Glib::ustring::format(std::setw(2), std::setfill(L'0'), std::right, std::to_string(sec)));

        return true;
    }, 500);

    //auto start TODO:check in config
    if (file.file_size < 1024*1024) {
        m_file_resume->clicked();
    }
}

void WidgetChatFileRecv::observer_handle(const ToxEvent& event) {
    //nothing yet
    if (event.type() == typeid(gToxFileRecv::EventFileProgress)) {
        auto data = event.get<gToxFileRecv::EventFileProgress>();

        if (data.file_number != m_file_number) {
            return;
        }

        m_file_progress->set_fraction(data.file_position / (double)data.file_size);

        if (data.file_position == data.file_size) {
            //finish !
            m_update_interval.disconnect();
            m_thread = Glib::Thread::create([this](){
                //TODO:check for filesize
                //try image;
                try {
                    auto image = Gdk::Pixbuf::create_from_file(m_recv.get_path());
                    if (image->get_width() > 256 || image->get_height() > 256) {
                        int w, h;
                        if (image->get_width() > image->get_height()) {
                            w = 256;
                            h = image->get_height()*256/image->get_width();
                        } else {
                            h = 256;
                            w = image->get_width()*256/image->get_height();
                        }
                        image = image->scale_simple(w, h, Gdk::InterpType::INTERP_BILINEAR);
                    }
                    if (image) {
                        std::lock_guard<std::mutex> lg(m_mutex);
                        m_signal_set_image.emit(image);
                        return;
                    }
                }
                catch (...) {}

                //try video
            });

            //display
            m_builder.get_widget<Gtk::Revealer>("revealer_download")->set_reveal_child(false);
            m_builder.get_widget<Gtk::Revealer>("revealer_preview")->set_reveal_child(true);
        }
    }
}

WidgetChatFileRecv* WidgetChatFileRecv::create(gToxObservable* instance, Toxmm::EventFileRecv file) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");
    return gToxBuilder(builder)
            .get_widget_derived<WidgetChatFileRecv>("chat_filerecv",
                                                    instance,
                                                    file);
}

WidgetChatFileRecv::~WidgetChatFileRecv() {
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_set_image_connection.disconnect();
    }
    if (m_thread != nullptr) {
        //wait for thread
        m_thread->join();
    }
}
