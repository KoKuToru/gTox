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
#include "WidgetChatLine.h"
#include "Generated/icon.h"
WidgetChatLine::WidgetChatLine(bool left_side) : m_side(left_side) {

    this->set_halign(m_side?Gtk::Align::ALIGN_START:Gtk::Align::ALIGN_END);
    auto hbox = Gtk::manage(new Gtk::HBox());
    add(*hbox);
    if (m_side) {
        m_avatar.set_halign(Gtk::Align::ALIGN_START);
        hbox->add(m_avatar);
    }
    hbox->add(m_vbox);
    if (!m_side) {
        m_avatar.set_halign(Gtk::Align::ALIGN_END);
        hbox->add(m_avatar);
    }
    m_vbox.set_spacing(5);

    m_avatar.set_name("Avatar");

    this->set_name(m_side?"WidgetChatLineLeft":"WidgetChatLineRight");
    set_size_request(500);
}

WidgetChatLine::~WidgetChatLine(){
}

bool WidgetChatLine::get_side() {
    return m_side;
}

void WidgetChatLine::add_line(unsigned long long timestamp, const Glib::ustring& message) {
    //TODO display timestmap
    (void)timestamp;
    auto label = Gtk::manage(new Gtk::Label());
    label->set_text(message);
    label->set_halign(Gtk::Align::ALIGN_START);
    label->set_selectable(true);
    label->set_line_wrap(true);
    label->set_line_wrap_mode(Pango::WrapMode::WRAP_WORD_CHAR);
    m_vbox.add(*label);
    label->show();

    queue_draw(); //<- wont redraw last line ?
}

void WidgetChatLine::on_size_allocate(Gtk::Allocation& allocation) {
    Gtk::Frame::on_size_allocate(allocation);
    int h = allocation.get_height();
    auto ic = ICON::load_icon(ICON::avatar);
    if (h < ic->get_height()) {
        m_avatar.property_pixbuf() = ICON::load_icon(ICON::avatar)->scale_simple(h, h, Gdk::INTERP_BILINEAR);
    } else {
        //TODO check if already done..
        m_avatar.property_pixbuf() = ICON::load_icon(ICON::avatar);
        m_avatar.set_valign(Gtk::Align::ALIGN_START);
    }
}
