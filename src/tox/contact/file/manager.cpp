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
#include "exception.h"
#include "file.h"
#include "file_recv.h"
#include "file_send.h"
#include "flatbuffers/generated/File_generated.h"

using namespace toxmm;

file_manager::type_signal_recv          file_manager::signal_recv_file() { return m_signal_recv_file; }
file_manager::type_signal_send          file_manager::signal_send_file() { return m_signal_send_file; }
file_manager::type_signal_recv_chunk    file_manager::signal_recv_chunk() { return m_signal_recv_chunk; }
file_manager::type_signal_send_chunk    file_manager::signal_send_chunk() { return m_signal_send_chunk; }
file_manager::type_signal_send_chunk_rq file_manager::signal_send_chunk_request() { return m_signal_send_chunk_rq; }
file_manager::type_signal_recv_control  file_manager::signal_recv_control() { return m_signal_recv_control; }
file_manager::type_signal_send_control  file_manager::signal_send_control() { return m_signal_send_control; }

file_manager::file_manager(std::shared_ptr<toxmm::contact> contact)
    : m_contact(contact) {

}

void file_manager::init() {
    auto ct = contact();
    if (!ct) {
        return;
    }

    ct->signal_send_file_chunk_request().connect(sigc::track_obj([this](fileNr nr, uint64_t position, size_t length) {
        auto file = find(nr);
        if (file) {
            file->pre_send_chunk_request(position, length);
            m_signal_send_chunk_rq(file, position, length);
        }
    }, *this));

    ct->signal_recv_file().connect(sigc::track_obj([this](fileNr nr, TOX_FILE_KIND kind, size_t size, Glib::ustring name) {
       auto c  = core();
       auto ct = contact();
       if (!c || !ct) {
           return;
       }

       fileId id;
       TOX_ERR_FILE_GET error;
       tox_file_get_file_id(c->toxcore(),
                            ct->property_nr().get_value(),
                            nr,
                            id,
                            &error);
       if (error != TOX_ERR_FILE_GET_OK) {
           throw toxmm::exception(error);
       }

       auto iter = std::find_if(m_file.begin(), m_file.end(), [&](auto file) {
           return file->is_recv() &&
                  file->m_property_active.get_value() == false &&
                  file->m_property_complete.get_value() == false &&
                  file->m_property_id.get_value() == id &&
                  file->m_property_size.get_value() == size &&
                  file->m_property_kind.get_value() == kind &&
                  file->m_property_name.get_value() == name;
       });
       if (iter != m_file.end()) {
           //resume file
           auto f = *iter;
           f->m_property_nr = nr;
           f->m_property_active = true;
           return;
       }

       //new file
       auto f = std::shared_ptr<toxmm::file>(new toxmm::file_recv(shared_from_this()));
       f->m_property_id = id;
       f->m_property_nr = nr;
       f->m_property_kind = kind,
       f->m_property_name = name;
       f->m_property_size = size;
       f->m_property_active = true;
       f->init();

       if (kind == TOX_FILE_KIND_DATA) {
           //setup path
           auto fname = name;
           size_t index = 0;
           while ((index = fname.find_first_of(Glib::ustring("/"), index))
           != Glib::ustring::npos) {
               fname.replace(index, 1, Glib::ustring(" "));
           }
           while(!fname.empty() && fname[0] == '.') {
               fname.erase(0);
           }
           if (fname.empty()) {
               fname = "download";
           }
           //make sure path doesn't exists
           index = 0;
           std::string fpath;
           do {
               Glib::ustring ffname;
               if (index == 0) {
                   ffname = fname;
               } else {
                   //find the last point before file extension
                   // demo.img.gz
                   //     ^ find this point
                   auto last_point = fname.size();
                   while(true) {
                       auto lp = fname.find_last_of('.', last_point);
                       if (lp == std::string::npos || last_point - lp > 3) {
                           break;
                       }
                       last_point = lp;
                   }

                   ffname = Glib::ustring::compose("%1(%2)%3",
                                              fname.substr(0, last_point),
                                              index,
                                              fname.substr(last_point));
               }
               fpath = Glib::build_filename(
                                 c->property_download_path().get_value(),
                                 ffname);
               ++index;
           } while (Glib::file_test(fpath, Glib::FILE_TEST_EXISTS));
           f->m_property_path = fpath;
       } else if (kind == TOX_FILE_KIND_AVATAR && size < 65*1024) {
           //check if file exists
           f->m_property_path = Glib::build_filename(
                  c->property_avatar_path().get_value(),
                  std::string(ct->property_addr_public().get_value()) + ".png");
           if (Glib::file_test(f->property_path().get_value(),
                               Glib::FILE_TEST_IS_REGULAR)) {
               //check if hash is different
               auto file = Gio::File::create_for_path(f->property_path()
                                               .get_value());
               std::vector<uint8_t> content(file->query_info()->get_size());
               gsize dummy;
               if (!content.empty()) {
                   file->read()->read_all((void*)content.data(),
                   content.size(),
                   dummy);
               }

               if (c->hash(content).get() == f->property_id().get_value().get()) {
                   //it is the same avatar, don't download it again
                   f->property_state() = TOX_FILE_CONTROL_CANCEL;
                   return;
               }

               //hash is different, remove this file
               Gio::File::create_for_path(f->property_path().get_value())
                                               ->remove();
           }
           if (f->property_size() == 0) {
               //nothing to download
               f->property_state() = TOX_FILE_CONTROL_CANCEL;
               return;
           }
           //when user goes offline remote this file
           std::weak_ptr<toxmm::file> fw = f;
           ct->property_connection().signal_changed().connect(
                                           sigc::track_obj([this, fw]() {
               auto ct = contact();
               if (ct && ct->property_connection() == TOX_CONNECTION_NONE) {
                   auto f = fw.lock();
                   if (f) {
                       f->property_state() = TOX_FILE_CONTROL_CANCEL;
                   }
               }
           }, *this));
           //start download !
           f->property_state() = TOX_FILE_CONTROL_RESUME;
       } else {
           f->property_state() = TOX_FILE_CONTROL_CANCEL;
           return;
       }

       //remove file from list when complete
       std::weak_ptr<toxmm::file> fw = f;
       f->property_complete().signal_changed().connect(sigc::track_obj([this, fw]() {
           auto f = fw.lock();
           if (f && f->property_complete()) {
               auto iter = std::find(m_file.begin(), m_file.end(), f);
               if (iter != m_file.end()) {
                   m_file.erase(iter);
                   save();
               }
           }
       }, *this));

       f->property_state().signal_changed().connect(sigc::track_obj([this]() {
           save();
       }, *this));

       //add file to list
       m_file.push_back(f);
       save();

       //emit signal
       m_signal_recv_file(f);
    }, *this));

    ct->signal_recv_file_chunk().connect(sigc::track_obj([this](fileNr nr, uint64_t position, const std::vector<uint8_t>& content) {
       auto file = find(nr);
       if (file) {
           file->pre_recv_chunk(position, content);
           m_signal_recv_chunk(file, content);
       }
    }, *this));

    ct->signal_recv_file_control().connect(sigc::track_obj([this](fileNr nr, TOX_FILE_CONTROL state) {
        auto file = find(nr);
        if (file) {
            file->m_property_state_remote = state;
        }
    }, *this));

    load();
}

