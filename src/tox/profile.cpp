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
#include "profile.h"
#include <sys/file.h>

using namespace toxmm;

#if defined _WIN32 || defined __CYGWIN__
    #include <iostream>
    int flock(int, int) {
        return 0;
    }
    #define LOCK_EX 0
    #define LOCK_NB 0
    #define LOCK_SH 0
    #define LOCK_UN 0
    void fsync(int) { }
#endif

void profile::open(const Glib::ustring& path) {
    if (can_read() || can_write()) {
        throw std::runtime_error("Profile already loaded");
    }
    m_path = path;
    m_fd = ::open(m_path.c_str(), O_RDWR|O_CREAT, 0600);
    if (m_fd == -1) {
        return;
    }

    if (flock(m_fd, LOCK_EX|LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            m_writeable = false;
            ::close(m_fd);
            m_fd = ::open(m_path.c_str(), O_RDONLY);
        } else {
            throw std::runtime_error("Profile flock error");
        }
    } else {
        m_writeable = true;
    }
    if (flock(m_fd, LOCK_SH) == -1) {
        throw std::runtime_error("Profile flock error");
    }
}

profile::~profile() {
    close();
}

void profile::close() {
    if (m_fd != -1) {
        flock(m_fd, LOCK_UN);
        ::close(m_fd);
        m_fd = -1;
        m_writeable = false;
    }
}

bool profile::can_write() {
    return m_writeable;
}

bool profile::can_read() {
    return m_fd != -1;
}

void profile::write(const std::vector<unsigned char>& data) {
#if defined _WIN32 || defined __CYGWIN__
    std::clog << "profile::write not support on windows yet !" << std::endl;
#else
    if (!can_write()) {
        throw std::runtime_error("profile::can_write() == false");
    }
    auto path_tmp = m_path + ".tmp";
    int tmp = ::open(path_tmp.c_str(), O_RDWR|O_CREAT, 0600);
    if (tmp == -1) {
        throw std::runtime_error("Couldn't create tmp file for profile");
    }

    ::write(tmp, data.data(), data.size());
    fsync(tmp);
    ::close(tmp);

    //SWAP !
    flock(m_fd, LOCK_UN);
    ::close(m_fd);
    m_fd = -1;
    if (rename(path_tmp.c_str(), m_path.c_str()) == -1) {
        throw std::runtime_error("Rename failed !");
    }

    //reopen
    m_fd = ::open(m_path.c_str(), O_RDWR|O_CREAT, 0600);
    if (m_fd == -1) {
        throw std::runtime_error("Couldn't reopen file !!");
    }
    if (flock(m_fd, LOCK_SH) == -1) {
        throw std::runtime_error("Profile flock error");
    }
#endif
}

std::vector<unsigned char> profile::read() {
    if (!can_read()) {
        throw std::runtime_error("profile::can_read() == false");
    }

    auto size = lseek(m_fd, 0, SEEK_END);
    lseek(m_fd, 0, SEEK_SET);

    std::vector<unsigned char> tmp(size);
    ::read(m_fd, tmp.data(), tmp.size());

    return tmp;
}

void profile::move(const Glib::ustring& new_path) {
    if (!can_read()) {
        throw std::runtime_error("profile::can_read() == false");
    }
    if (!can_write()) {
        throw std::runtime_error("profile::can_write() == false");
    }

    //move file
    close();
    if (rename(m_path.c_str(), new_path.c_str()) == -1) {
        throw std::runtime_error("Rename failed !");
    }

    //reopen
    open(new_path);
}
