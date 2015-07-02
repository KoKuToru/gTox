#include "gToxFileManager.h"
#include "gToxFileRecv2.h"
#include <iostream>

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

gToxFileManager::gToxFileManager(Toxmm* tox):
    m_tox(tox) {

    //shared_from_this() can not be used in constructor !!!
}

void gToxFileManager::init() {
    for (auto item : m_tox->database().gtoxfiletransf_get()) {
        Toxmm::FriendAddr addr;
        auto addr_vec = Toxmm::from_hex(item.friend_addr);
        std::copy(addr_vec.begin(), addr_vec.end(), addr.begin());
        auto friend_nr = m_tox->get_friend_number(addr);

        if (item.is_recv) {
            auto file = std::make_shared<gToxFileRecv2>(
                            shared_from_this(),
                            item.id,
                            friend_nr,
                            item.file_nr,
                            item.file_id,
                            item.file_kind,
                            item.file_name,
                            item.file_path,
                            0,
                            item.file_size,
                            item.status);
            m_file.insert({item.id, file});
        } else {
            auto file = std::make_shared<gToxFileSend2>(
                            shared_from_this(),
                            item.id,
                            friend_nr,
                            item.file_nr,
                            item.file_id,
                            item.file_kind,
                            item.file_name,
                            item.file_path,
                            0,
                            item.file_size,
                            item.status);
            m_file.insert({item.id, file});
        }
    }
}

gToxFileManager::~gToxFileManager() {
}

std::shared_ptr<gToxFileTransf> gToxFileManager::find(size_t unique_file_id) {
    std::shared_ptr<gToxFileTransf> res;
    auto iter = m_file.find(unique_file_id);
    if (iter != m_file.end()) {
        res = iter->second;
    }
    return res;
}

std::vector<std::shared_ptr<gToxFileTransf>> gToxFileManager::find_by_friend_nr(Toxmm::FriendNr nr) {
    std::vector<std::shared_ptr<gToxFileTransf>> result;
    for (auto item : m_file) {
        auto& file = item.second;
        if (file->m_friend_nr == nr) {
            result.push_back(file);
        }
    }
    return result;
}

