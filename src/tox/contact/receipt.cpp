#include "receipt.h"
#include "contact.h"

using namespace toxmm;

receipt::type_signal_okay receipt::signal_okay() { return m_signal_okay; }
receipt::type_signal_lost receipt::signal_lost() { return m_signal_lost; }

Glib::PropertyProxy_ReadOnly<receiptNr> receipt::property_nr()
{ return {this, "receipt-nr"}; }

receipt::receipt(std::shared_ptr<contact> contact, receiptNr nr):
    Glib::ObjectBase(typeid(receipt)),
    m_property_nr(*this, "receipt-nr") {
    m_property_nr = nr;

    m_connection = contact->signal_receipt().connect(sigc::track_obj([this](receiptNr nr) {
        if (property_nr().get_value() < nr) {
            m_signal_lost();
            m_connection.disconnect();
        } else if (property_nr().get_value() == nr) {
            m_signal_okay();
            m_connection.disconnect();
        }
    }, *this));
}
