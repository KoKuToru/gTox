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
#ifndef TOXMM_CONTACT_MANAGER_H
#define TOXMM_CONTACT_MANAGER_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"

namespace toxmm2 {
    class contact_manager : public std::enable_shared_from_this<contact_manager> {
            friend class core;
        public:
            using type_signal_request = sigc::signal<void, contactAddr, Glib::ustring>;
            using type_signal_removed = sigc::signal<void, std::shared_ptr<contact>>;
            using type_signal_added   = sigc::signal<void, std::shared_ptr<contact>>;

            type_signal_request signal_request();
            type_signal_removed signal_removed();
            type_signal_added   signal_added();

            std::shared_ptr<contact> find(contactAddrPublic addr);
            std::shared_ptr<contact> find(contactNr nr);

            const std::vector<std::shared_ptr<contact>>& get_all();

            void add_contact(contactAddrPublic addr_public);
            void add_contact(contactAddr addr, const Glib::ustring& message);

            void destroy();
            ~contact_manager();

            std::shared_ptr<toxmm2::core> core();

        private:
            std::weak_ptr<toxmm2::core> m_core;

            std::vector<std::shared_ptr<contact>> m_contact;

            type_signal_request m_signal_request;
            type_signal_removed m_signal_removed;
            type_signal_added   m_signal_added;

            contact_manager(std::shared_ptr<toxmm2::core> core);
            contact_manager(const contact_manager&) = delete;
            void operator=(const contact_manager&) = delete;

            void init();
    };

}

#endif
