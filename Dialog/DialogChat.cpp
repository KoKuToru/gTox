#include "DialogChat.h"

DialogChat::DialogChat() {
    this->set_border_width(1);
    this->set_default_geometry(/*300*/800, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    //Setup titlebar
    m_header.set_title("Chat");
    m_header.set_subtitle("with DemoUser");
    //m_header.set_show_close_button();

    this->set_titlebar(m_header);

    //Setup content
    this->add(m_chat);

    this->show_all();
}

DialogChat::~DialogChat() {

}
