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
#include "chat_file_popover.h"
#include "utils/builder.h"
#include <glibmm/i18n.h>
#include <tox/contact/file/file.h>

using namespace widget;

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

chat_file_popover::chat_file_popover(const std::shared_ptr<toxmm::file>& file): m_file(file) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    utils::builder builder = Gtk::Builder::create_from_resource("/org/gtox/ui/chat_filerecv.ui");

    builder.get_widget("chat_file_popover", m_body);
    builder.get_widget("file_resume", m_file_resume);
    builder.get_widget("file_cancel", m_file_cancel);
    builder.get_widget("file_pause", m_file_pause);
    builder.get_widget("file_progress", m_file_progress);
    builder.get_widget("file_open_bar", m_file_open_bar);
    builder.get_widget("file_speed", m_file_speed);
    builder.get_widget("file_size", m_file_size);
    builder.get_widget("file_time", m_file_time);
    builder.get_widget("file_name2", m_file_name);
    builder.get_widget("file_dir", m_file_dir);
    builder.get_widget("file_open", m_file_open);
    builder.get_widget("file_control", m_file_control);
    builder.get_widget("file_info_box_1", m_file_info_box_1);
    builder.get_widget("file_info_box_2", m_file_info_box_2);

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
        utils::debug::scope_log log(DBG_LVL_4("gtox"), {});
        //TODO: Replace the following line with
        //      Glib::format_size(input, G_FORMAT_SIZE_DEFAULT)
        //      but will need Glib 2.45.31 or newer
        output = Glib::convert_return_gchar_ptr_to_ustring(
                     g_format_size_full(input, G_FORMAT_SIZE_IEC_UNITS));
        return true;
    }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_progress(),
                             m_file_progress->property_fraction(),
                             binding_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_complete(),
                             m_file_progress->property_visible(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_complete(),
                             m_file_info_box_1->property_visible(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_complete(),
                             m_file_info_box_2->property_visible(),
                             binding_flags | Glib::BINDING_INVERT_BOOLEAN));
    //Button handling
    auto proprety_state_update = [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto state = m_file->property_state().get_value();
        bool file_resume = state == TOX_FILE_CONTROL_RESUME;
        bool file_pause  = state == TOX_FILE_CONTROL_PAUSE;
        bool file_cancel = state == TOX_FILE_CONTROL_CANCEL;

        if (file_resume != m_file_resume->property_active()) {
            m_file_resume->property_active() = file_resume;
        }
        if (file_pause != m_file_pause->property_active()) {
            m_file_pause->property_active() = file_pause;
        }
        if (file_cancel != m_file_cancel->property_active()) {
            m_file_cancel->property_active() = file_cancel;
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

    m_file_resume->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_resume->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_RESUME;
        }
    }, *this));
    m_file_pause->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_pause->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_PAUSE;
        }
    }, *this));
    m_file_cancel->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        if (m_file_cancel->property_active()) {
            m_file->property_state() = TOX_FILE_CONTROL_CANCEL;
        }
    }, *this));

    m_bindings.push_back(Glib::Binding::bind_property(
                             m_file->property_active(),
                             m_file_control->property_sensitive(),
                             binding_flags));

    //Buttons when file complete
    m_file_dir->signal_clicked().connect([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto filemanager = Gio::AppInfo::get_default_for_type("inode/directory",
                                                              true);

        try {
            if (!filemanager) {
                throw std::runtime_error("No filemanager found");
            }
            filemanager->launch_uri(
                        Glib::filename_to_uri(m_file->property_path().get_value()));
        } catch (...) {
            // TODO Show user error if no filemanager
        }
    });
    m_file_open->signal_clicked().connect([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        try {
            Gio::AppInfo::launch_default_for_uri(
                        Glib::filename_to_uri(
                            m_file->property_path().get_value()));
        } catch (...) {
            // TODO Show user error if no filemanager
        }
    });

    m_file->property_complete()
            .signal_changed()
            .connect(sigc::mem_fun(*this, &chat_file_popover::update_complete));
    update_complete();

    add(*m_body);
}

chat_file_popover::~chat_file_popover() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    delete m_body;
}

void chat_file_popover::update_complete() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    if (m_file->is_recv() && !m_file->property_complete().get_value()) {
        return;
    }

    m_file_open_bar->show();
}
