#ifndef WIDGETCHAT_H
#define WIDGETCHAT_H

#include <gtkmm.h>
#include "WidgetChatBox.h"

//Content of DialogChat
class WidgetChat: public Gtk::VPaned {
    private:
        WidgetChatBox input;
        WidgetChatBox output;
    public:
        WidgetChat();
        ~WidgetChat();
};

#endif
