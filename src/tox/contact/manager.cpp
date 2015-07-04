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
#include "exception.h"

using namespace toxmm2;

contact_manager::type_signal_request contact_manager::signal_request() { return m_signal_request; }
contact_manager::type_signal_removed contact_manager::signal_removed() { return m_signal_removed; }
contact_manager::type_signal_removed contact_manager::signal_added  () { return m_signal_added; }

contact_manager::contact_manager(std::shared_ptr<core> core):
    m_core(core) {
}

void contact_manager::init() {
    m_core->signal_contact_request().connect(sigc::track_obj([this](contactAddr contact_addr, Glib::ustring message) {
        signal_request().emit(contact_addr, message);
    }, *this));

    m_core->signal_contact_message().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring message) {
                                                 auto contact = find(contact_nr);
                                                 if (contact) {
                                                     contact->m_signal_recv_message.emit(message);
                                                 }
                                             }, *this));

    m_core->signal_contact_action().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring action) {
                                                auto contact = find(contact_nr);
                                                if (contact) {
                                                    contact->m_signal_recv_action.emit(action);
                                                }
                                            }, *this));

    m_core->signal_contact_name().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring name) {
                                              auto contact = find(contact_nr);
                                              if (contact) {
                                                  contact->m_property_name = name;
                                              }
                                          }, *this));

    m_core->signal_contact_status_message().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring message) {
                                                        auto contact = find(contact_nr);
                                                        if (contact) {
                                                            contact->m_property_status_message = message;
                                                        }
                                                    }, *this));

    m_core->signal_contact_status().connect(sigc::track_obj([this](contactNr contact_nr, TOX_USER_STATUS status) {
                                                auto contact = find(contact_nr);
                                                if (contact) {
                                                    contact->m_property_status = status;
                                                }
                                            }, *this));

    m_core->signal_contact_typing().connect(sigc::track_obj([this](contactNr contact_nr, bool is_typing) {
                                                auto contact = find(contact_nr);
                                                if (contact) {
                                                    contact->m_property_typing = is_typing;
                                                }
                                            }, *this));

    m_core->signal_contact_read_receipt().connect(sigc::track_obj([this](contactNr contact_nr, receiptNr receipt_nr) {
                                                      auto contact = find(contact_nr);
                                                      if (contact) {
                                                          contact->m_signal_receipt.emit(receipt_nr);
                                                      }
                                                  }, *this));

    m_core->signal_contact_connection_status().connect(sigc::track_obj([this](contactNr contact_nr, TOX_CONNECTION connection) {
                                                           auto contact = find(contact_nr);
                                                           if (contact) {
                                                               contact->m_property_connection = connection;
                                                           }
                                                       }, *this));

    //load contacts
    m_contact.resize(tox_self_get_friend_list_size(m_core->toxcore()));
    std::vector<uint32_t> tmp(m_contact.size());
    tox_self_get_friend_list(m_core->toxcore(), tmp.data());
    for (size_t i = 0; i < tmp.size(); ++i) {
        m_contact[i] = std::shared_ptr<contact>(new contact(m_core, contactNr(tmp[i])));
    }
}

void contact_manager::destroy() {
    m_contact.clear();
}

contact_manager::~contact_manager() {
}

std::shared_ptr<contact> contact_manager::find(contactAddrPublic addr) {
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

const std::vector<std::shared_ptr<contact>>& contact_manager::get_all() {
    return m_contact;
}

void contact_manager::add_contact(contactAddrPublic addr_public) {
    TOX_ERR_FRIEND_ADD error;
    auto nr = tox_friend_add_norequest(m_core->toxcore(), addr_public, &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw exception(error);
    }
    auto contact = std::shared_ptr<toxmm2::contact>(new toxmm2::contact(m_core, contactNr(nr)));
    m_contact.push_back(contact);
    m_signal_added.emit(contact);
}

void contact_manager::add_contact(contactAddr addr, const Glib::ustring& message) {
    TOX_ERR_FRIEND_ADD error;
    auto nr = tox_friend_add(m_core->toxcore(), addr, (const uint8_t*)message.data(), message.size(), &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw exception(error);
    }
    auto contact = std::shared_ptr<toxmm2::contact>(new toxmm2::contact(m_core, contactNr(nr)));
    m_contact.push_back(contact);
    m_signal_added.emit(contact);
}
