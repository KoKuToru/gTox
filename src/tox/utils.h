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
#ifndef INST_PROP
#define INST_PROP(t, a, n) \
    private: \
    Glib::Property<t> m_ ## a{*this, n}; \
    public: \
    Glib::PropertyProxy<t> a() { \
    return {this, n}; \
    } \
    Glib::PropertyProxy_ReadOnly<t> a() const { \
    return {this, n}; \
    }
#endif
#ifndef INST_PROP_RO
#define INST_PROP_RO(t, a, n) \
    private: \
    Glib::Property<t> m_ ## a{*this, n}; \
    public: \
    Glib::PropertyProxy_ReadOnly<t> a() { \
    return {this, n}; \
    }
#endif
#ifndef INST_SIGNAL
#define INST_SIGNAL(n, ...) \
    private: \
    using type_ ## n = sigc::signal<__VA_ARGS__>; \
    type_ ## n m_ ## n; \
    public: \
    const type_ ## n n() const { \
    return m_ ## n; \
    } \
    type_ ## n n() { \
    return m_ ## n; \
    }
#endif
