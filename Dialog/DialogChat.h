#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <gtkmm.h>

class DialogChat: public Gtk::Window {
    private:
        Gtk::Paned     m_header_paned;
        Gtk::HeaderBar m_headerbar_chat;
        Gtk::HeaderBar m_headerbar_contact;

        Gtk::Paned    m_content_paned;
        Gtk::Notebook m_content_chat;
        Gtk::VBox     m_content_contact;

        Gtk::Label m_test1;
        Gtk::Label m_test2;
        Gtk::Label m_test3;
        Gtk::Label m_test4;
    public:
        DialogChat();
        ~DialogChat();
};

#endif
