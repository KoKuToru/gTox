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

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

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

gToxFileSend::gToxFileSend(gToxObservable* observable,
                           Toxmm::FriendNr nr,
                           TOX_FILE_KIND kind,
                           Glib::ustring path,
                           Toxmm::FileId id,
                           uint64_t filesize)
    : gToxObserver(observable),
      m_friend_nr(nr),
      m_path(path),
      m_kind(kind) {

    throw std::runtime_error("gToxFileSend RESUME NOT IMPLEMENTED");
}

gToxFileSend::~gToxFileSend() {
    m_cancel->cancel();
    m_timeout.disconnect();
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

    if (ev.type() == typeid(Toxmm::EventFileSendChunkRequest)) {
        auto data = ev.get<Toxmm::EventFileSendChunkRequest>();
        if (data.nr != m_friend_nr || data.file_number != m_nr) {
            return;
        }

        //Handle upload
        m_queue.push(data);
        send_chunk();
    } else if (ev.type() == typeid(Toxmm::EventFileControl)) {
        auto data = ev.get<Toxmm::EventFileControl>();
        if (data.nr != m_friend_nr || data.file_number != m_nr) {
            return;
        }

        switch (data.control) {
            case TOX_FILE_CONTROL_RESUME:
                m_state = SENDING;
                break;
            case TOX_FILE_CONTROL_PAUSE:
                m_state = PAUSED;
                break;
            case TOX_FILE_CONTROL_CANCEL:
                m_state = STOPPED;
                break;
        }
    }
}

void gToxFileSend::send_chunk() {
    if (m_queue.empty() || m_cancel->is_cancelled() || m_is_sending_chunk) {
        return;
    }

    m_is_sending_chunk = true;

    auto data = m_queue.front();

    m_stream->seek(data.position, Glib::SeekType::SEEK_TYPE_SET);
    m_stream->read_bytes_async(data.size, [this, data](Glib::RefPtr<Gio::AsyncResult>& result) {
        auto bytes = m_stream->read_bytes_finish(result);
        gsize size;
        try {
        tox().file_send_chunk(m_friend_nr,
                              m_nr, data.position,
                              std::vector<uint8_t>( (uint8_t*)bytes->get_data(size),
                                                   ((uint8_t*)bytes->get_data(size)) + bytes->get_size()));
        } catch (Toxmm::Exception exp) {
            if (exp.type() == typeid(TOX_ERR_FILE_SEND_CHUNK) && exp.what_id() ==
                    TOX_ERR_FILE_SEND_CHUNK_SENDQ) {
                //thats stupid
                auto cancel = m_cancel;
                Glib::signal_timeout().connect_once([cancel, this]() {
                    if (!m_cancel->is_cancelled()) {
                        m_is_sending_chunk = false;
                        send_chunk();
                    }
                }, 100);
                return;
            }
            throw;
        }

        m_position = data.position + size;
        emit_progress();

        //take next
        m_queue.pop();
        m_is_sending_chunk = false;
        send_chunk();
    }, m_cancel);
}

Glib::ustring gToxFileSend::get_path() {
    return m_path;
}
