#include "audio_notification.h"

using namespace utils;

std::vector<audio_notification*> audio_notification::m_others;

audio_notification::audio_notification(const Glib::ustring& audio_uri) {
    m_audio_player.signal_error().connect(sigc::track_obj([this](Glib::ustring error) {
        std::cerr << "GSTREAMER-ERROR: " << error << std::endl;
        delete this;
    }, *this));
    m_audio_player.property_position().signal_changed().connect(sigc::track_obj([this]() {
        delete this;
    }, *this));
    m_audio_player.property_uri() = audio_uri;
    m_audio_player.property_volume() = 1;
    m_audio_player.property_state() = Gst::STATE_PLAYING;

    for (auto n : m_others) {
        n->m_audio_player.property_volume() = 0;
    }
    m_others.push_back(this);
}

audio_notification::~audio_notification() {
    auto iter = std::find(
                    m_others.begin(),
                    m_others.end(),
                    this);
    if (iter != m_others.end()) {
        m_others.erase(iter);
    }
}
