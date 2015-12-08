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
#include "call.h"
#include "contact.h"
#include "core.h"
#include "exception.h"

using namespace toxmm;

std::shared_ptr<toxmm::contact> call::contact() {
    return m_contact.lock();
}

std::shared_ptr<toxmm::core> call::core() {
    auto c = contact();
    if (!c) {
        return nullptr;
    }
    auto cr = c->core();
    if (!cr) {
        return nullptr;
    }
    return cr;
}

std::shared_ptr<toxmm::av> call::av() {
    auto cr = core();
    if (!cr) {
        return nullptr;
    }
    return cr->av();
}
#include<iostream>
call::call(const std::shared_ptr<toxmm::contact>& contact)
    : Glib::ObjectBase(typeid(call)),
      m_contact(contact) {

    m_property_state = CALL_CANCEL;
    m_property_remote_state = CALL_CANCEL;
    m_property_suggested_audio_kilobitrate = 30;
    m_property_suggested_video_kilobitrate = 128;
    m_prev_call_state = CALL_CANCEL;

    //install all events
    property_state().signal_changed().connect(sigc::track_obj([this]() {
        auto contact = toxmm::call::contact();
        auto av = toxmm::call::av();
        if (!contact || !av) {
            return;
        }
        try {
            switch (property_state().get_value()) {
                case CALL_RESUME:
                    if (property_remote_state() == CALL_CANCEL) {
                        if (m_prev_call_state != CALL_CANCEL) {
                            break;
                        }
                        av->call(contact->property_nr(),
                                 property_suggested_audio_kilobitrate(),
                                 property_suggested_video_kilobitrate());
                        m_property_video_kilobitrate.set_value(property_suggested_audio_kilobitrate());
                        m_property_audio_kilobitrate.set_value(property_suggested_video_kilobitrate());
                    } else if (m_prev_call_state == CALL_CANCEL) {
                        av->answer(contact->property_nr(),
                                   property_suggested_audio_kilobitrate(),
                                   property_suggested_video_kilobitrate());
                        m_property_video_kilobitrate.set_value(property_suggested_audio_kilobitrate());
                        m_property_audio_kilobitrate.set_value(property_suggested_video_kilobitrate());
                    } else {
                        av->call_control(contact->property_nr(),
                                         TOXAV_CALL_CONTROL_RESUME);
                    }
                    break;
                case CALL_PAUSE:
                    if (property_remote_state() == CALL_CANCEL ||
                        m_prev_call_state == CALL_PAUSE) {
                        break;
                    }
                    av->call_control(contact->property_nr(),
                                     TOXAV_CALL_CONTROL_PAUSE);
                    break;
                case CALL_CANCEL:
                    if (property_remote_state() == CALL_CANCEL) {
                        break;
                    }
                    av->call_control(contact->property_nr(),
                                     TOXAV_CALL_CONTROL_CANCEL);
                    m_property_video_kilobitrate.set_value(0);
                    m_property_audio_kilobitrate.set_value(0);
                    m_property_suggested_audio_kilobitrate = 30;
                    m_property_suggested_video_kilobitrate = 128;
                    m_property_remote_state.set_value(CALL_CANCEL);
                    break;
            }
        } catch(const exception& ex) {
            if (property_state() != CALL_CANCEL) {
                property_state() = CALL_CANCEL;
            }
            if (ex.type() == std::type_index(typeid(TOXAV_ERR_CALL_CONTROL)) &&
                (ex.what_id() == TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_IN_CALL ||
                 ex.what_id() == TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_FOUND)) {
                // ignore this error
            } else if (ex.type() == std::type_index(typeid(TOXAV_ERR_CALL)) &&
                       (ex.what_id() == TOXAV_ERR_CALL_FRIEND_NOT_FOUND ||
                        ex.what_id() == TOXAV_ERR_CALL_FRIEND_NOT_CONNECTED ||
                        ex.what_id() == TOXAV_ERR_CALL_FRIEND_ALREADY_IN_CALL)) {
                // ignore this errro
            } else {
                throw;
            }
        }
        m_prev_call_state = property_state();
    }, *this));
    property_video_frame().signal_changed().connect(sigc::track_obj([this]() {
        auto contact = toxmm::call::contact();
        auto av = toxmm::call::av();
        if (!contact || !av) {
            return;
        }
        try {
            av->send_video_frame(contact->property_nr(),
                                 property_video_frame());
        } catch (const exception& ex) {
            if (ex.type() != std::type_index(typeid(TOXAV_ERR_SEND_FRAME))) {
                throw;
            }
            if (ex.what_id() != TOXAV_ERR_SEND_FRAME_FRIEND_NOT_IN_CALL) {
                throw;
            }
        }
    }, *this));
    property_audio_frame().signal_changed().connect(sigc::track_obj([this]() {
        auto contact = toxmm::call::contact();
        auto av = toxmm::call::av();
        if (!contact || !av) {
            return;
        }
        try {
            av->send_audio_frame(contact->property_nr(),
                                 property_audio_frame());
        } catch (const exception& ex) {
            if (ex.type() != std::type_index(typeid(TOXAV_ERR_SEND_FRAME))) {
                throw;
            }
            if (ex.what_id() != TOXAV_ERR_SEND_FRAME_FRIEND_NOT_IN_CALL) {
                throw;
            }
            m_property_remote_state = CALL_CANCEL;
            m_signal_error();
        }
    }, *this));
    auto update_kilobitrate = sigc::track_obj([this]() {
        auto contact = toxmm::call::contact();
        auto av = toxmm::call::av();
        if (!contact || !av) {
            return;
        }
        try {
            av->set_bit_rate(contact->property_nr(),
                             property_audio_kilobitrate(),
                             property_video_kilobitrate());
        } catch (const exception& ex) {
            if (ex.type() != std::type_index(typeid(TOXAV_ERR_BIT_RATE_SET)) ||
                ex.what_id() != TOXAV_ERR_BIT_RATE_SET_FRIEND_NOT_IN_CALL) {
                m_property_state = CALL_CANCEL;
                m_property_remote_state = CALL_CANCEL;
                m_signal_error();
                throw;
            }
        }
    }, *this);
    property_video_kilobitrate().signal_changed().connect(update_kilobitrate);
    property_audio_kilobitrate().signal_changed().connect(update_kilobitrate);
    signal_error().connect(sigc::track_obj([this]() {
        property_state() = CALL_CANCEL;
    }, *this));
    signal_finish().connect(sigc::track_obj([this]() {
        property_state() = CALL_CANCEL;
    }, *this));
    contact->property_connection().signal_changed().connect(sigc::track_obj([this]() {
        auto ct = toxmm::call::contact();
        if (!ct) {
            return;
        }
        if (ct->property_connection().get_value() == TOX_CONNECTION_NONE) {
            m_property_state = CALL_CANCEL;
            m_property_remote_state = CALL_CANCEL;
        }
    }, *this));
}
