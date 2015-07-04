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
#include "types.h"
#include "core.h"

using namespace toxmm2;

std::string contactAddr::to_hex() {
    return core::to_hex(m_addr.begin(), m_addr.size());
}

decltype(contactAddr::m_addr) contactAddr::from_hex(const std::string& hex) {
    decltype(contactAddr::m_addr) tmp;
    auto tmp_hex = core::from_hex(hex);
    std::copy(tmp_hex.begin(), tmp_hex.begin() + std::min(tmp_hex.size(), tmp.size()), tmp.begin());
    return tmp;
}

std::string contactPublicAddr::to_hex() {
    return core::to_hex(m_addr.begin(), m_addr.size());
}

decltype(contactPublicAddr::m_addr) contactPublicAddr::from_hex(const std::string& hex) {
    decltype(contactPublicAddr::m_addr) tmp;
    auto tmp_hex = core::from_hex(hex);
    std::copy(tmp_hex.begin(), tmp_hex.begin() + std::min(tmp_hex.size(), tmp.size()), tmp.begin());
    return tmp;
}

std::string fileId::to_hex() {
    return core::to_hex(m_id.begin(), m_id.size());
}

decltype(fileId::m_id) fileId::from_hex(const std::string& hex) {
    decltype(fileId::m_id) tmp;
    auto tmp_hex = core::from_hex(hex);
    std::copy(tmp_hex.begin(), tmp_hex.begin() + std::min(tmp_hex.size(), tmp.size()), tmp.begin());
    return tmp;
}

std::string hash::to_hex() {
    return core::to_hex(m_hash.begin(), m_hash.size());
}

decltype(hash::m_hash) hash::from_hex(const std::string& hex) {
    decltype(hash::m_hash) tmp;
    auto tmp_hex = core::from_hex(hex);
    std::copy(tmp_hex.begin(), tmp_hex.begin() + std::min(tmp_hex.size(), tmp.size()), tmp.begin());
    return tmp;
}
