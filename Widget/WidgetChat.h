#ifndef WIDGETCHAT_H
#define WIDGETCHAT_H

#include <gtkmm.h>

//Content of DialogChat
class WidgetChat: public Gtk::Box {
    private:
        Gtk::Label m_test;

    public:
        WidgetChat();
        ~WidgetChat();
};

#endif
