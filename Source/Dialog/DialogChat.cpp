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
#include "Generated/icon.h"
#include "DialogContact.h"
#include "Widget/WidgetContactListItem.h"

DialogChat::DialogChat(Toxmm::FriendNr nr)
    : m_in_window(false),
      m_icon_attach(ICON::load_icon(ICON::chat_attach)),
      m_chat(nr) {
    this->set_border_width(1);
    this->set_default_geometry(256, 256);
    this->set_position(Gtk::WindowPosition::WIN_POS_NONE);

    // Setup titlebar
    m_header.set_title(Toxmm::instance().get_name_or_address(nr));
    m_header.set_subtitle(Toxmm::instance().get_status_message(nr));
    m_header.set_show_close_button();

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
            if (data.nr == nr && is_visible()) {
                ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{nr}));
            }
        } else if (ev.type() == typeid(Toxmm::EventFriendMessage)) {
            auto data = ev.get<Toxmm::EventFriendMessage>();
            if (data.nr == nr && is_visible()) {
                ToxEventCallback::notify(ToxEvent(WidgetContactListItem::EventStopSpin{nr}));
            }
        }
    };

    m_btn_xxtach.set_image(m_icon_attach);

    m_headerbar_btn_left.get_style_context()->add_class("linked");
    m_headerbar_btn_left.add(m_btn_xxtach);
    m_header.pack_start(m_headerbar_btn_left);

    //this->set_titlebar(m_header);

    // Setup content
    //add(m_chat);

    m_btn_xxtach.signal_clicked().connect([this]() {
        //DialogContact::instance().attach_chat(m_chat.get_friend_nr());
    });

    m_chat.show_all();
    m_header.show_all();
    ToxEventCallback::notify(ToxEvent(DialogContact::EventAttachWidget{
                                          &m_header,
                                          &m_chat
                                      }));
}

DialogChat::~DialogChat() {
}

void DialogChat::show() {
    ToxEventCallback::notify(ToxEvent(DialogContact::EventDetachWidget{
                                          true,
                                          &m_header,
                                          &m_chat
                                      }));
    add(m_chat);
    Gtk::Window::show();
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
        //no idea
        return true;
    } else {
        return m_chat.get_mapped();
    }
}
