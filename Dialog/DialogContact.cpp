#include "DialogContact.h"

DialogContact::DialogContact() {
    this->set_border_width(1);
    this->set_default_geometry(/*300*/800, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    //Setup titlebar
    m_headerbar_contact.set_title("Contacts");
    m_headerbar_contact.set_subtitle("KoKuToru");
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");
    m_headerbar_chat.set_subtitle("with DemoUser");

    m_header_paned.pack1(m_headerbar_chat  , true, true);
    m_header_paned.pack2(m_headerbar_contact, true, false);

    this->set_titlebar(m_header_paned);

    //Setup content
    m_paned.pack1(m_chat, true, true);
    m_paned.pack2(m_contact, true, true);
    this->add(m_paned);

    //Connect properties C++ version ?
    g_object_bind_property(m_header_paned.gobj(), "position",
                           m_paned.gobj(),        "position",
                           GBindingFlags(G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE));


    this->show_all();
}

DialogContact::~DialogContact() {

}
