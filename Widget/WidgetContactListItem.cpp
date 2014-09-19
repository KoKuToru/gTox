#include "WidgetContactListItem.h"

WidgetContactListItem::WidgetContactListItem() {
    m_name.set_text("Contact Nr. ?");
    m_status.set_text("Status..");

    m_avatar.set_size_request(64, 64);

    m_layout.attach(m_avatar, 0, 0, 1, 2);
    m_layout.attach(m_name, 1, 0, 1, 1);
    m_layout.attach(m_status, 1, 1, 1, 1);

    m_name.set_halign(Gtk::Align::ALIGN_START);
    m_status.set_halign(Gtk::Align::ALIGN_START);

    this->add(m_layout);
}

WidgetContactListItem::~WidgetContactListItem() {

}
