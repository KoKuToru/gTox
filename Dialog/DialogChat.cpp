#include "DialogChat.h"

DialogChat::DialogChat() {
    this->set_border_width(1);
    this->set_default_geometry(/*300*/800, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    //Setup titlebar
    m_headerbar_contact.set_title("Demo");
    m_headerbar_contact.set_subtitle("...");
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");

    m_header_paned.pack1(m_headerbar_chat  , true, true);
    m_header_paned.pack2(m_headerbar_contact, true, false);

    this->set_titlebar(m_header_paned);

    //Setup content
    m_content_paned.pack1(m_content_chat, true, true);
    m_content_paned.pack2(m_content_contact, true, true);
    this->add(m_content_paned);

    //Connect properties C++ version ?
    g_object_bind_property(m_header_paned.gobj(), "position",
                           m_content_paned.gobj(), "position",
                           GBindingFlags(G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE));

    //test data
    m_content_chat.add(m_test1);
    m_content_chat.add(m_test2);

    m_test3.set_text("test3");
    m_content_contact.pack_start(m_test3, false, false);
    m_test4.set_text("test4");
    m_content_contact.pack_start(m_test4, false, false);

    this->show_all();
}

DialogChat::~DialogChat() {

}
