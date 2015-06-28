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
#include "Widget/WidgetAvatar.h"

uint64_t operator "" _kib(unsigned long long input) {
    return input*1024;
}

gToxFileRecv::gToxFileRecv(gToxObservable* observable,
                           Toxmm::EventFileRecv file)
    : gToxObserver(observable), m_file(file) {
    try {
        if (m_file.kind == TOX_FILE_KIND_AVATAR) {
            if (m_file.file_size > 65_kib) {
                //ignore file !
                cancel();
                return;
            }

            m_path = WidgetAvatar::get_avatar_path(observable, m_file.nr);
            if (m_file.file_size == 0) {
                //Remove avatar
                WidgetAvatar::set_avatar(observable, m_path, Glib::RefPtr<Gdk::Pixbuf>());
                return;
            }

            //check if
            m_fd = ::open(m_path.c_str(), O_RDONLY);
            if (m_fd != -1) {
                std::vector<uint8_t> data;

                while(true) {
                    char buf[1024];
                    int r = ::read(m_fd, buf, 1024);
                    if (r <= 0) {
                        break;
                    }
                    std::copy(buf, buf + r, std::back_inserter(data));
                }
                close(m_fd);
                m_fd = -1;

                auto file_id_should = tox().hash(data);
                auto file_id = tox().file_get_field_id(m_file.nr, m_file.file_number);
                if (!std::equal(file_id.begin(), file_id.end(), file_id_should.begin())) {
                    //Remove avatar, because it's wrong
                    WidgetAvatar::set_avatar(observable, m_path, Glib::RefPtr<Gdk::Pixbuf>());
                }
            }
        } else {
            m_path = tox().database().config_get("SETTINGS_FILETRANSFER_SAVE_TO",
                                                 Glib::build_filename(Glib::get_user_special_dir(GUserDirectory::G_USER_DIRECTORY_DOWNLOAD), "gtox"));
            if (!Glib::file_test(m_path, Glib::FILE_TEST_IS_DIR)) {
                Gio::File::create_for_path(m_path)->make_directory_with_parents();
            }
            m_path = Glib::build_filename(m_path, m_file.filename);
        }
        m_fd = ::open(m_path.c_str(), O_RDWR|O_CREAT, 0600);
        if (m_fd == -1) {
            throw std::runtime_error("gToxFileRecv couldn't open file");
        }
        //read file size
        m_position = lseek(m_fd, 0, SEEK_END);

        tox().file_seek(m_file.nr, m_file.file_number, m_position);
    } catch(const Toxmm::Exception& exp) {
        //Ignore
        if (m_fd != -1) {
            ::close(m_fd);
            m_fd = -1;
        }
    }
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
    if (m_state != PAUSED) {
        return;
    }
    m_state = RECVING;
    if (m_file.file_size == m_position) {
        cancel();

        observer_notify(ToxEvent(EventFileProgress{
                                     this,
                                     m_file.nr,
                                     m_file.kind,
                                     m_file.file_number,
                                     m_position,
                                     m_file.file_size
                                 }));

        if (m_file.kind == TOX_FILE_KIND_AVATAR) {
            WidgetAvatar::set_avatar(observable(),
                                     m_path,
                                     WidgetAvatar::get_avatar(m_path, true));
        }
        return;
    }
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_RESUME);
}

void gToxFileRecv::cancel() {
    if (m_state != STOPPED) {
        return;
    }
    m_state = STOPPED;
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_CANCEL);
    //TODO: delete file ?
}

void gToxFileRecv::pause() {
    if (m_state != RECVING) {
        return;
    }
    m_state = PAUSED;
    tox().file_control(m_file.nr, m_file.file_number, TOX_FILE_CONTROL_PAUSE);
}

void gToxFileRecv::observer_handle(const ToxEvent& ev) {
    if (ev.type() != typeid(Toxmm::EventFileRecvChunk)) {
        return;
    }
    auto data = ev.get<Toxmm::EventFileRecvChunk>();
    if (data.nr != m_file.nr || data.file_number != m_file.file_number) {
        return;
    }

    if (m_fd == -1) {
        return;
    }

    //Handle download
    if (lseek(m_fd, data.file_position, SEEK_SET) != (__off_t)data.file_position) {
        throw std::runtime_error("gToxFileRecv couldn't seek file");
    }

    auto ret = write(m_fd, data.file_data.data(), data.file_data.size());
    if (ret != ssize_t(data.file_data.size())) {
        throw std::runtime_error("gToxFileRecv couldn't write file");
    }

    m_position = data.file_position + data.file_data.size();

    if (m_position == m_file.file_size) {
        close(m_fd);
        m_fd = -1;
    }

    observer_notify(ToxEvent(EventFileProgress{
                                 this,
                                 m_file.nr,
                                 m_file.kind,
                                 m_file.file_number,
                                 m_position,
                                 m_file.file_size
                             }));
}

Glib::ustring gToxFileRecv::get_path() {
    return m_path;
}

gToxFileRecv::gToxFileRecv(const gToxFileRecv& o)
    : gToxObserver(o) {
    if (o.m_fd != -1) {
        m_fd = dup(o.m_fd);
    } else {
        m_fd = -1;
    }
    m_file = o.m_file;
    m_path = o.m_path;
    m_position = o.m_position;
}

void gToxFileRecv::operator=(const gToxFileRecv& o) {
    gToxObserver::operator=(o);
    int old_fd = m_fd;
    if (o.m_fd != -1) {
        m_fd = dup(o.m_fd);
    } else {
        m_fd = -1;
    }
    close(old_fd);
    m_file = o.m_file;
    m_path = o.m_path;
    m_position = o.m_position;
}
