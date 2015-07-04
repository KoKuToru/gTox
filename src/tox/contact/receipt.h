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
#ifndef TOXMM_CONTACT_RECEIPT_H
#define TOXMM_CONTACT_RECEIPT_H

#include <glibmm.h>
#include "types.h"

namespace toxmm2 {
    class receipt : public Glib::Object, public std::enable_shared_from_this<receipt> {
            friend class contact;
        private:
            typedef sigc::signal<void> type_signal_okay;
            typedef sigc::signal<void> type_signal_lost;

            type_signal_okay signal_okay();
            type_signal_lost signal_lost();

            Glib::PropertyProxy_ReadOnly<receiptNr> property_nr();

        private:
            receipt(std::shared_ptr<contact> contact, receiptNr nr);
            receipt(const receipt&) = delete;
            void operator=(const receipt&) = delete;

            Glib::Property<receiptNr> m_property_nr;

            type_signal_okay m_signal_okay;
            type_signal_lost m_signal_lost;

            sigc::connection m_connection;
    };
}

#endif
