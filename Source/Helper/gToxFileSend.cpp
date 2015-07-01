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
#include "gToxFileSend.h"
#include <iostream>

gToxFileSend::gToxFileSend(gToxObservable* observable,
                           Toxmm::FriendNr nr,
                           TOX_FILE_KIND kind,
                           Glib::ustring path)
    : gToxObserver(observable),
      m_friend_nr(nr),
      m_path(path),
      m_kind(kind) {

    //New file transfer
    m_nr = tox().file_send(nr, kind, path);
    m_id = tox().file_get_file_id(nr, m_nr);
    m_file = Gio::File::create_for_path(path);
    m_stream = m_file->read();
    m_size = m_stream->query_info()->get_size();
}

gToxFileSend::~gToxFileSend() {
    try {
        pause();
    } catch (...) {}
}

void gToxFileSend::get_progress(uint64_t& position, uint64_t& size) {
    position = m_position;
    size = m_size;
}

void gToxFileSend::emit_progress() {
    if (m_position == m_size) {
        m_state = STOPPED;
        if (m_stream) {
            m_stream->close();
            m_stream.reset();

            auto addr = tox().get_address(m_friend_nr);
            /*tox().database().toxcore_log_set_file_sent(
                        Toxmm::to_hex(addr.data(), addr.size()),
                        m_path,
                        m_nr,
                        m_id);
                        */
        }
    }
    observer_notify(ToxEvent(EventFileProgress{
                                 this,
                                 m_friend_nr,
                                 m_path,
                                 m_nr,
                                 m_position,
                                 m_size
                             }));
}

void gToxFileSend::resume() {
    if (!m_stream || (m_state != PAUSED && m_state != INITIAL)) {
        return;
    }

    m_state = SENDING;

    tox().file_control(m_friend_nr, m_nr, TOX_FILE_CONTROL_RESUME);
}

void gToxFileSend::cancel() {
    if (!m_stream || (m_state != STOPPED && m_state != INITIAL)) {
        return;
    }

    m_stream->close();
    m_stream.reset();

    try {
        m_file->trash();
    } catch (...) {
        m_file->remove();
    }

    m_state = STOPPED;
    tox().file_control(m_friend_nr, m_nr, TOX_FILE_CONTROL_CANCEL);

    auto addr = tox().get_address(m_friend_nr);

    /*tox().database().toxcore_log_set_file_sent(
                Toxmm::to_hex(addr.data(), addr.size()),
                m_path,
                m_nr,
                m_id);
                */
}

void gToxFileSend::pause() {
    if (!m_stream || m_state != SENDING) {
        return;
    }
    m_state = PAUSED;
    tox().file_control(m_friend_nr, m_nr, TOX_FILE_CONTROL_PAUSE);
}

void gToxFileSend::observer_handle(const ToxEvent& ev) {
    if (!m_stream) {
        return;
    }

    /*
    if (ev.type() != typeid(Toxmm::EventFileSendChunk)) {
        return;
    }

    auto data = ev.get<Toxmm::EventFileRecvChunk>();
    if (data.nr != m_file.nr || data.file_number != m_file.file_number) {
        return;
    }

    //Handle download
    if (lseek(m_fd, data.file_position, SEEK_SET) != (__off_t)data.file_position) {
        throw std::runtime_error("gToxFileSend couldn't seek file");
    }

    auto ret = write(m_fd, data.file_data.data(), data.file_data.size());
    if (ret != ssize_t(data.file_data.size())) {
        throw std::runtime_error("gToxFileSend couldn't write file");
    }

    m_position = data.file_position + data.file_data.size();

    emit_progress();*/
}

Glib::ustring gToxFileSend::get_path() {
    return m_path;
}
