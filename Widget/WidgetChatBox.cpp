#include "WidgetChatBox.h"

WidgetChatBox::WidgetChatBox(){
    m_ScrolledWindow.add(m_TextView);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_TextBuffer = Gtk::TextBuffer::create();
    m_TextBuffer->set_text("Hello World!");
    m_TextView.set_buffer(m_TextBuffer);
    pack_start(m_ScrolledWindow);
}

WidgetChatBox::~WidgetChatBox(){
}
