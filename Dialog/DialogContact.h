#ifndef DIALOGCONTACT_H
#define DIALOGCONTACT_H

#include <gtkmm.h>

#include "../Widget/WidgetChat.h"
#include "../Widget/WidgetContact.h"

//contact list with pinned chat
class DialogContact: public Gtk::Window {
    private:
        Gtk::Paned     m_header_paned;
        Gtk::HeaderBar m_headerbar_chat;
        Gtk::HeaderBar m_headerbar_contact;

        Gtk::Paned    m_paned;

        WidgetChat m_chat;
        WidgetContact m_contact;

    public:
        DialogContact();
        ~DialogContact();
};

#endif
