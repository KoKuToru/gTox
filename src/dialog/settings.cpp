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
#include "settings.h"
#include "main.h"
#include <glibmm/i18n.h>
#include "utils/webcam.h"

using namespace dialog;

#ifndef SIGC_CPP11_HACK
#define SIGC_CPP11_HACK
namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}
#endif

#include <iostream>

settings::settings(main& main)
    : Glib::ObjectBase(typeid(settings)),
      detachable_window(sigc::mem_fun(main,
                                      &dialog::main::detachable_window_add),
                        sigc::mem_fun(main,
                                      &dialog::main::detachable_window_del)),
      m_main(main) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    utils::builder builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_settings.ui"));

    builder.get_widget("body", m_body);

    builder.get_widget("tray_visible" , m_tray_visible);
    builder.get_widget("tray_on_start", m_tray_on_start);
    builder.get_widget("tray_on_close", m_tray_on_close);

    builder.get_widget("c_n_on_message", m_c_n_on_message);
    builder.get_widget("c_n_with_audio", m_c_n_with_audio);
    builder.get_widget("c_auto_away", m_c_auto_away);
    builder.get_widget("c_send_typing", m_c_send_typing);
    builder.get_widget("c_chat_logging", m_c_logging);

    builder.get_widget("ft_saveto", m_ft_save_to);
    builder.get_widget("ft_auto_accept", m_ft_auto_accept);
    builder.get_widget("ft_display_inline", m_ft_display_inline);

    builder.get_widget("cl_use_compact", m_cl_use_compact);
    builder.get_widget("cl_display_active", m_cl_display_active);

    builder.get_widget("connection_udp", m_connection_udp);
    builder.get_widget("proxy_type", m_proxy_type);
    builder.get_widget("proxy_host", m_proxy_host);
    builder.get_widget("proxy_port", m_proxy_port);

    builder.get_widget("t_color", m_t_color);

    builder.get_widget("p_remember", m_p_remember);

    builder.get_widget("video_default_device", m_video_device);
    builder.get_widget("settings_video_error", m_video_error);
    m_video_preview = builder.get_widget_derived<widget::imagescaled>(
                          "settings_video_player");

    property_body() = m_body;

    property_headerbar_title() = _("gTox settings");
    property_headerbar_subtitle() = _("Configure gTox");

    auto binding_flag = Glib::BINDING_DEFAULT |
                        Glib::BINDING_SYNC_CREATE |
                        Glib::BINDING_BIDIRECTIONAL;

    //SYSTEMTRAY SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_systemtray_visible(),
                             m_tray_visible->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_systemtray_on_start(),
                             m_tray_on_start->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_systemtray_on_close(),
                             m_tray_on_close->property_active(),
                             binding_flag));

    //CHAT-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_chat_notification_on_message(),
                             m_c_n_on_message->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_chat_notification_with_audio(),
                             m_c_n_with_audio->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_chat_auto_away(),
                             m_c_auto_away->property_active_id(),
                             binding_flag,
                             [](const int& value_in, Glib::ustring& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = std::to_string(value_in);
                                 return true;
                             },
                             [](const Glib::ustring& value_in, int& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in.raw() });
                                 value_out = std::stoi(value_in);
                                 return true;
                             }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_chat_send_typing(),
                             m_c_send_typing->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_chat_logging(),
                             m_c_logging->property_active(),
                             binding_flag));

    //FILETRANSFER-SETTINGS
    m_main.tox()->config()->property_download_path()
            .signal_changed().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_ft_save_to->set_file(Gio::File::create_for_path(
                                   m_main.tox()->config()
                                   ->property_download_path().get_value()));
    }, *this));
    m_ft_save_to->set_file(Gio::File::create_for_path(
                               m_main.tox()->config()
                               ->property_download_path().get_value()));
    m_ft_save_to->signal_file_set().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_main.tox()->config()
                ->property_download_path() = m_ft_save_to->get_file()
                                             ->get_path();
    }, *this));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_file_auto_accept(),
                             m_ft_auto_accept->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_file_display_inline(),
                             m_ft_display_inline->property_active(),
                             binding_flag));

    //CONTACTLIST-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_contacts_compact_list(),
                             m_cl_use_compact->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.config()->property_contacts_display_active(),
                             m_cl_display_active->property_active(),
                             binding_flag));

    //GLOBAL CONNECTION-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_connection_udp(),
                             m_connection_udp->property_active(),
                             binding_flag));

    //GLOBAL PROXY-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_proxy_type(),
                             m_proxy_type->property_active_id(),
                             binding_flag,
                             [](const TOX_PROXY_TYPE& value_in, Glib::ustring& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = std::to_string(value_in);
                                 return true;
                             },
                             [](const Glib::ustring& value_in, TOX_PROXY_TYPE& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in.raw() });
                                 value_out = TOX_PROXY_TYPE(std::stoi(value_in));
                                 return true;
                             }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_proxy_host(),
                             m_proxy_host->property_text(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_proxy_port(),
                             m_proxy_port->property_text(),
                             binding_flag,
                             [](const int& value_in, Glib::ustring& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = std::to_string(value_in);
                                 return true;
                             },
                             [](const Glib::ustring& value_in, int& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in.raw() });
                                 try {
                                     value_out = std::stoi(value_in);
                                 } catch (...) {
                                     return false;
                                 }
                                 return true;
                             }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_proxy_type(),
                             m_proxy_host->property_sensitive(),
                             Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                             [](const TOX_PROXY_TYPE& value_in, bool& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = value_in != TOX_PROXY_TYPE_NONE;
                                 return true;
                             }));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_main.tox()->config()->property_proxy_type(),
                             m_proxy_port->property_sensitive(),
                             Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE,
                             [](const TOX_PROXY_TYPE& value_in, bool& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = value_in != TOX_PROXY_TYPE_NONE;
                                 return true;
                             }));

    //GLOBAL THEME-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_theme_color(),
                             m_t_color->property_active_id(),
                             binding_flag,
                             [](const int& value_in, Glib::ustring& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in });
                                 value_out = std::to_string(value_in);
                                 return true;
                             },
                             [](const Glib::ustring& value_in, int& value_out) {
                                 utils::debug::scope_log log(DBG_LVL_4("gtox"), { value_in.raw() });
                                 try {
                                     value_out = std::stoi(value_in);
                                 } catch (...) {
                                     return false;
                                 }
                                 return true;
                             }));

    //GLOBAL PROFILE-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_profile_remember(),
                             m_p_remember->property_active(),
                             binding_flag));

    //GLOBAL VIDEO-SETTINGS
    auto webcam_devices_store = Glib::RefPtr<Gtk::ListStore>
                                ::cast_dynamic(m_video_device->get_model());
    for (auto device : utils::webcam::get_webcam_devices()) {
        auto new_row = webcam_devices_store->append();
        new_row->set_value(0, utils::webcam::get_webcam_device_name(device));
    }
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_video_default_device(),
                             m_video_device->property_active_id(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             m_webcam.property_pixbuf(),
                             m_video_preview->property_pixbuf(),
                             binding_flag & (~Glib::BINDING_BIDIRECTIONAL)));
    auto change_video_source = sigc::track_obj([this]() {
        m_video_error->set_label("");
        auto name = m_video_device->property_active_id().get_value();
        auto device = utils::webcam::get_webcam_device_by_name(name);
        m_webcam.property_device() = device;
        if (device) {
            m_webcam.property_state() = Gst::STATE_PLAYING;
        }
    }, *this);
    m_video_device->property_active_id()
            .signal_changed().connect(change_video_source);
    m_video_preview->signal_unmap().connect_notify(sigc::track_obj([this](){
        m_webcam.property_state() = Gst::STATE_NULL;
    }, *this));
    m_video_preview->signal_map().connect_notify(change_video_source);
    m_webcam.signal_error().connect(sigc::track_obj([this](Glib::ustring error) {
        m_video_error->set_label(error);
    }, *this));
}

settings::~settings() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    delete m_video_preview;
    delete m_body;
}
