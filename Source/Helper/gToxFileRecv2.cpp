/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#include "gToxFileRecv2.h"

bool gToxFileRecv2::is_recv() {
    return true;
}

void gToxFileRecv2::deactivate() {
    m_stream.reset();
    if (state() == gToxFileTransf::CANCEL) {
        Gio::File::create_for_path(path())->remove();
    }
}

void gToxFileRecv2::activate() {
    m_stream = Gio::File::create_for_path(path())->append_to();
}

void gToxFileRecv2::recv_chunk(uint64_t position, const std::vector<uint8_t>& data, std::function<void()> callback) {
    //TODO make async !
    m_stream->seek(position, Glib::SEEK_TYPE_SET);
    m_stream->write_bytes(Glib::Bytes::create(data.data(), data.size()));
    callback();
}

void gToxFileRecv2::send_chunk(uint64_t, size_t, std::function<void(const std::vector<uint8_t>&)>) {
    throw std::runtime_error("gToxFileRecv2 send_chunk not implemented");
}

void gToxFileRecv2::resume() {
    auto manager = m_manager.lock();
    if (manager) {
        manager->resume(this);
    }
}

void gToxFileRecv2::pause() {
    auto manager = m_manager.lock();
    if (manager) {
        manager->pause(this);
    }
}

void gToxFileRecv2::cancel() {
    auto manager = m_manager.lock();
    if (manager) {
        manager->cancel(this);
    }
}
