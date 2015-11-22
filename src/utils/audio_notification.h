#ifndef GTOX_NOTIFICATION_H
#define GTOX_NOTIFICATION_H

#include <vector>
#include "gstreamer.h"

namespace utils {
    class audio_notification {
        private:
            static std::vector<audio_notification*> m_others;

            gstreamer m_audio_player;

        public:
            audio_notification(const audio_notification&) = delete;
            audio_notification(const Glib::ustring& audio_uri);

            void operator=(const audio_notification&) = delete;

            ~audio_notification();
    };
}

#endif
