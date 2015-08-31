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
#include "file_send.h"
#include "core.h"
#include "contact/contact.h"
#include "exception.h"

using namespace toxmm;

file_send::file_send(std::shared_ptr<toxmm::file_manager> manager):
    Glib::ObjectBase(typeid(file_send)),
    file(manager) {
}

void file_send::resume() {
    m_queue.clear();
    if (m_stream) {
        m_stream.reset();
    }
    auto file = Gio::File::create_for_path(property_path().get_value());
    if (file->query_exists()) {
        m_stream = file->read();
    }
}

void file_send::send_chunk_request(uint64_t position, size_t size) {
    if (!m_stream) {
        return;
    }

    m_queue.push_back({position, size});

    iterate();
}

void file_send::iterate() {
    while (!m_queue.empty()) {
        auto last     = m_queue.front();
        auto position = last.first;
        auto size     = last.second;

        //TODO: make async
        if (m_stream) {
            m_stream->seek(position, Glib::SEEK_TYPE_SET);
        }
        std::vector<uint8_t> chunk(size);
        gsize dummy;
        if (m_stream) {
            m_stream->read_all(chunk.data(), chunk.size(), dummy);
        }

        //send
        auto c  = core();
        auto ct = contact();
        if (!c || !ct) {
            return;
        }

        TOX_ERR_FILE_SEND_CHUNK error;
        tox_file_send_chunk(c->toxcore(),
                            ct->property_nr().get_value(),
                            property_nr().get_value(),
                            position,
                            chunk.data(),
                            chunk.size(),
                            &error);
        switch (error) {
            case TOX_ERR_FILE_SEND_CHUNK_SENDQ:
                //try again later !
                Glib::signal_idle().connect_once(sigc::track_obj([this]() {
                    iterate();
                }, *this));
                return;
            case TOX_ERR_FILE_SEND_CHUNK_OK:
                break;
            default:
                throw toxmm::exception(error);
        }

        m_queue.pop_front();
    }
}

void file_send::recv_chunk(uint64_t, const std::vector<uint8_t>&) {
    throw std::runtime_error("file_recv::send_chunk_request");
}

void file_send::finish() {
    m_queue.clear();
    if (m_stream) {
        m_stream.reset();
    }
}

void file_send::abort() {
    m_queue.clear();
    if (m_stream) {
        m_stream.reset();
    }
}

bool file_send::is_recv() {
    return false;
}