void gToxFileManager::observer_handle(const ToxEvent& ev) {

    if (ev.type() == typeid(Toxmm::EventUserStatus)) {
        auto data = ev.get<Toxmm::EventUserStatus>();

        auto files = find_by_friend_nr(data.nr);
        for (auto file : files) {
            switch (data.status) {
                case Toxmm::OFFLINE:
                    file->deactivate();
                    file->m_active = false;
                    break;
                default:
                    //TODO FOR RESUME
                    break;
            }
            m_tox->inject_event(ToxEvent(EventFileUpdate{
                                         file
                                     }));
        }
    } else if (ev.type() == typeid(Toxmm::EventFileControl)) {
        auto data = ev.get<Toxmm::EventFileControl>();

        auto files = find_by_friend_nr(data.nr);
        for (auto file : files) {
            if (file->m_active && file->m_file_nr == data.file_number) {
                switch (data.control) {
                    case TOX_FILE_CONTROL_RESUME:
                        file->m_state = gToxFileTransf::RESUME;
                        break;
                    case TOX_FILE_CONTROL_PAUSE:
                        file->m_state = gToxFileTransf::PAUSE;
                        break;
                    case TOX_FILE_CONTROL_CANCEL:
                        file->m_state = gToxFileTransf::CANCEL;
                        break;
                }
                m_tox->inject_event(ToxEvent(EventFileUpdate{
                                             file
                                         }));
                return;
            }
        }
    } else if (ev.type() == typeid(Toxmm::EventFileRecv)) {
        auto data = ev.get<Toxmm::EventFileRecv>();
        auto file_id = m_tox->file_get_file_id(data.nr, data.file_number);
        //check if file exists
        auto files = find_by_friend_nr(data.nr);
        for (auto file : files) {
            if (data.filename == file->m_file_name &&
                data.file_size == file->m_file_size &&
                file_id == file->m_file_id) {
                file->m_file_nr = data.file_number;
                file->activate();
                file->m_active = true;
                return;
            }
        }
        //doesnt
        auto unique_id = std::hash<std::string>()(data.filename +
                                                  std::string((const char*)file_id.data(),
                                                              file_id.size()));
        //make 100% sure it's unique
        while (m_file.find(unique_id) != m_file.end()) {
            unique_id += 1;
        }
        std::shared_ptr<gToxFileTransf> file = std::make_shared<gToxFileRecv2>(
                        shared_from_this(),
                        unique_id,
                        data.nr,
                        data.file_number,
                        file_id,
                        data.kind,
                        data.filename,
                        data.filepath,
                        0,
                        data.file_size,
                        int(gToxFileTransf::PAUSE));
        m_file.insert({unique_id, file});
        auto addr = m_tox->get_address(data.nr);
        //store in db
        m_tox->database().gtoxfiletransf_insert({
                                                   unique_id,
                                                   true,
                                                   Toxmm::to_hex(addr.begin(), addr.size()),
                                                   data.file_number,
                                                   file_id,
                                                   data.kind,
                                                   data.filename,
                                                   data.filepath,
                                                   data.file_size,
                                                   int(file->state())
                                               });
        file->activate();
        file->m_active = true;
        m_tox->inject_event(ToxEvent(EventNewFile{
                                     file
                                 }));
    } else if (ev.type() == typeid(Toxmm::EventFileRecvChunk)) {
        auto data = ev.get<Toxmm::EventFileRecvChunk>();

        auto files = find_by_friend_nr(data.nr);
        for (auto file : files) {
            if (file->m_active && file->m_file_nr == data.file_number) {
                file->recv_chunk(data.file_position, data.file_data, [this, file, data]() {
                    file->m_file_position = data.file_position + data.file_data.size();
                    std::clog << "in recv chunk"; file->file_position();
                    if (file->m_file_position >= file->m_file_size) {
                        file->m_state = gToxFileTransf::FINISH;
                        file->deactivate();
                        file->m_active = false;
                        m_tox->database().gtoxfiletransf_update(file->id(), int(file->m_state));
                    }
                    m_tox->inject_event(ToxEvent(EventFileUpdate{
                                                 file
                                             }));
                });
                return;
            }
        }
    } else if (ev.type() == typeid(Toxmm::EventFileSendChunkRequest)) {
        auto data = ev.get<Toxmm::EventFileSendChunkRequest>();

        auto files = find_by_friend_nr(data.nr);
        for (auto file : files) {
            if (file->m_active && file->m_file_nr == data.file_number) {
                file->send_chunk(data.position, data.size, [this, file, data](const std::vector<uint8_t>& chunk) {
                    m_tox->file_send_chunk(file->m_friend_nr, file->m_file_nr, data.position, chunk);
                    file->m_file_position = data.position + chunk.size();
                    m_tox->inject_event(ToxEvent(EventFileUpdate{
                                                 file
                                             }));
                });
                return;
            }
        }
    }
}

void gToxFileManager::resume(gToxFileTransf* file) {
    if (file->m_state == gToxFileTransf::FINISH ||
            file->m_state == gToxFileTransf::CANCEL ||
            file->m_state == gToxFileTransf::RESUME) {
        return;
    }

    file->m_state = gToxFileTransf::RESUME;
    m_tox->database().gtoxfiletransf_update(file->id(), int(file->m_state));

    if (file->m_active) {
        m_tox->file_control(file->m_friend_nr, file->m_file_nr, TOX_FILE_CONTROL_RESUME);
    }
}

void gToxFileManager::pause(gToxFileTransf* file) {
    if (file->m_state == gToxFileTransf::FINISH ||
            file->m_state == gToxFileTransf::CANCEL ||
            file->m_state == gToxFileTransf::PAUSE) {
        return;
    }

    file->m_state = gToxFileTransf::PAUSE;
    m_tox->database().gtoxfiletransf_update(file->id(), int(file->m_state));

    if (file->m_active) {
        m_tox->file_control(file->m_friend_nr, file->m_file_nr, TOX_FILE_CONTROL_PAUSE);
    }
}

void gToxFileManager::cancel(gToxFileTransf* file) {
    if (file->m_state == gToxFileTransf::FINISH ||
            file->m_state == gToxFileTransf::CANCEL) {
        return;
    }

    file->m_state = gToxFileTransf::CANCEL;
    m_tox->database().gtoxfiletransf_update(file->id(), int(file->m_state));

    file->deactivate();

    if (file->m_active) {
        file->m_active = false;
        m_tox->file_control(file->m_friend_nr, file->m_file_nr, TOX_FILE_CONTROL_CANCEL);
    }
}
