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
#ifndef TOXMM_FILE_H
#define TOXMM_FILE_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"

template<typename T, bool writeable = true> using PropProxy = typename std::conditional<writeable, Glib::PropertyProxy<T>, Glib::PropertyProxy_ReadOnly<T>>::type;
template<typename T> using Prop = typename Glib::Property<T>;
template<bool writeable = true, typename T> constexpr PropProxy<T, writeable> proxy(Glib::ObjectBase* self, Glib::Property<T>& prop) {
    class Hack: public Glib::PropertyBase {
        public:
            const char* get_name_internal() const {
                return PropertyBase::get_name_internal();
            }
    };
    const Glib::PropertyBase* base = &prop;
    return {self, ((const Hack*)base)->get_name_internal()};
}

namespace toxmm2 {
    class file_manager;
    class contact_manager;
    class contact;
    class core;

    class file:
            virtual public Glib::Object,
            public std::enable_shared_from_this<file> {

            friend class file_manager;
            friend class file_recv;

        public:
            //props
            auto property_id()           -> PropProxy<fileId, false>;
            auto property_nr()           -> PropProxy<fileNr, false>;
            auto property_kind()         -> PropProxy<TOX_FILE_KIND, false>;
            auto property_position()     -> PropProxy<uint64_t, false>;
            auto property_size()         -> PropProxy<uint64_t, false>;
            auto property_name()         -> PropProxy<Glib::ustring, false>;
            auto property_path()         -> PropProxy<Glib::ustring, false>;
            auto property_state()        -> PropProxy<TOX_FILE_CONTROL>;
            auto property_state_remote() -> PropProxy<TOX_FILE_CONTROL, false>;
            auto property_progress()     -> PropProxy<double, false>;
            auto property_complete()     -> PropProxy<bool, false>;

            auto core()            -> std::shared_ptr<toxmm2::core>;
            auto file_manager()    -> std::shared_ptr<toxmm2::file_manager>;
            auto contact_manager() -> std::shared_ptr<toxmm2::contact_manager>;
            auto contact()         -> std::shared_ptr<toxmm2::contact>;

            ~file() {}

        protected:
            virtual void resume() = 0;
            virtual void send_chunk_request(uint64_t position, size_t length) = 0;
            virtual void recv_chunk(uint64_t position, const std::vector<uint8_t>& data) = 0;
            virtual void finish() = 0;
            virtual void abort() = 0;
            virtual bool is_recv() = 0;

            void pre_send_chunk_request(uint64_t position, size_t length);
            void pre_recv_chunk(uint64_t position, const std::vector<uint8_t>& data);

        private:
            std::weak_ptr<toxmm2::file_manager> m_file_manager;

            Prop<fileId>           m_property_id;
            Prop<fileNr>           m_property_nr;
            Prop<TOX_FILE_KIND>    m_property_kind;
            Prop<uint64_t>         m_property_position;
            Prop<uint64_t>         m_property_size;
            Prop<Glib::ustring>    m_property_name;
            Prop<Glib::ustring>    m_property_path;
            Prop<TOX_FILE_CONTROL> m_property_state;
            Prop<TOX_FILE_CONTROL> m_property_state_remote;
            Prop<double>           m_property_progress;
            Prop<bool>             m_property_complete;

            file(std::shared_ptr<toxmm2::file_manager> manager);
            file(const file&) = delete;
            void operator=(const file&) = delete;

            void init();
    };
}
#endif
