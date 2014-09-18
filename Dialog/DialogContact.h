#ifndef DIALOGCONTACT_H
#define DIALOGCONTACT_H

#include <gtkmm.h>

#include "../Widget/WidgetChat.h"
#include "../Widget/WidgetContact.h"

#include "DialogChat.h"

//contact list with pinned chat
class DialogContact: public Gtk::Window {
    private:
        Gtk::Paned     m_header_paned;
        Gtk::HeaderBar m_headerbar_chat;
        Gtk::HeaderBar m_headerbar_contact;

        Gtk::Paned     m_paned;

        Gtk::Image     m_icon_attach;
        Gtk::Image     m_icon_detach;

        Gtk::Button    m_btn_xxtach;

        Gtk::Box       m_headerbar_btn_right;

        WidgetChat m_chat;
        WidgetContact m_contact;

        DialogChat     m_chat_dialog; //probably a list in the future
    public:
        DialogContact();
        ~DialogContact();

    protected:
        void detachChat();
};

#endif
