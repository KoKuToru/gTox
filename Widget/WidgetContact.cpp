#include "WidgetContact.h"

WidgetContact::WidgetContact() {
    //test data
    m_test.set_text("Contact Nr. 1");
    m_list.add(m_test);

    this->add(m_list);
}

WidgetContact::~WidgetContact() {

}
