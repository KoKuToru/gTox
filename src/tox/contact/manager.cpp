/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
**/
#include "manager.h"
#include "core.h"
#include "contact.h"

using namespace toxmm2;

contact_manager::type_signal_request contact_manager::signal_request() { return m_signal_request; }
contact_manager::type_signal_removed contact_manager::signal_removed() { return m_signal_removed; }

contact_manager::contact_manager(std::shared_ptr<core> core):
    m_core(core) {
}

void contact_manager::init() {
    m_connections.push_back(
                m_core->signal_contact_request().connect([this](contactAddr contact_addr, Glib::ustring message) {
                    signal_request().emit(contact_addr, message);
                }));
    m_connections.push_back(
                m_core->signal_contact_message().connect([this](contactNr contact_nr, Glib::ustring message) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_signal_new_message.emit(message);
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_action().connect([this](contactNr contact_nr, Glib::ustring action) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_signal_new_action.emit(action);
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_name().connect([this](contactNr contact_nr, Glib::ustring name) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_property_name = name;
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_status_message().connect([this](contactNr contact_nr, Glib::ustring message) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_property_status_message = message;
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_status().connect([this](contactNr contact_nr, TOX_USER_STATUS status) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_property_status = status;
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_typing().connect([this](contactNr contact_nr, bool is_typing) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_property_typing = is_typing;
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_read_receipt().connect([this](contactNr contact_nr, receiptNr receipt_nr) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_signal_receipt.emit(receipt_nr);
                    }
                }));
    m_connections.push_back(
                m_core->signal_contact_connection_status().connect([this](contactNr contact_nr, TOX_CONNECTION connection) {
                    auto contact = find(contact_nr);
                    if (contact) {
                        contact->m_property_connection = connection;
                    }
                }));
}

void contact_manager::destroy() {
    m_contact.clear();
}

contact_manager::~contact_manager() {
    //disconnect all signals
    for (auto connection : m_connections) {
        connection.disconnect();
    }
}

std::shared_ptr<contact> contact_manager::find(contactAddr addr) {
    std::shared_ptr<contact> res;
    auto iter = std::find_if(m_contact.begin(), m_contact.end(), [addr](auto contact) {
        return contact->m_property_addr.get_value() == addr;
    });
    if (iter != m_contact.end()) {
        res = *iter;
    }
    return res;
}

std::shared_ptr<contact> contact_manager::find(contactNr nr) {
    std::shared_ptr<contact> res;
    auto iter = std::find_if(m_contact.begin(), m_contact.end(), [nr](auto contact) {
        return contact->m_property_nr.get_value() == nr;
    });
    if (iter != m_contact.end()) {
        res = *iter;
    }
    return res;
}
