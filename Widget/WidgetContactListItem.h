#ifndef WIDGETCONTACTLISTITEM_H
#define WIDGETCONTACTLISTITEM_H

#include <gtkmm.h>

class WidgetContactListItem: public Gtk::ListBoxRow {
    private:
        Gtk::Image  m_avatar;
        Gtk::Label  m_name;
        Gtk::Label  m_status;

        Gtk::Grid   m_layout;
    public:
        WidgetContactListItem();
        ~WidgetContactListItem();
};

#endif
