#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <gtkmm.h>

#include "../Widget/WidgetChat.h"

//Single chat window
class DialogChat: public Gtk::Window {
    private:
        Gtk::HeaderBar m_header;

        WidgetChat m_chat;

    public:
        DialogChat();
        ~DialogChat();

        void show();
};

#endif
