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
#include "DialogChat.h"
#include "DialogContact.h"
#include "Widget/WidgetContactListItem.h"
#include "Helper/Canberra.h"

DialogChat::DialogChat(Toxmm::FriendNr nr)
    : m_in_window(false),
      m_chat(nr) {
    this->set_border_width(1);
    this->set_default_geometry(256, 256);
    this->set_position(Gtk::WindowPosition::WIN_POS_NONE);

    // Setup titlebar
    m_header.set_title(Toxmm::instance().get_name_or_address(nr));
    m_header.set_subtitle(Toxmm::instance().get_status_message(nr));
    m_header.property_hexpand() = true;

    // custom close button for chat
    auto x_box = Gtk::manage(new Gtk::Box());
    auto btn_x = Gtk::manage(new Gtk::Button());
    x_box->add(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)));
    btn_x->set_image_from_icon_name("window-close-symbolic");
    btn_x->get_style_context()->add_class("titlebutton");
    btn_x->set_valign(Gtk::ALIGN_CENTER);
    x_box->add(*btn_x);
    x_box->show_all();
    x_box->get_style_context()->add_class("right");
    x_box->set_spacing(6);
    m_header.pack_end(*x_box);

    set_titlebar(m_header_box);
    m_header_box.property_hexpand() = true;
    m_header_box.show();

    btn_x->signal_clicked().connect([this, nr](){
        Gtk::Window::hide();
        Glib::signal_idle().connect_once([nr](){
            ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventDestroyChat{nr}));
        });
    });

    m_tox_callback = [this, nr](const ToxEvent& ev) {
        if (ev.type() == typeid(Toxmm::EventName))  {
            auto data = ev.get<Toxmm::EventName>();
            if (data.nr == nr) {
                m_header.set_title(Toxmm::instance().get_name_or_address(nr));
            }
        } else if (ev.type() == typeid(Toxmm::EventStatusMessage)) {
            auto data = ev.get<Toxmm::EventStatusMessage>();
            if (data.nr == nr) {
                m_header.set_subtitle(Toxmm::instance().get_status_message(nr));
            }
        } else if (ev.type() == typeid(Toxmm::EventFriendAction)) {
            auto data = ev.get<Toxmm::EventFriendAction>();
            if (data.nr == nr) {
                if (is_visible()) {
                    ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{nr}));
                } else {
                    notify(Toxmm::instance().get_name_or_address(nr),
                           data.message);
                }
            }
        } else if (ev.type() == typeid(Toxmm::EventFriendMessage)) {
            auto data = ev.get<Toxmm::EventFriendMessage>();
            if (data.nr == nr) {
                if (is_visible()) {
                    ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{nr}));
                } else {
                    notify(Toxmm::instance().get_name_or_address(nr),
                           data.message);
                }
            }
        }
    };

    m_btn_xxtach.set_image_from_icon_name("chat-detach-symbolic");

    m_headerbar_btn_left.get_style_context()->add_class("linked");
    m_headerbar_btn_left.add(m_btn_xxtach);
    m_header.pack_start(m_headerbar_btn_left);

    add(m_chat_box);
    m_chat_box.property_expand() = true;
    m_chat_box.show();

    m_btn_xxtach.signal_clicked().connect([this]() {
        if (m_in_window) {
            m_btn_xxtach.set_image_from_icon_name("chat-attach-symbolic");
            hide();
            present();
        } else {
            m_btn_xxtach.set_image_from_icon_name("chat-detach-symbolic");
            show();
        }
    });

    m_chat.property_expand() = true;
    m_chat.show_all();
    m_header.show_all();
    /*ToxEventCallback::notify(ToxEvent(DialogContact::EventAttachWidget{
                                          &m_header,
                                          &m_chat
                                      }));*/
}

DialogChat::~DialogChat() {
    if (!m_in_window) {
        int x,y,w,h;
        ToxEventCallback::notify(ToxEvent(DialogContact::EventDetachWidget{
                                              false,
                                              &m_header,
                                              &m_chat,
                                              x,
                                              y,
                                              w,
                                              h
                                          }));
    }
    if (!m_notify) {
        try  {
            m_notify->close();
        } catch (...){
            //ignore
        }
    }
}

void DialogChat::show() {
    if (!m_in_window) {
        int x,y,w,h;
        ToxEventCallback::notify(ToxEvent(DialogContact::EventDetachWidget{
                                              true,
                                              &m_header,
                                              &m_chat,
                                              x,
                                              y,
                                              w,
                                              h
                                          }));
        move(x, y);
        resize(w, h);
        m_header_box.add(m_header);
        m_chat_box.add(m_chat);
        m_in_window = true;
        m_btn_xxtach.set_image_from_icon_name("chat-attach-symbolic");
    }
    Gtk::Window::show();
}

void DialogChat::hide() {
    if (m_in_window) {
        m_header_box.remove(m_header);
        m_chat_box.remove(m_chat);
        m_in_window = false;

        ToxEventCallback::notify(ToxEvent(DialogContact::EventAttachWidget{
                                              &m_header,
                                              &m_chat
                                          }));
        m_btn_xxtach.set_image_from_icon_name("chat-detach-symbolic");
    }
    Gtk::Window::hide();
}

void DialogChat::present() {
    if (m_in_window) {
        Gtk::Window::present();
    } else {
        ToxEventCallback::notify(ToxEvent(DialogContact::EventPresentWidget{
                                              &m_header,
                                              &m_chat
                                          }));
    }
    ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{m_chat.get_friend_nr()}));
}

bool DialogChat::is_visible() {
    if (m_in_window) {
        return property_is_active();
    } else {
        return m_chat.get_mapped();
    }
}

void DialogChat::notify(const Glib::ustring& title, const Glib::ustring& message) {
    //Notification
    if (!m_notify) {
        m_notify = std::make_shared<Notify::Notification>(title, message);
        m_notify->set_image_from_pixbuf(Gdk::Pixbuf::create_from_resource("/org/gtox/icon/avatar.svg")
                                        ->scale_simple(
                                            64,
                                            64,
                                            Gdk::INTERP_BILINEAR));
        m_notify->signal_closed().connect([this](){
            //m_notify->get_closed_reason() ???
            present();
        });
        try {
            m_notify->show();
        } catch (...) {
            //ignore
        }
    } else {
        m_notify->update(title, message);
        try {
            m_notify->show();
        } catch (...) {
            //ignore
        }
    }
    //Sound:
    Canberra::play("message-new-instant");
}
