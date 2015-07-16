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
#include "contact/contact.h"
#include "contact/manager.h"
#include "core.h"

using namespace toxmm2;

file_manager::type_signal_recv          file_manager::signal_recv_file() { return m_signal_recv_file; }
file_manager::type_signal_send          file_manager::signal_send_file() { return m_signal_send_file; }
file_manager::type_signal_recv_chunk    file_manager::signal_recv_chunk() { return m_signal_recv_chunk; }
file_manager::type_signal_send_chunk    file_manager::signal_send_chunk() { return m_signal_send_chunk; }
file_manager::type_signal_send_chunk_rq file_manager::signal_send_chunk_request() { return m_signal_send_chunk_rq; }
file_manager::type_signal_recv_control  file_manager::signal_recv_control() { return m_signal_recv_control; }
file_manager::type_signal_send_control  file_manager::signal_send_control() { return m_signal_send_control; }

file_manager::file_manager(std::shared_ptr<toxmm2::contact> contact)
    : m_contact(contact) {

}

void file_manager::init() {
    auto ct = contact();
    if (!ct) {
        return;
    }

    ct->signal_send_file_chunk_request().connect(sigc::track_obj([this](fileNr nr, uint64_t position, size_t length) {
       /*auto file = find(nr);
       if (file) {
           file->
       }*/
    }, *this));

    ct->signal_recv_file().connect(sigc::track_obj([this](fileNr nr, TOX_FILE_KIND kind, size_t size, Glib::ustring name) {
       /*auto file = find(nr);
       if (file) {
           file->
       }*/
    }, *this));

    ct->signal_recv_file_chunk().connect(sigc::track_obj([this](fileNr nr, uint64_t position, const std::vector<uint8_t>& content) {
       /*auto file = find(nr);
       if (file) {
           file->
       }*/
    }, *this));
}

std::shared_ptr<toxmm2::core> file_manager::core() {
    auto ct = contact();
    if (!ct) {
        return nullptr;
    }
    return ct->core();
}

std::shared_ptr<toxmm2::contact_manager> file_manager::contact_manager() {
    auto ct = contact();
    if (!ct) {
        return nullptr;
    }
    return ct->contact_manager();
}

std::shared_ptr<toxmm2::contact> file_manager::contact() {
    return m_contact.lock();
}
