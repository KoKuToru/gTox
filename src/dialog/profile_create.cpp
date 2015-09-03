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
#include "utils/debug.h"

using namespace dialog;

profile_create::profile_create(BaseObjectType* cobject,
                               utils::builder builder,
                               const Glib::ustring& path):
    Gtk::Assistant(cobject),
    m_aborted(true),
    m_path(path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { path.raw() });

    property_resizable() = false;
    set_size_request(800, 600);
    set_position(Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS);

    builder.get_widget("assistant_username", m_username);
    builder.get_widget("assistant_statusmessage", m_status);
    builder.get_widget("assistant_file_tox", m_file_tox);

    auto w = builder.get_widget<Gtk::Widget>("assistant_first_page");
    m_username->signal_changed().connect([this, w]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        toxmm::contactAddrPublic addr;
        m_last_toxcore = toxmm::core::create_state(m_username->get_text(), m_status->get_text(), addr);

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
            if (w) {
                set_page_complete(*w, false);
            }
        }
    });

    m_status->signal_changed().connect([this, w]() {
        utils::debug::scope_log log(DBG_LVL_2("gtox"), {});
        toxmm::contactAddrPublic addr;
        m_last_toxcore = toxmm::core::create_state(m_username->get_text(), m_status->get_text(), addr);
    });
}

utils::builder::ref<profile_create> profile_create::create(const Glib::ustring& path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { path.raw() });
    return utils::builder::create_ref<profile_create>(
                "/org/gtox/ui/dialog_assistant.ui",
                "dialog_assistant",
                path);
}

profile_create::~profile_create() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

void profile_create::on_cancel() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    m_path.clear();
    hide();
}

void profile_create::on_apply() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
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
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

bool profile_create::is_aborted() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_aborted;
}

Glib::ustring profile_create::get_path() {
    utils::debug::scope_log log(DBG_LVL_5("gtox"), {});
    return m_path;
}
