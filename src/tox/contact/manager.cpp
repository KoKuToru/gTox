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

using namespace toxmm;

contact_manager::type_signal_request contact_manager::signal_request() { return m_signal_request; }
contact_manager::type_signal_removed contact_manager::signal_removed() { return m_signal_removed; }
contact_manager::type_signal_removed contact_manager::signal_added  () { return m_signal_added; }

contact_manager::contact_manager(std::shared_ptr<toxmm::core> core):
    m_core(core) {
}

void contact_manager::init() {
    auto c = core();
    if (!c) {
        return;
    }

    c->signal_contact_request().connect(sigc::track_obj([this](contactAddrPublic contact_addr, Glib::ustring message) {
        signal_request().emit(contact_addr, message);
    }, *this));

    c->signal_contact_message().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring message) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_recv_message(message);
        }
    }, *this));

    c->signal_contact_action().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring action) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_recv_action(action);
        }
    }, *this));

    c->signal_contact_name().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring name) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_property_name = name;
        }
    }, *this));

    c->signal_contact_status_message().connect(sigc::track_obj([this](contactNr contact_nr, Glib::ustring message) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_property_status_message = message;
        }
    }, *this));

    c->signal_contact_status().connect(sigc::track_obj([this](contactNr contact_nr, TOX_USER_STATUS status) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_property_status = status;
        }
    }, *this));

    c->signal_contact_typing().connect(sigc::track_obj([this](contactNr contact_nr, bool is_typing) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_property_typing = is_typing;
        }
    }, *this));

    c->signal_contact_read_receipt().connect(sigc::track_obj([this](contactNr contact_nr, receiptNr receipt_nr) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_receipt(receipt_nr);
        }
    }, *this));

    c->signal_contact_connection_status().connect(sigc::track_obj([this](contactNr contact_nr, TOX_CONNECTION connection) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_property_connection = connection;
        }
    }, *this));

    c->signal_file_chunk_request().connect(sigc::track_obj([this](contactNr contact_nr, fileNr file_nr, uint64_t position, size_t length) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_send_file_chunk_rq(file_nr, position, length);
        }
    }, *this));

    c->signal_file_recv().connect(sigc::track_obj([this](contactNr contact_nr, fileNr file_nr, TOX_FILE_KIND file_kind, size_t file_size, Glib::ustring file_name) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_recv_file(file_nr, file_kind, file_size, file_name);
        }
    }, *this));

    c->signal_file_recv_chunk().connect(sigc::track_obj([this](contactNr contact_nr, fileNr file_nr, uint64_t position, const std::vector<uint8_t>& content) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_recv_file_chunk(file_nr, position, content);
        }
    }, *this));

    c->signal_file_recv_control().connect(sigc::track_obj([this](contactNr contact_nr, fileNr file_nr, TOX_FILE_CONTROL state) {
        auto contact = find(contact_nr);
        if (contact) {
            contact->m_signal_recv_file_control(file_nr, state);
        }
    }, *this));

    //load contacts
    m_contact.resize(tox_self_get_friend_list_size(c->toxcore()));
    std::vector<uint32_t> tmp(m_contact.size());
    tox_self_get_friend_list(c->toxcore(), tmp.data());
    for (size_t i = 0; i < tmp.size(); ++i) {
        m_contact[i] = std::shared_ptr<contact>(new contact(shared_from_this(), contactNr(tmp[i])));
        m_contact[i]->init();
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
    auto c = core();
    if (!c) {
        return;
    }

    TOX_ERR_FRIEND_ADD error;
    auto nr = tox_friend_add_norequest(c->toxcore(), addr_public, &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw exception(error);
    }
    auto contact = std::shared_ptr<toxmm::contact>(new toxmm::contact(shared_from_this(), contactNr(nr)));
    contact->init();
    m_contact.push_back(contact);
    m_signal_added(contact);
}

void contact_manager::add_contact(contactAddr addr, const std::string& message) {
    auto c = core();
    if (!c) {
        return;
    }

    TOX_ERR_FRIEND_ADD error;
    auto nr = tox_friend_add(c->toxcore(), addr, (const uint8_t*)message.data(), message.size(), &error);
    if (error != TOX_ERR_FRIEND_ADD_OK) {
        throw exception(error);
    }
    auto contact = std::shared_ptr<toxmm::contact>(new toxmm::contact(shared_from_this(), contactNr(nr)));
    contact->init();
    m_contact.push_back(contact);
    m_signal_added(contact);
}

void contact_manager::remove_contact(std::shared_ptr<contact> contact) {
    auto c = core();
    if (!c) {
        return;
    }

    TOX_ERR_FRIEND_DELETE error;
    tox_friend_delete(c->toxcore(),
                      contact->property_nr().get_value(),
                      &error);
    if (error != TOX_ERR_FRIEND_DELETE_OK) {
        throw exception(error);
    }
    auto iter = std::find(m_contact.begin(),
                          m_contact.end(),
                          contact);
    if (iter != m_contact.end()) {
        m_contact.erase(iter);
    }
    m_signal_removed(contact);
}

std::shared_ptr<toxmm::core> contact_manager::core() {
    return m_core.lock();
}
