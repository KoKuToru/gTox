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
#include "DialogCss.h"
#include "Generated/theme.h"

DialogCss::DialogCss() {
    this->set_border_width(1);
    this->set_default_geometry(256, 256);
    this->set_position(Gtk::WindowPosition::WIN_POS_NONE);

    // Setup titlebar
    m_header.set_title("CSS Debug");
    // m_header.set_subtitle("with DemoUser");
    m_header.set_show_close_button();

    this->set_titlebar(m_header);

    m_text.get_buffer()->set_text(THEME::main);
    m_text.get_buffer()->signal_changed().connect([this]() {
        try {
            auto css = Gtk::CssProvider::create();
            if (css->load_from_data(m_text.get_buffer()->get_text())) {
                auto screen = Gdk::Screen::get_default();
                auto ctx = get_style_context();
                if (m_last) {
                    ctx->remove_provider_for_screen(screen, m_last);
                }
                ctx->add_provider_for_screen(
                    screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                m_last = css;
                m_header.set_subtitle("SUCCESS");
            } else {
                m_header.set_subtitle("ERROR");
            }
        }
        catch (...) {
            m_header.set_subtitle("ERROR");
        }
    });

    auto scroll = Gtk::manage(new Gtk::ScrolledWindow());
    scroll->add(m_text);
    add(*scroll);
}

DialogCss::~DialogCss() {}

void DialogCss::show() { this->show_all(); }