std::shared_ptr<file> file_manager::find(fileNr nr) {
    auto iter = std::find_if(m_file.begin(), m_file.end(), [&](auto file) {
        return file->m_property_active.get_value() &&
               file->m_property_nr.get_value() == nr;
    });
    if (iter == m_file.end()) {
        return nullptr;
    }
    return *iter;
}

std::shared_ptr<file> file_manager::find(uniqueId id) {
    auto iter = std::find_if(m_file.begin(), m_file.end(), [&](auto file) {
        return file->m_property_uuid.get_value() == id;
    });
    if (iter == m_file.end()) {
        return nullptr;
    }
    return *iter;
}

std::shared_ptr<file> file_manager::send_file(const Glib::ustring& path, bool avatar) {
    //new file
    auto c  = core();
    auto ct = contact();
    if (!c || !ct) {
        return nullptr;
    }

    auto file  = Gio::File::create_for_path(path);
    auto f = std::shared_ptr<toxmm::file>(new toxmm::file_send(shared_from_this()));

    f->m_property_kind = avatar?TOX_FILE_KIND_AVATAR:TOX_FILE_KIND_DATA,
    f->m_property_name = file->get_basename();
    if (!path.empty() && file->query_exists()) {
        f->m_property_size = file->query_info()->get_size();
    } else {
        f->m_property_size = 0;
    }

    f->m_property_path = path;

    if (avatar && !path.empty()) {
        //get hash
        auto file = Gio::File::create_for_path(f->property_path()
                                        .get_value());
        std::vector<uint8_t> content;
        if (file->query_exists()) {
            content.resize(file->query_info()->get_size());
            if (!content.empty()) {
                gsize dummy;
                file->read()->read_all((void*)content.data(),
                                       content.size(),
                                       dummy);
            }
        }
        f->m_property_id.set_value(c->hash(content).get());
    }

    TOX_ERR_FILE_SEND error;

    f->m_property_nr = tox_file_send(
                           c->toxcore(),
                           ct->property_nr().get_value(),
                           f->property_kind(),
                           f->property_size(),
                           avatar?f->property_id().get_value():nullptr,
                           (const uint8_t*)f->property_name().get_value().c_str(),
                           f->property_name().get_value().bytes(),
                           &error);
    if (error != TOX_ERR_FILE_SEND_OK) {
        throw toxmm::exception(error);
    }

    fileId id;
    TOX_ERR_FILE_GET ferror;
    tox_file_get_file_id(c->toxcore(),
                         ct->property_nr().get_value(),
                         f->property_nr().get_value(),
                         id,
                         &ferror);
    if (ferror != TOX_ERR_FILE_GET_OK) {
        throw toxmm::exception(error);
    }

    f->m_property_id = id;
    f->m_property_active = true;
    f->m_property_state = TOX_FILE_CONTROL_PAUSE;
    f->m_property_state_remote = TOX_FILE_CONTROL_PAUSE;
    f->init();

    //remove file from list when complete
    std::weak_ptr<toxmm::file> fw = f;
    f->property_complete().signal_changed().connect(sigc::track_obj([this, fw]() {
        auto f = fw.lock();
        if (f && f->property_complete()) {
            auto iter = std::find(m_file.begin(), m_file.end(), f);
            if (iter != m_file.end()) {
                m_file.erase(iter);
                save();
            }
        }
    }, *this));

    f->property_state().signal_changed().connect(sigc::track_obj([this]() {
        save();
    }, *this));

    //add file to list
    m_file.push_back(f);
    save();

    if (!path.empty()) {
        f->property_state() = TOX_FILE_CONTROL_RESUME;
    } else {
        f->property_state() = TOX_FILE_CONTROL_CANCEL;
    }

    //emit signal
    m_signal_send_file(f);

    return f;
}

