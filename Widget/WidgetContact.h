#ifndef WIDGETCONTACT_H
#define WIDGETCONTACT_H

#include <gtkmm.h>

class WidgetContact: public Gtk::Box {
    private:
        Gtk::VBox   m_list;
        Gtk::Label  m_test;
    public:
        WidgetContact();
        ~WidgetContact();
};

#endif
