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

using namespace dialog;

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

#include <iostream>

settings::settings(main& main)
    : Glib::ObjectBase(typeid(settings)),
      detachable_window(sigc::mem_fun(main,
                                      &dialog::main::detachable_window_add),
                        sigc::mem_fun(main,
                                      &dialog::main::detachable_window_del)),
      m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_settings.ui")),
      m_main(main) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    m_builder.get_widget("headerbar_attached", m_headerbar_attached);
    m_builder.get_widget("headerbar_detached", m_headerbar_detached);
    m_builder.get_widget("body", m_body);
    m_builder.get_widget("attach", m_btn_attach);
    m_builder.get_widget("detach", m_btn_detach);
    m_builder.get_widget("btn_prev", m_btn_prev);
    m_builder.get_widget("btn_next", m_btn_next);
    m_builder.get_widget("close_btn_attached", m_btn_close_attached);
    m_builder.get_widget("close_btn_detached", m_btn_close_detached);

    m_builder.get_widget("tray_visible" , m_tray_visible);
    m_builder.get_widget("tray_on_start", m_tray_on_start);
    m_builder.get_widget("tray_on_close", m_tray_on_close);

    m_builder.get_widget("c_n_on_message", m_c_n_on_message);
    m_builder.get_widget("c_n_with_audio", m_c_n_with_audio);
    m_builder.get_widget("c_auto_away", m_c_auto_away);
    m_builder.get_widget("c_send_typing", m_c_send_typing);
    m_builder.get_widget("c_chat_logging", m_c_logging);

    m_builder.get_widget("ft_saveto", m_ft_save_to);
    m_builder.get_widget("ft_auto_accept", m_ft_auto_accept);
    m_builder.get_widget("ft_display_inline", m_ft_display_inline);

    m_builder.get_widget("cl_use_compact", m_cl_use_compact);
    m_builder.get_widget("cl_display_active", m_cl_display_active);

    m_builder.get_widget("connection_udp", m_connection_udp);
    m_builder.get_widget("connection_tcp", m_connection_tcp);
    m_builder.get_widget("proxy_type", m_proxy_type);
    m_builder.get_widget("proxy_host", m_proxy_host);
    m_builder.get_widget("proxy_port", m_proxy_port);

    m_builder.get_widget("t_color", m_t_color);

    m_builder.get_widget("p_remember", m_p_remember);

    m_builder.get_widget("video_default_device", m_video_device);

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
    m_main.config()->property_file_save_path().signal_changed().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_ft_save_to->set_file(Gio::File::create_for_path(m_main.config()->property_file_save_path().get_value()));
    }, *this));
    m_ft_save_to->set_file(Gio::File::create_for_path(m_main.config()->property_file_save_path().get_value()));
    m_ft_save_to->signal_file_set().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        m_main.config()->property_file_save_path() = m_ft_save_to->get_file()->get_path();
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
                             config::global().property_connection_tcp(),
                             m_connection_tcp->property_active(),
                             binding_flag));
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_connection_udp(),
                             m_connection_udp->property_active(),
                             binding_flag));

    //GLOBAL PROXY-SETTINGS
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_proxy_type(),
                             m_proxy_type->property_active_id(),
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
                             config::global().property_proxy_host(),
                             m_proxy_host->property_text(),
                             binding_flag));
    //TOOD find out why this fails ?
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_proxy_port(),
                             m_proxy_host->property_text(),
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
    m_bindings.push_back(Glib::Binding::bind_property(
                             config::global().property_video_default_device(),
                             m_video_device->property_active_id(),
                             binding_flag));
}

settings::~settings() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}
