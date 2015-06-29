/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "WidgetNotification.h"

gToxBuilderRef<WidgetNotification> WidgetNotification::create(gToxObservable* instance, DialogContact::EventAddNotification event) {
    return gToxBuilder(Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_notification.ui"))
            .get_widget_derived<WidgetNotification>("contact_list_item",
                                                    instance,
                                                    event);
}

WidgetNotification::WidgetNotification(BaseObjectType* cobject, gToxBuilder builder,
                                       gToxObservable* observable,
                                       DialogContact::EventAddNotification event)
    : Gtk::ListBoxRow(cobject),
      gToxObserver(observable) {
    builder.get_widget("title", m_title);
    builder.get_widget("message", m_message);
    builder.get_widget("image", m_image);
    builder.get_widget("icon", m_icon);
    builder->get_widget("action_bar", m_action_bar);
    set_event(event);
    show();
}

WidgetNotification::~WidgetNotification() {
    if (m_notify) {
        m_notify->signal_closed().connect([](){});
        m_notify->close();
        m_notify.reset();
    }
}

void WidgetNotification::set_event(DialogContact::EventAddNotification event) {
    m_event = event;

    m_notify = std::make_shared<Notify::Notification>(m_event.title, m_event.message);

    m_title->set_text(m_event.title);
    m_message->set_text(m_event.message);
    if (m_event.image) {
        m_image->property_pixbuf() = m_event.image->scale_simple(
                                       64,
                                       64,
                                       Gdk::INTERP_BILINEAR);
        m_notify->set_image_from_pixbuf(m_image->property_pixbuf());
    } else {
        m_notify->set_image_from_pixbuf(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/icon_128.svg"));
    }

    m_icon->property_pixbuf() = Gdk::Pixbuf::create_from_resource("/org/gtox/icon/notification.svg");

    //handle actions
    for (auto action : event.actions) {
        m_notify->add_action(action.first, action.first, [this, action](const Glib::ustring&){
            //Who knows if notification events are on the same thread as gtk ?
            Glib::signal_idle().connect_once([this, action](){
                observer_notify(action.second);
                Glib::signal_idle().connect_once([this](){
                    delete this;
                });
            });
        });
        auto action_btn = Gtk::manage(new Gtk::Button(action.first));
        action_btn->signal_clicked().connect([this, action](){
            observer_notify(action.second);
            Glib::signal_idle().connect_once([this](){
                delete this;
            });
        });
        action_btn->show();
        m_action_bar->add(*action_btn);
    }

    m_notify->signal_closed().connect([this](){
        activated();
    });

    if (m_event.show_on_desktop) {
        try {
            m_notify->show();
        } catch (...) {
            //Probably a desktop without Notification system
            m_notify.reset();
        }
    }
}

void WidgetNotification::activated() {
    observer_notify(m_event.activated_action);
    Glib::signal_idle().connect_once([this](){
        delete this;
    });
}
