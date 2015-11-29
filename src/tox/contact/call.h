/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics

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
#ifndef TOXMM_CONTACT_CALL_H
#define TOXMM_CONTACT_CALL_H

#include <glibmm.h>
#include "types.h"
#include "av.h"

// check if these macros are working nice without proplems
// and use them everywhere
#define INST_PROP(t, a, n) \
    private: \
        Glib::Property<t> m_ ## a{*this, n}; \
    public: \
        Glib::PropertyProxy<t> a() { \
            return {this, n}; \
        } \
        Glib::PropertyProxy_ReadOnly<t> a() const { \
            return {this, n}; \
        }
#define INST_PROP_RO(t, a, n) \
    private: \
        Glib::Property<t> m_ ## a{*this, n}; \
    public: \
        Glib::PropertyProxy_ReadOnly<t> a() { \
            return {this, n}; \
        }
#define INST_SIGNAL(n, ...) \
    private: \
        using type_ ## n = sigc::signal<__VA_ARGS__>; \
        type_ ## n m_ ## n; \
    public: \
        const type_ ## n n() const { \
            return m_ ## n; \
        } \
        type_ ## n n() { \
            return m_ ## n; \
        }

namespace toxmm {
    class call : public Glib::Object, public std::enable_shared_from_this<call> {
            friend class contact;
            friend class contact_manager;
        public:
            enum CALL_STATE {
                CALL_RESUME,
                CALL_PAUSE,
                CALL_CANCEL
            };

            std::shared_ptr<toxmm::contact> contact();
            std::shared_ptr<toxmm::core> core();
            std::shared_ptr<toxmm::av> av();

        private:
            std::weak_ptr<toxmm::contact> m_contact;

            call(const std::shared_ptr<toxmm::contact>& contact);

            // Install all properties
            INST_PROP    (CALL_STATE, property_state, "call-state")
            INST_PROP    (av::image , property_video_frame, "call-video-frame")
            INST_PROP    (av::audio , property_audio_frame, "call-audio-frame")
            INST_PROP_RO (CALL_STATE, property_remote_state, "call-remote-state")
            INST_PROP_RO (av::image , property_remote_video_frame, "call-remote-video-frame")
            INST_PROP_RO (av::audio , property_remote_audio_frame, "call-remote-audio-frame")
            INST_PROP    (uint32_t  , property_video_kilobitrate, "call-video-kilobitrate")
            INST_PROP    (uint32_t  , property_audio_kilobitrate, "call-audio-kilobitrate")
            INST_PROP_RO (uint32_t  , property_suggested_video_kilobitrate, "call-suggested-video-kilobitrate")
            INST_PROP_RO (uint32_t  , property_suggested_audio_kilobitrate, "call-suggested-audio-kilobitrate")

            // Install all signals
            INST_SIGNAL (signal_incoming_call     , void)
            INST_SIGNAL (signal_error             , void)
            INST_SIGNAL (signal_finish            , void)
            INST_SIGNAL (signal_suggestion_updated, void)
    };
}

#endif