std::shared_ptr<toxmm::core> file_manager::core() {
    auto ct = contact();
    return ct ? ct->core() : nullptr;
}

std::shared_ptr<toxmm::contact_manager> file_manager::contact_manager() {
    auto ct = contact();
    return ct ? ct->contact_manager() : nullptr;
}

std::shared_ptr<toxmm::contact> file_manager::contact() {
    return m_contact.lock();
}

void file_manager::load() {
    auto c  = core();
    auto ct = contact();
    if (!c || !ct) {
        return;
    }

    //load old data if possible
    std::vector<uint8_t> content;
    c->storage()->load({ct->property_addr_public().get_value(),
                        "file-manager"}, content);
    if (content.empty()) {
        return;
    }

    auto verify = flatbuffers::Verifier(content.data(), content.size());
    if (!flatbuffers::File::VerifyManagerBuffer(verify)) {
        throw std::runtime_error("flatbuffers::Config::VerifyManagerBuffer failed");
    }

    auto data = flatbuffers::File::GetManager(content.data());

    for(auto file: *data->files()) {
        std::shared_ptr<toxmm::file> f;
        if (file->is_recv()) {
            f = decltype(f)(new toxmm::file_recv(shared_from_this()));
            //todo check if file size is still the same
        } else {
            f = decltype(f)(new toxmm::file_send(shared_from_this()));
            //todo check if file size is still the same
        }
        f->m_property_uuid = std::string(file->uuid()->begin(), file->uuid()->end());
        f->m_property_id = std::string(file->id()->begin(), file->id()->end());
        f->m_property_kind = TOX_FILE_KIND(file->kind()),
        f->m_property_name = std::string(file->name()->begin(), file->name()->end());
        f->m_property_path = std::string(file->path()->begin(), file->path()->end());
        f->m_property_size = file->size();
        f->m_property_active = false;
        f->m_property_state = TOX_FILE_CONTROL(file->state());
        f->m_property_state_remote = TOX_FILE_CONTROL_PAUSE;
        f->m_property_position = file->position();
        f->m_property_progress = file->progress();
        f->init();

        //remove file from list when complete
        std::weak_ptr<toxmm::file> fw = f;
        f->property_complete().signal_changed().connect(sigc::track_obj([this, fw]() {
            auto f = fw.lock();
            if (f && f->property_complete()) {
                auto iter = std::find(m_file.begin(), m_file.end(), f);
                if (iter != m_file.end()) {
                    m_file.erase(iter);
                    save();
                }
            }
        }, *this));

        f->property_state().signal_changed().connect(sigc::track_obj([this]() {
            save();
        }, *this));

        //add file to list
        m_file.push_back(f);
    }
}

