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
#ifndef TOXMM_TYPES_H
#define TOXMM_TYPES_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>

namespace toxmm2 {
    class core;
    class file;
    class contact;
    class contact_manager;
    class profile;

    class contactNr {
        private:
            uint32_t m_nr;
        public:
            operator decltype(m_nr)() { return m_nr; }
            operator std::string() { return std::to_string(m_nr); }
            contactNr(): m_nr() {}
            contactNr(decltype(m_nr) nr): m_nr(nr) {}
            contactNr(const std::string& nr): m_nr(std::stoi(nr)) {}
            bool operator==(const contactNr& o) const { return m_nr == o.m_nr; }
            bool operator!=(const contactNr& o) const { return m_nr != o.m_nr; }
            bool operator< (const contactNr& o) const { return m_nr <  o.m_nr; }
            bool operator<=(const contactNr& o) const { return m_nr <= o.m_nr; }
            bool operator> (const contactNr& o) const { return m_nr >  o.m_nr; }
            bool operator>=(const contactNr& o) const { return m_nr >= o.m_nr; }
    };
    class receiptNr {
        private:
            uint32_t m_nr;
        public:
            operator decltype(m_nr)() { return m_nr; }
            operator std::string() { return std::to_string(m_nr); }
            receiptNr(): m_nr() {}
            receiptNr(decltype(m_nr) nr): m_nr(nr) {}
            receiptNr(const std::string& nr): m_nr(std::stoi(nr)) {}
            bool operator==(const receiptNr& o) const { return m_nr == o.m_nr; }
            bool operator!=(const receiptNr& o) const { return m_nr != o.m_nr; }
            bool operator< (const receiptNr& o) const { return m_nr <  o.m_nr; }
            bool operator<=(const receiptNr& o) const { return m_nr <= o.m_nr; }
            bool operator> (const receiptNr& o) const { return m_nr >  o.m_nr; }
            bool operator>=(const receiptNr& o) const { return m_nr >= o.m_nr; }
    };
    class fileNr {
        private:
            uint32_t m_nr;
        public:
            operator decltype(m_nr)() { return m_nr; }
            operator std::string() { return std::to_string(m_nr); }
            fileNr(): m_nr() {}
            fileNr(decltype(m_nr) nr): m_nr(nr) {}
            fileNr(const std::string& nr): m_nr(std::stoi(nr)) {}
            bool operator==(const fileNr& o) const { return m_nr == o.m_nr; }
            bool operator!=(const fileNr& o) const { return m_nr != o.m_nr; }
            bool operator< (const fileNr& o) const { return m_nr <  o.m_nr; }
            bool operator<=(const fileNr& o) const { return m_nr <= o.m_nr; }
            bool operator> (const fileNr& o) const { return m_nr >  o.m_nr; }
            bool operator>=(const fileNr& o) const { return m_nr >= o.m_nr; }
    };
    class contactAddrPublic {
        private:
            std::array<uint8_t, TOX_PUBLIC_KEY_SIZE> m_addr;
            std::string to_hex();
            decltype(m_addr) from_hex(const std::string& hex);
        public:
            operator decltype(m_addr)() { return m_addr; }
            operator std::string() { return to_hex(); }
            operator uint8_t*() { return m_addr.data(); }
            contactAddrPublic(): m_addr() {}
            contactAddrPublic(const uint8_t* addr) {
                std::copy(addr, addr + m_addr.size(), m_addr.begin());
            }
            contactAddrPublic(decltype(m_addr) addr): m_addr(addr) {}
            contactAddrPublic(const std::string& addr): m_addr(from_hex(addr)) {}
            bool operator==(const contactAddrPublic& o) const { return m_addr == o.m_addr; }
            bool operator!=(const contactAddrPublic& o) const { return m_addr != o.m_addr; }
            bool operator< (const contactAddrPublic& o) const { return m_addr <  o.m_addr; }
            bool operator<=(const contactAddrPublic& o) const { return m_addr <= o.m_addr; }
            bool operator> (const contactAddrPublic& o) const { return m_addr >  o.m_addr; }
            bool operator>=(const contactAddrPublic& o) const { return m_addr >= o.m_addr; }
    };
    class contactAddr {
        private:
           std::array<uint8_t, TOX_ADDRESS_SIZE> m_addr;
           std::string to_hex();
           decltype(m_addr) from_hex(const std::string& hex);
        public:
           operator decltype(m_addr)() { return m_addr; }
           operator std::string() { return to_hex(); }
           operator uint8_t*() { return m_addr.data(); }
           operator contactAddrPublic() { return contactAddrPublic(m_addr.data()); }
           contactAddr(): m_addr() {}
           contactAddr(const uint8_t* addr) {
               std::copy(addr, addr + m_addr.size(), m_addr.begin());
           }
           contactAddr(decltype(m_addr) addr): m_addr(addr) {}
           contactAddr(const std::string& addr): m_addr(from_hex(addr)) {}
           bool operator==(const contactAddr& o) const { return m_addr == o.m_addr; }
           bool operator!=(const contactAddr& o) const { return m_addr != o.m_addr; }
           bool operator< (const contactAddr& o) const { return m_addr <  o.m_addr; }
           bool operator<=(const contactAddr& o) const { return m_addr <= o.m_addr; }
           bool operator> (const contactAddr& o) const { return m_addr >  o.m_addr; }
           bool operator>=(const contactAddr& o) const { return m_addr >= o.m_addr; }
    };
    class fileId {
        private:
            std::array<uint8_t, TOX_FILE_ID_LENGTH> m_id;
            std::string to_hex();
            decltype(m_id) from_hex(const std::string& hex);
        public:
            operator decltype(m_id)() { return m_id; }
            operator std::string() { return to_hex(); }
            operator uint8_t*() { return m_id.data(); }
            fileId(): m_id() {}
            fileId(const uint8_t* id) {
                std::copy(id, id + m_id.size(), m_id.begin());
            }
            fileId(decltype(m_id) id): m_id(id) {}
            fileId(const std::string& id): m_id(from_hex(id)) {}
            bool operator==(const fileId& o) const { return m_id == o.m_id; }
            bool operator!=(const fileId& o) const { return m_id != o.m_id; }
            bool operator< (const fileId& o) const { return m_id <  o.m_id; }
            bool operator<=(const fileId& o) const { return m_id <= o.m_id; }
            bool operator> (const fileId& o) const { return m_id >  o.m_id; }
            bool operator>=(const fileId& o) const { return m_id >= o.m_id; }
    };
    class hash {
         private:
            std::array<uint8_t, TOX_HASH_LENGTH> m_hash;
            std::string to_hex();
            decltype(m_hash) from_hex(const std::string& hex);
        public:
            operator decltype(m_hash)() { return m_hash; }
            operator std::string() { return to_hex(); }
            operator uint8_t*() { return m_hash.data(); }
            hash(): m_hash() {}
            hash(const uint8_t* hash) {
                std::copy(hash, hash + m_hash.size(), m_hash.begin());
            }
            hash(decltype(m_hash) hash): m_hash(hash) {}
            hash(const std::string& hash): m_hash(from_hex(hash)) {}
            bool operator==(const hash& o) const { return m_hash == o.m_hash; }
            bool operator!=(const hash& o) const { return m_hash != o.m_hash; }
            bool operator< (const hash& o) const { return m_hash <  o.m_hash; }
            bool operator<=(const hash& o) const { return m_hash <= o.m_hash; }
            bool operator> (const hash& o) const { return m_hash >  o.m_hash; }
            bool operator>=(const hash& o) const { return m_hash >= o.m_hash; }
    };
}

#endif
