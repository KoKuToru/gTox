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
#include "file_recv.h"
#include "exception.h"

using namespace toxmm;

file_recv::file_recv(std::shared_ptr<toxmm::file_manager> manager):
    Glib::ObjectBase(typeid(file_recv)),
    file(manager) {
}

void file_recv::resume() {
    if (m_stream) {
        m_stream.reset();
    }
    auto file = Gio::File::create_for_path(property_path().get_value());
    m_stream = file->append_to();
    m_stream->seek(0, Glib::SEEK_TYPE_END);
    m_stream->seek(std::min(goffset(property_size()), m_stream->tell()),
                   Glib::SEEK_TYPE_SET);
    m_stream->truncate(m_stream->tell());
    try {
        seek(uint64_t(m_stream->tell()));
    } catch (toxmm::exception) {
        //this is pretty stupid
        //when we pause and then resume
        //seek is not possible !
        //
        //we effectly ignore the error now
        //if we can seek we seek
        //if we can't seek we think we seeked
        //
        //it looks like we can only seek once each file before resume
        //but we have no way of knowing here if it's the first time we resume
        //or the second time..
        //so we will just try and hope
    }
}

void file_recv::send_chunk_request(uint64_t, size_t) {
    throw std::runtime_error("file_recv::send_chunk_request");
}

void file_recv::recv_chunk(uint64_t position, const std::vector<uint8_t>& chunk) {
    if (!m_stream) {
        return;
    }
    //TODO: make async
    m_stream->seek(position, Glib::SEEK_TYPE_SET);
    m_stream->write_bytes(Glib::Bytes::create(chunk.data(), chunk.size()));
}

void file_recv::finish() {
    if (m_stream) {
        m_stream.reset();
    }
}

void file_recv::abort() {
    if (m_stream) {
        m_stream.reset();
    }
    auto file = Gio::File::create_for_path(property_path().get_value());
    try {
        file->remove();
    } catch (...) {
        //ignore
    }
}

bool file_recv::is_recv() {
    return true;
}
