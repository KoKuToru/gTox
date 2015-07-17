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
#include "file.h"
#include "manager.h"
#include "contact/contact.h"
#include "contact/manager.h"
#include "core.h"
#include "exception.h"

using namespace toxmm2;

Glib::PropertyProxy_ReadOnly<fileId>           file::property_id()
{ return Glib::PropertyProxy_ReadOnly<fileId>(this, "file-id"); }
Glib::PropertyProxy_ReadOnly<fileNr>           file::property_nr()
{ return Glib::PropertyProxy_ReadOnly<fileNr>(this, "file-nr"); }
Glib::PropertyProxy_ReadOnly<TOX_FILE_KIND>    file::property_kind()
{ return Glib::PropertyProxy_ReadOnly<TOX_FILE_KIND>(this, "file-kind"); }
Glib::PropertyProxy_ReadOnly<uint64_t>         file::property_position()
{ return Glib::PropertyProxy_ReadOnly<uint64_t>(this, "file-position"); }
Glib::PropertyProxy_ReadOnly<size_t>           file::property_size()
{ return Glib::PropertyProxy_ReadOnly<size_t>(this, "file-size"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>    file::property_name()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "file-name"); }
Glib::PropertyProxy_ReadOnly<Glib::ustring>    file::property_path()
{ return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "file-path"); }
Glib::PropertyProxy<TOX_FILE_CONTROL>          file::property_state()
{ return m_property_state.get_proxy(); }
Glib::PropertyProxy_ReadOnly<TOX_FILE_CONTROL> file::property_state_remote()
{ return Glib::PropertyProxy_ReadOnly<TOX_FILE_CONTROL>(this, "file-state-remote"); }

file::file(std::shared_ptr<toxmm2::file_manager> manager):
    Glib::ObjectBase(typeid(file)),
    m_file_manager(manager),
    m_property_id  (*this, "file-id"),
    m_property_nr  (*this, "file-nr"),
    m_property_kind(*this, "file-kind"),
    m_property_position(*this, "file-position"),
    m_property_size (*this, "file-position"),
    m_property_name (*this, "file-name"),
    m_property_path (*this, "file-path"),
    m_property_state(*this, "file-state"),
    m_property_state_remote(*this, "file-state-remote") {
}

void file::init() {
    property_state().signal_changed().connect([this]() {
        //send changes
        auto c  = core();
        auto ct = contact();
        if (!c || !ct) {
            return;
        }
        TOX_ERR_FILE_CONTROL error;
        tox_file_control(c->toxcore(),
                         ct->property_nr().get_value(),
                         property_nr().get_value(),
                         property_state(),
                         &error);
        if (error != TOX_ERR_FILE_CONTROL_OK) {
            throw toxmm2::exception(error);
        }
    });
}

std::shared_ptr<toxmm2::core> file::core() {
    auto m = file_manager();
    return m ? m->core() : nullptr;
}

std::shared_ptr<toxmm2::file_manager> file::file_manager() {
    return m_file_manager.lock();
}

std::shared_ptr<toxmm2::contact_manager> file::contact_manager() {
    auto m = file_manager();
    return m ? m->contact_manager() : nullptr;
}

std::shared_ptr<toxmm2::contact> file::contact() {
    auto m = file_manager();
    return m ? m->contact() : nullptr;
}
