#include "DialogChat.h"

DialogChat::DialogChat() {
    this->set_border_width(1);
    this->set_default_geometry(256, 256);
    this->set_position(Gtk::WindowPosition::WIN_POS_NONE);

    //Setup titlebar
    m_header.set_title("Chat");
    m_header.set_subtitle("with DemoUser");
    //m_header.set_show_close_button();

    this->set_titlebar(m_header);

    //Setup content
    this->add(m_chat);
}

DialogChat::~DialogChat() {

}

void DialogChat::show() {
    this->show_all();
}
