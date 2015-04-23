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

WidgetNotification* WidgetNotification::create(DialogContact::EventAddNotification event) {
    auto builder = Gtk::Builder::create_from_resource("/org/gtox/ui/list_item_notification.ui");
    WidgetNotification* tmp = nullptr;
    builder->get_widget_derived("contact_list_item", tmp);
    tmp->set_event(event);
    tmp->show();
    return tmp;
}

WidgetNotification::WidgetNotification(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ListBoxRow(cobject),
      m_builder(builder) {

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

    Gtk::Label* title;
    Gtk::Label* message;
    Gtk::Image* image;
    Gtk::Image* icon;

    m_builder->get_widget("title", title);
    m_builder->get_widget("message", message);
    m_builder->get_widget("image", image);
    m_builder->get_widget("icon", icon);

    m_notify = std::make_shared<Notify::Notification>(m_event.title, m_event.message);

    title->set_text(m_event.title);
    message->set_text(m_event.message);
    if (m_event.image) {
        image->property_pixbuf() = m_event.image->scale_simple(
                                       64,
                                       64,
                                       Gdk::INTERP_BILINEAR);
        m_notify->set_image_from_pixbuf(image->property_pixbuf());
    } else {
        m_notify->set_image_from_pixbuf(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/icon_128.png"));
    }

    icon->property_pixbuf() = Gdk::Pixbuf::create_from_resource("/org/gtox/icon/notification.png");

    Gtk::Box* action_bar;
    m_builder->get_widget("action_bar", action_bar);
    //handle actions
    for (auto action : event.actions) {
        m_notify->add_action(action.first, action.first, [this, action](const Glib::ustring&){
            //Who knows if notification events are on the same thread as gtk ?
            Glib::signal_idle().connect_once([this, action](){
                ToxEventCallback::notify(action.second);
                Glib::signal_idle().connect_once([this](){
                    delete this;
                });
            });
        });
        auto action_btn = Gtk::manage(new Gtk::Button(action.first));
        action_btn->signal_clicked().connect([this, action](){
            ToxEventCallback::notify(action.second);
            Glib::signal_idle().connect_once([this](){
                delete this;
            });
        });
        action_btn->show();
        action_bar->add(*action_btn);
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
    ToxEventCallback::notify(m_event.activated_action);
    Glib::signal_idle().connect_once([this](){
        delete this;
    });
}
