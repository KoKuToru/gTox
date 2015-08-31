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

std::set<Glib::ustring>& profile::create_used_files() {
    static std::set<Glib::ustring> used_files;
    return used_files;
}

profile::profile(): m_used_files(create_used_files()) {

}

void profile::open(const Glib::ustring& path) {
    if (m_file) {
        close();
    }

    m_file = Gio::File::create_for_path(path);

    m_can_write = m_used_files.find(path) == m_used_files.end();
    m_can_read  = true;

    if (m_can_write) {
        try {
            m_file->open_readwrite();
        } catch (...) {
            m_can_write = false;
        }
    }

    try {
        m_stream = m_file->read();
    } catch (...) {
        m_can_read = false;
    }

    m_used_files.insert(m_file->get_path());
}

profile::~profile() {
    close();
}

void profile::close() {
    m_can_write = false;
    m_can_read  = false;

    if (m_file) {
        m_used_files.erase(m_file->get_path());
    }

    m_stream.clear();
    m_file.clear();
}

bool profile::can_write() {
    return m_can_write;
}

bool profile::can_read() {
    return m_can_read;
}

void profile::write(const std::vector<unsigned char>& data) {
    if (!can_write()) {
        throw std::runtime_error("profile::can_write() == false");
    }

    m_stream.clear();

    Glib::RefPtr<Gio::FileOutputStream> stream;
    try {
        stream = m_file->replace(std::string(), true);
    } catch (Gio::Error exp) {
        if (exp.code() != Gio::Error::CANT_CREATE_BACKUP) {
            throw;
        }
        stream = m_file->replace();
    }

    stream->truncate(0);
    stream->write_bytes(Glib::Bytes::create((gconstpointer)data.data(),
                                              data.size()));
    stream->close();

    open(m_file->get_path());
}

std::vector<unsigned char> profile::read() {
    if (!can_read()) {
        throw std::runtime_error("profile::can_read() == false");
    }

    std::vector<unsigned char> data(m_stream->query_info()->get_size());

    gsize size;
    m_stream->seek(0, Glib::SeekType::SEEK_TYPE_SET);
    m_stream->read_all((void*)data.data(), data.size(), size);

    return data;
}

void profile::move(const Glib::ustring& new_path) {
    if (!can_read()) {
        throw std::runtime_error("profile::can_read() == false");
    }
    if (!can_write()) {
        throw std::runtime_error("profile::can_write() == false");
    }

    m_stream.clear();

    m_file->move(
                Gio::File::create_for_path(new_path));

    open(new_path);
}
