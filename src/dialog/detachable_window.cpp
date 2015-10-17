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
#include "detachable_window.h"
#include "main.h"
using namespace dialog;

auto detachable_window::property_has_focus()          -> Glib::PropertyProxy<bool> {
    return m_prop_has_focus.get_proxy();
}

auto detachable_window::property_is_attached()        -> Glib::PropertyProxy<bool> {
    return m_prop_is_attached.get_proxy();
}

auto detachable_window::property_headerbar_title()    -> Glib::PropertyProxy<Glib::ustring> {
    return m_prop_headerbar_title.get_proxy();
}

auto detachable_window::property_headerbar_subtitle() -> Glib::PropertyProxy<Glib::ustring> {
    return m_prop_headerbar_subtitle.get_proxy();
}

auto detachable_window::property_body()               -> Glib::PropertyProxy<Gtk::Widget*> {
    return m_prop_body.get_proxy();
}

auto detachable_window::property_headerbar()          -> Glib::PropertyProxy_ReadOnly<Gtk::HeaderBar*> {
    return {this, "detachable-window-headerbar"};
}

detachable_window::type_signal_close detachable_window::signal_close() {
    return m_signal_close;
}

detachable_window::detachable_window(type_slot_detachable_add main_add,
                                     type_slot_detachable_del main_del)
    : Glib::ObjectBase(typeid(detachable_window)),
      m_builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_detachable.ui")),
      m_prop_has_focus(*this, "detachable-window-has-focus", false),
      m_prop_is_attached(*this, "detachable-window-is-attached", true),
      m_prop_headerbar_title(*this, "detachable-window-headerbar-title"),
      m_prop_headerbar_subtitle(*this, "detachable-window-headerbar-subtitle"),
      m_prop_body(*this, "detachable-window-body"),
      m_prop_headerbar(*this, "detachable-window-headerbar") {
    utils::debug::scope_log(DBG_LVL_1("gtox"), {});
    m_builder.get_widget("headerbar_attached", m_headerbar_attached);
    m_builder.get_widget("headerbar_detached", m_headerbar_detached);
    m_builder.get_widget("attach", m_btn_attach);
    m_builder.get_widget("detach", m_btn_detach);
    m_builder.get_widget("btn_attached", m_btn_close_attached);
    m_builder.get_widget("btn_detached", m_btn_close_detached);

    m_prop_headerbar = m_headerbar_attached;

    auto bind_flags = Glib::BINDING_DEFAULT | Glib::BINDING_SYNC_CREATE;
    m_bindings.push_back(Glib::Binding::bind_property(
                             property_headerbar_title(),
                             m_headerbar_attached->property_title(),
                             bind_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             property_headerbar_title(),
                             m_headerbar_detached->property_title(),
                             bind_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             property_headerbar_subtitle(),
                             m_headerbar_attached->property_subtitle(),
                             bind_flags));
    m_bindings.push_back(Glib::Binding::bind_property(
                             property_headerbar_subtitle(),
                             m_headerbar_detached->property_subtitle(),
                             bind_flags));

    m_btn_detach->signal_clicked().connect(sigc::track_obj([this, main_del]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        Gtk::Widget* body = property_body().get_value();
        Gtk::Window* parent = dynamic_cast<Gtk::Window *>(body->get_toplevel());
        if (parent) {
            auto gravity = parent->get_gravity();
            int x, y, w, h;
            parent->set_gravity(Gdk::GRAVITY_NORTH_WEST);
            parent->get_position(x, y);
            parent->get_size(w, h);
            parent->set_gravity(gravity);
            move(x, y);
            resize(body->get_width(), h);
        }
        main_del(this);
        add(*body);
        show();
    }, *this));
    m_btn_attach->signal_clicked().connect(sigc::track_obj([this, main_add]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        remove();
        main_add(this);
        hide();
    }, *this));
    m_btn_close_attached->signal_clicked().connect(sigc::track_obj([this, main_del]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        main_del(this);
        m_signal_close();
    }, *this));
    m_btn_close_detached->signal_clicked().connect(sigc::track_obj([this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        remove();
        hide();
        m_signal_close();
    }, *this));

    auto update_has_focus = [this]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto body = property_body().get_value();
        auto parent = dynamic_cast<Gtk::Window *>(body->get_toplevel());

        m_prop_has_focus = (property_is_attached()
                            && parent
                            && parent->property_is_active()
                            && body->get_mapped())
                           || property_is_active().get_value();
    };

    property_body()
            .signal_changed()
            .connect(sigc::track_obj([this,
                                     main_add,
                                     main_del,
                                     update_has_focus]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        auto body = property_body().get_value();

        if (property_is_attached()) {
            main_del(this);
            if (body) {
                main_add(this);
            }
        } else {
            remove();
            if (body) {
                add(*property_body());
            }
        }

        if (body) {
            body->signal_map()
                    .connect_notify(
                        sigc::track_obj(update_has_focus, *this, *body));
            body->signal_unmap()
                    .connect_notify(
                        sigc::track_obj(update_has_focus, *this, *body));
        }
    }, *this));

    property_is_active()
            .signal_changed()
            .connect(sigc::track_obj(update_has_focus, *this));

    set_titlebar(*m_headerbar_detached);
}

detachable_window::~detachable_window() {
    utils::debug::scope_log(DBG_LVL_1("gtox"), {});
}
