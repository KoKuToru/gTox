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
        auto file = find(nr);
        if (file) {
            file->send_chunk_request(position, length);
            m_signal_send_chunk_rq(file, position, length);
        }
    }, *this));

    ct->signal_recv_file().connect(sigc::track_obj([this](fileNr nr, TOX_FILE_KIND kind, size_t size, Glib::ustring name) {
       auto c = core();
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
           throw toxmm2::exception(error);
       }

       auto iter = std::find_if(m_file.begin(), m_file.end(), [&](auto file) {
           return file->is_recv() &&
                  file->m_property_id.get_value() == id &&
                  file->m_property_size.get_value() == size &&
                  file->m_property_kind.get_value() == kind &&
                  file->m_property_name.get_value() == name;
       });
       if (iter != m_file.end()) {
           //resume file
           auto f = *iter;
           f->m_property_nr = nr;
           f->m_property_state = TOX_FILE_CONTROL_PAUSE;
           f->m_property_state_remote = TOX_FILE_CONTROL_RESUME;
           return;
       }

       //new file
       auto f = std::shared_ptr<toxmm2::file>(new toxmm2::file_recv(shared_from_this()));
       f->m_property_id = id;
       f->m_property_nr = nr;
       f->m_property_kind = kind,
       f->m_property_name = name;
       f->m_property_state = TOX_FILE_CONTROL_PAUSE;
       f->m_property_state_remote = TOX_FILE_CONTROL_RESUME;

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
               auto last_point = fname.find_last_of('.');
               ffname = (last_point == std::string::npos)?
                   Glib::ustring::compose("%1 (%2)",
                                               ffname,
                                               index):
                   Glib::ustring::compose("%1 (%2)%3",
                                               ffname.substr(0, last_point),
                                               index,
                                               ffname.substr(last_point));
           }
           fpath = Glib::build_filename(
               core()->property_download_path().get_value(),
               fname);
           ++index;
       } while (Glib::file_test(fpath, Glib::FILE_TEST_EXISTS));
       f->m_property_path = fpath;

       f->init();

       m_file.push_back(f);

       m_signal_recv_file(f);
    }, *this));

    ct->signal_recv_file_chunk().connect(sigc::track_obj([this](fileNr nr, uint64_t position, const std::vector<uint8_t>& content) {
       auto file = find(nr);
       if (file) {
           file->recv_chunk(position, content);
           m_signal_recv_chunk(file, content);
       }
    }, *this));
}

std::shared_ptr<file> file_manager::find(fileNr nr) {
    auto iter = std::find_if(m_file.begin(), m_file.end(), [&](auto file) {
        return file->m_property_nr.get_value() == nr;
    });
    if (iter == m_file.end()) {
        return nullptr;
    }
    return *iter;
}

std::shared_ptr<toxmm2::core> file_manager::core() {
    auto ct = contact();
    return ct ? ct->core() : nullptr;
}

std::shared_ptr<toxmm2::contact_manager> file_manager::contact_manager() {
    auto ct = contact();
    return ct ? ct->contact_manager() : nullptr;
}

std::shared_ptr<toxmm2::contact> file_manager::contact() {
    return m_contact.lock();
}
