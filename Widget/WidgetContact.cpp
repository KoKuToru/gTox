#include "WidgetContact.h"

WidgetContact::WidgetContact() {
    //test data
    for(int i = 0; i < 20; ++i) {
        items.emplace_back(new WidgetContactListItem);
        m_list.add(*(items.back().get()));
    }
    //this->add(m_list);
    this->add(m_list);//, true, true);
}

WidgetContact::~WidgetContact() {

}
