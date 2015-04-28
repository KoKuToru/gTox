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
#include "gToxFileRecv.h"
#include <iostream>

gToxFileRecv::gToxFileRecv(gToxObservable* observable,
                           Toxmm::EventFileRecv file)
    : gToxObserver(observable), m_file(file) {
    if (m_file.kind == TOX_FILE_KIND_AVATAR) {
        auto addr = tox().get_address(file.nr);
        m_path = Glib::build_filename(Glib::get_user_config_dir(), "tox", Toxmm::to_hex(addr.data(), TOX_PUBLIC_KEY_SIZE) + ".png");
    } else {
        m_path = Glib::build_filename(Glib::get_user_special_dir(GUserDirectory::G_USER_DIRECTORY_DOWNLOAD), m_file.filename);
    }
    std::clog << "Download " << m_path << std::endl;
    m_fd = ::open(m_path.c_str(), O_RDWR|O_CREAT, 0600);
    if (m_fd == -1) {
        throw std::runtime_error("gToxFileRecv couldn't open file");
    }
    //read file size
    m_position = lseek(m_fd, 0, SEEK_END);
}

gToxFileRecv::~gToxFileRecv() {
    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }
}

void gToxFileRecv::get_progress(uint64_t& position, uint64_t& size) {
    position = m_position;
    size = m_file.file_size;
}

void gToxFileRecv::resume() {
    //tox().file_seek()
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_RESUME);
}

void gToxFileRecv::cancel() {
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_CANCEL);
}

void gToxFileRecv::pause() {
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_PAUSE);
}

void gToxFileRecv::observer_handle(const ToxEvent& ev) {
    if (ev.type() != typeid(Toxmm::EventFileRecvChunk)) {
        return;
    }
    auto data = ev.get<Toxmm::EventFileRecvChunk>();
    if (data.nr != m_file.nr && data.file_number != m_file.file_number) {
        return;
    }

    //Handle download
    auto ret = write(m_fd, data.file_data.data(), data.file_data.size());
    if (ret != ssize_t(data.file_data.size())) {
        std::clog << Glib::strerror(errno) << std::endl;
        throw std::runtime_error("gToxFileRecv couldn't write file");
    }

    m_position += data.file_data.size();

    observer_notify(ToxEvent(EventFileProgress{
                                 m_file.nr,
                                 m_file.file_number,
                                 m_position,
                                 m_file.file_size
                             }));

    //finish event ?
}

gToxFileRecv::gToxFileRecv(const gToxFileRecv& o)
    : gToxObserver(o) {
    m_fd = dup(o.m_fd);
    m_file = o.m_file;
    m_path = o.m_path;
    m_position = o.m_position;
}

void gToxFileRecv::operator=(const gToxFileRecv& o) {
    gToxObserver::operator=(o);
    m_fd = dup(o.m_fd);
    m_file = o.m_file;
    m_path = o.m_path;
    m_position = o.m_position;
}
