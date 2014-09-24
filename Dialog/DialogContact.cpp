#include "DialogContact.h"
#include "../Generated/icon.h"
#include <gdkmm.h>

Glib::RefPtr<Gdk::Pixbuf> load_icon(const std::string data) {
    auto tmp = Gio::MemoryInputStream::create();
    tmp->add_data(data);
    return Gdk::Pixbuf::create_from_stream(tmp);
}

DialogContact::DialogContact():
    m_icon_attach(load_icon(ICON::chat_attach)),
    m_icon_detach(load_icon(ICON::chat_detach))
{


    this->set_border_width(1);
    this->set_default_geometry(/*300*/800, 600);
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);

    //Setup titlebar
    m_headerbar_contact.set_title("Contacts");
    m_headerbar_contact.set_subtitle("KoKuToru");
    m_headerbar_contact.set_show_close_button();

    m_headerbar_chat.set_title("Chat");
    m_headerbar_chat.set_subtitle("with DemoUser");

    m_btn_xxtach.set_image(m_icon_detach);

    m_headerbar_btn_right.get_style_context()->add_class("linked");
    m_headerbar_btn_right.add(m_btn_xxtach);
    m_headerbar_chat.pack_end(m_headerbar_btn_right);

    m_header_paned.pack1(m_headerbar_chat  , true, true);
    m_header_paned.pack2(m_headerbar_contact, false, true);

    this->set_titlebar(m_header_paned);

    //Setup content
    m_paned.pack1(m_chat, true, true);
    m_paned.pack2(m_contact, false, true);
    this->add(m_paned);

    //Connect properties C++ version ?
    g_object_bind_property(m_header_paned.gobj(), "position",
                           m_paned.gobj(),        "position",
                           GBindingFlags(G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE));

    //events
    m_btn_xxtach.signal_clicked().connect(sigc::mem_fun(this, &DialogContact::detachChat));

    this->show_all();
}

DialogContact::~DialogContact() {

}

void DialogContact::detachChat() {
    this->property_gravity() = Gdk::GRAVITY_NORTH_WEST;
    int x,y;
    this->get_position(x, y);
    this->property_gravity() = Gdk::GRAVITY_NORTH_EAST;
    int w,h;
    this->get_size(w, h);
    int hw = m_headerbar_chat.get_width();
    w -= hw;
    this->resize(w, h);

    m_header_paned.remove(m_headerbar_chat);
    m_paned.remove(m_chat);

    m_chat_dialog.move(x, y);
    m_chat_dialog.resize(hw+300, h); //too small why ?
    m_chat_dialog.show();
}
