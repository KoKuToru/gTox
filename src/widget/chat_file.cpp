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
#include "chat_file.h"
#include "tox/contact/file/file.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

using namespace widget;

file::file(BaseObjectType* cobject,
           utils::builder builder,
           const std::shared_ptr<toxmm2::file>& file):
    Gtk::Frame(cobject),
    m_file(file) {

    builder.get_widget("file_resume", m_file_resume);
    builder.get_widget("file_cancel", m_file_cancel);
    builder.get_widget("file_pause", m_file_pause);
    builder.get_widget("file_progress", m_file_progress);
    builder.get_widget("revealer_download", m_revealer_download);
    builder.get_widget("spinner", m_spinner);
    builder.get_widget("file_open_bar", m_file_open_bar);
    builder.get_widget("file_speed", m_file_speed);
    builder.get_widget("file_size", m_file_size);
    builder.get_widget("file_time", m_file_time);
    builder.get_widget("file_name", m_file_name);
    builder.get_widget("widget_list", m_widget_list);
    builder.get_widget("file_info_box_1", m_file_info_box_1);
    builder.get_widget("file_info_box_2", m_file_info_box_2);
    builder.get_widget("file_dir", m_file_dir);
    builder.get_widget("file_open", m_file_open);

    auto binding_flags = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_name(),
                             m_file_name->property_label(),
                             binding_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_size(),
                             m_file_size->property_label(),
                             binding_flags,
                             [](const uint64_t& input, Glib::ustring& output) {
        //TODO: human readable string
        output = std::to_string(input);
        return true;
    }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_progress(),
                             m_file_progress->property_fraction(),
                             binding_flags));

    //Button handling
    auto proprety_state_update = [this]() {
        m_file_resume->property_active() = false;
        m_file_pause ->property_active() = false;
        m_file_cancel->property_active() = false;
        switch (m_file->property_state().get_value()) {
            case TOX_FILE_CONTROL_RESUME:
                m_file_resume->property_active() = true;
                break;
            case TOX_FILE_CONTROL_PAUSE:
                m_file_pause ->property_active() = true;
                break;
            case TOX_FILE_CONTROL_CANCEL:
                m_file_cancel->property_active() = true;
                break;
        }
    };
    m_file->property_state()
            .signal_changed()
            .connect(sigc::track_obj(proprety_state_update, *this));
    proprety_state_update();

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_resume->property_active(),
                             m_file_resume->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_pause->property_active(),
                             m_file_pause->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file_cancel->property_active(),
                             m_file_cancel->property_sensitive(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));

    m_file_resume->signal_clicked().connect([this]() {
        if (m_file_resume->property_active() &&
                m_file->property_state() != TOX_FILE_CONTROL_RESUME) {
            m_file->property_state() = TOX_FILE_CONTROL_RESUME;
        }
    });
    m_file_pause->signal_clicked().connect([this]() {
        if (m_file_pause->property_active() &&
                m_file->property_state() != TOX_FILE_CONTROL_PAUSE) {
            m_file->property_state() = TOX_FILE_CONTROL_PAUSE;
        }
    });
    m_file_cancel->signal_clicked().connect([this]() {
        if (m_file_cancel->property_active() &&
                m_file->property_state() != TOX_FILE_CONTROL_CANCEL) {
            m_file->property_state() = TOX_FILE_CONTROL_CANCEL;
        }
    });

    //Display control
    m_dispatcher.emit([this, binding_flags]() {
        m_bindings.push_back(Glib::Binding::bind_property(
                                 m_file->property_complete(),
                                 m_revealer_download->property_reveal_child(),
                                 binding_flags| Glib::BINDING_INVERT_BOOLEAN));
    });
}

utils::builder::ref<file> file::create(const std::shared_ptr<toxmm2::file>& file) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");
    return utils::builder(builder)
            .get_widget_derived<widget::file>("chat_filerecv",
                                              file);
}
