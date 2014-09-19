#ifndef WIDGETCONTACT_H
#define WIDGETCONTACT_H

#include <gtkmm.h>

#include "WidgetContactListItem.h"

class WidgetContact: public Gtk::ScrolledWindow {
    private:
        Gtk::ListBox m_list;

        std::vector<std::shared_ptr<WidgetContactListItem>> items;
    public:
        WidgetContact();
        ~WidgetContact();
};

#endif
