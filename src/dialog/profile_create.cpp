/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
    Copyright (C) 2014  Maurice Mohlek

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
#include "profile_create.h"
#include "tox/types.h"
#include "tox/core.h"
#include <glibmm/i18n.h>
using namespace dialog;

profile_create::profile_create(BaseObjectType* cobject,
                               utils::builder builder,
                               const Glib::ustring& path):
    Gtk::Assistant(cobject),
    m_aborted(true),
    m_path(path) {

    property_resizable() = false;
    set_size_request(800, 600);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS);

    builder.get_widget("assistant_username", m_username);
    builder.get_widget("assistant_statusmessage", m_status);
    builder.get_widget("assistant_logging", m_logging);
    builder.get_widget("assistant_file_tox", m_file_tox);
    builder.get_widget("assistant_file_gtox", m_file_gtox);


    auto w = builder.get_widget<Gtk::Widget>("assistant_first_page");
    m_username->signal_changed().connect([this, w]() {
        toxmm2::contactAddrPublic addr;
        m_last_toxcore = toxmm2::core::create_state(m_username->get_text(), m_status->get_text(), addr);
        m_file_gtox->set_text(Glib::build_filename(m_path, "gtox", std::string(addr) + ".sqlite"));

        if (!m_username->get_text().empty()) {
            int i = 0;
            do {
                auto name = m_username->get_text() + (i?("_"+std::to_string(i)):"");
                m_file_tox->set_text(Glib::build_filename(m_path, name + ".tox"));
                i += 1;
            } while (Glib::file_test(m_file_tox->get_text(), Glib::FILE_TEST_IS_REGULAR));

            if (w) {
                set_page_complete(*w, true);
            }
        } else {
            m_file_tox->set_text("");
            m_file_gtox->set_text("");
            if (w) {
                set_page_complete(*w, false);
            }
        }
    });

    m_status->signal_changed().connect([this, w]() {
        toxmm2::contactAddrPublic addr;
        m_last_toxcore = toxmm2::core::create_state(m_username->get_text(), m_status->get_text(), addr);
        m_file_gtox->set_text(Glib::build_filename(m_path, "gtox", std::string(addr) + ".sqlite"));
    });
}

utils::builder::ref<profile_create> profile_create::create(const Glib::ustring& path) {
    return utils::builder(Gtk::Builder::create_from_resource("/org/gtox/ui/dialog_assistant.ui"))
            .get_widget_derived<profile_create>("dialog_assistant", path);
}

profile_create::~profile_create() {
}

void profile_create::on_cancel() {
    m_path.clear();
    hide();
}

void profile_create::on_apply() {
    auto stream = Gio::File::create_for_path(m_file_tox->get_text())->create_file();
    auto bytes = Glib::Bytes::create((void*)m_last_toxcore.data(), m_last_toxcore.size());
    stream->write_bytes(bytes);
    stream->close();
    m_aborted = false;
    m_path = m_file_tox->get_text();
    m_path.clear();
    hide();
}

void profile_create::on_close() {

}

bool profile_create::is_aborted() {
    return m_aborted;
}

Glib::ustring profile_create::get_path() {
    return m_path;
}
