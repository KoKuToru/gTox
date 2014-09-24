#ifndef WIDGETCHATBOX_H
#define WIDGETCHATBOX_H

#include <gtkmm.h>

class WidgetChatBox: public Gtk::Box{
    private:
        Gtk::ScrolledWindow m_ScrolledWindow;
        Gtk::TextView m_TextView;
        Glib::RefPtr<Gtk::TextBuffer> m_TextBuffer;
    public:
        WidgetChatBox();
        ~WidgetChatBox();
};

#endif
