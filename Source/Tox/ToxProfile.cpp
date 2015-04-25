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
#include "ToxProfile.h"

ToxProfile::ToxProfile(const Glib::ustring& path) {
    m_path = path;
    m_fd = open(m_path.c_str(), O_RDWR|O_CREAT|O_EXCL, 0444);
    if (m_fd != -1) {
        m_writeable = true;
        return;
    }
    //try read only
    m_fd = open(m_path.c_str(), O_RDONLY);
}

ToxProfile::~ToxProfile() {
    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }
}

bool ToxProfile::can_write() {
    return m_writeable;
}

bool ToxProfile::can_read() {
    return m_fd != -1;
}

void ToxProfile::write(const std::vector<unsigned char>& data) {
    if (!can_write()) {
        throw std::runtime_error("ToxProfile::can_write() == false");
    }
    auto path_tmp = m_path + ".tmp";
    int tmp = open(path_tmp.c_str(), O_RDWR|O_CREAT|O_EXCL, 0444);
    if (tmp == -1) {
        throw std::runtime_error("Couldn't create tmp file for profile");
    }

    ::write(tmp, data.data(), data.size());
    fsync(tmp);
    close(tmp);

    //SWAP !
    close(m_fd);
    m_fd = -1;
    if (rename(path_tmp.c_str(), m_path.c_str()) == -1) {
        throw std::runtime_error("Rename failed !");
    }

    //reopen
    m_fd = open(m_path.c_str(), O_RDWR|O_CREAT|O_EXCL, 0444);
    if (m_fd != -1) {
        throw std::runtime_error("Couldn't reopen file !!");
    }
}

void ToxProfile::move(const Glib::ustring& new_path) {
    //SWAP !
    close(m_fd);
    m_fd = -1;
    if (rename(m_path.c_str(), new_path.c_str()) == -1) {
        throw std::runtime_error("Rename failed !");
    }

    m_path = new_path;

    //reopen
    m_fd = open(m_path.c_str(), O_RDWR|O_CREAT|O_EXCL, 0444);
    if (m_fd != -1) {
        throw std::runtime_error("Couldn't reopen file !!");
    }
}