void file_manager::save() {
    auto c  = core();
    auto ct = contact();
    if (!c || !ct) {
        return;
    }

    using namespace flatbuffers;
    using File = flatbuffers::File::File;
    using FileBuilder = flatbuffers::File::FileBuilder;
    using FileManagerBuilder = flatbuffers::File::ManagerBuilder;

    std::vector<Offset<File>> vec(m_file.size());
    vec.clear();
    FlatBufferBuilder fbb;
    for(auto file: m_file) {
        auto uuid = fbb.CreateString(file->property_uuid().get_value());
        auto id   = fbb.CreateString(file->property_id().get_value());
        auto name = fbb.CreateString(file->property_name().get_value());
        auto path = fbb.CreateString(file->property_path().get_value());
        FileBuilder fb(fbb);
        fb.add_uuid(uuid);
        fb.add_id(id);
        fb.add_kind(int(file->property_kind()));
        fb.add_position(file->property_position());
        fb.add_size(file->property_size());
        fb.add_name(name);
        fb.add_path(path);
        fb.add_progress(file->property_progress());
        fb.add_is_recv(file->is_recv());
        fb.add_state(int(file->property_state()));
        vec.push_back(fb.Finish());
    }

    auto f_vec = fbb.CreateVector(vec);
    FileManagerBuilder fmb(fbb);
    fmb.add_files(f_vec);

    flatbuffers::File::FinishManagerBuffer(fbb, fmb.Finish());

    std::vector<uint8_t> content(fbb.GetBufferPointer(),
                                 fbb.GetBufferPointer() + fbb.GetSize());
    c->storage()->save({ct->property_addr_public().get_value(),
                        "file-manager"}, content);
}

file_manager::~file_manager() {
    save();
}
