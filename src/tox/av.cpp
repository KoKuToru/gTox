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
#include "av.h"
#include "core.h"
#include "exception.h"
#include <iostream>

using namespace toxmm;

av::type_signal_call                av::signal_call() {
    return m_signal_call;
}

av::type_signal_call_state          av::signal_call_state() {
    return m_signal_call_state;
}

av::type_signal_bit_rate_status     av::signal_bit_rate_status() {
    return m_signal_bit_rate_status;
}

av::type_signal_audio_receive_frame av::signal_audio_receive_frame() {
    return m_signal_audio_receive_frame;
}

av::type_signal_video_receive_frame av::signal_video_receive_frame() {
    return m_signal_video_receive_frame;
}

av::av(const std::shared_ptr<toxmm::core>& core)
    : m_av(nullptr),
      m_core(core) {
}

int iter_id = 0;

void av::init() {
    auto c = core();
    if (!c) {
        return;
    }

    TOXAV_ERR_NEW error;
    m_av = toxav_new(c->toxcore(), &error);
    if (error != TOXAV_ERR_NEW_OK) {
        throw exception(error);
    }

    toxav_callback_call(m_av, [](ToxAV*, uint32_t nr, bool audio_enabled, bool video_enabled, void* _this) {
        ((av*)_this)->m_signal_call(contactNr(nr), audio_enabled, video_enabled);
    }, this);
    toxav_callback_call_state(m_av, [](ToxAV*, uint32_t nr, uint32_t state, void* _this) {
        ((av*)_this)->m_signal_call_state(contactNr(nr), state);
    }, this);
    toxav_callback_bit_rate_status(m_av, [](ToxAV*, uint32_t nr, uint32_t audio_bit_rate, uint32_t video_bit_rate, void* _this) {
        ((av*)_this)->m_signal_bit_rate_status(contactNr(nr), audio_bit_rate, video_bit_rate);
    }, this);
    toxav_callback_audio_receive_frame(m_av, [](ToxAV*, uint32_t nr, const int16_t* pcm, size_t sample_count, uint8_t channels, uint32_t sampling_rate, void* _this) {
        audio ad(audio::al(sample_count * 1000 / sampling_rate),
                 audio::ch(channels),
                 audio::sr(sampling_rate));
        // ad.sample_count must be sample_count ! .. TODO: exception ?
        std::copy(pcm, pcm + sample_count * channels, ad.data());
        ((av*)_this)->m_signal_audio_receive_frame(contactNr(nr), ad);
    }, this);
    toxav_callback_video_receive_frame(m_av, [](ToxAV*, uint32_t nr, uint16_t width, uint16_t height, const uint8_t *y, const uint8_t *u, const uint8_t *v, int32_t ystride, int32_t ustride, int32_t vstride, void* _this) {
        std::clog << iter_id << " Got frame " << std::endl;
        image img(width, height);

        int y_size = std::max(width/1, std::abs(ystride)) * height;
        int u_size = std::max(width/2, std::abs(ustride)) * (height / 2);
        int v_size = std::max(width/2, std::abs(vstride)) * (height / 2);

        if (ystride < 0) {
            y = y + y_size;
        }
        if (ustride < 0) {
            u = u + u_size;
        }
        if (vstride < 0) {
            v = v + v_size;
        }

        for (int cy = 0; cy < height; ++cy) {
            const uint8_t* row_y = y + ystride * cy;
            const uint8_t* row_u = u + ustride * (cy / 2);
            const uint8_t* row_v = v + vstride * (cy / 2);
            for (int cx = 0; cx < width; ++cx) {
                img[{cx, cy}] = av::pixel::from_yuv(row_y[cx],
                                                    row_u[cx/2],
                                                    row_v[cx/2]);
            }
        }
        ((av*)_this)->m_signal_video_receive_frame(contactNr(nr), img);
    }, this);

    m_update_interval = Glib::signal_timeout()
                        .connect(sigc::mem_fun(this, &av::update),
                                 update_optimal_interval());
}

void av::call(contactNr nr,
              uint32_t audio_bit_rate,
              uint32_t video_bit_rate) {
    if (!m_av) {
        return;
    }

    TOXAV_ERR_CALL error;
    toxav_call(m_av,
               nr,
               audio_bit_rate,
               video_bit_rate,
               &error);
    if (error != TOXAV_ERR_CALL_OK) {
        throw exception(error);
    }
}

void av::answer(contactNr nr,
                uint32_t audio_bit_rate,
                uint32_t video_bit_rate) {
    if (!m_av) {
        return;
    }

    TOXAV_ERR_ANSWER error;
    toxav_answer(m_av,
                 nr,
                 audio_bit_rate,
                 video_bit_rate,
                 &error);
    if (error != TOXAV_ERR_ANSWER_OK) {
        throw exception(error);
    }
}

void av::call_control(contactNr nr,
                      TOXAV_CALL_CONTROL control) {
    if (!m_av) {
        return;
    }

    TOXAV_ERR_CALL_CONTROL error;
    toxav_call_control(m_av,
                       nr,
                       control,
                       &error);
    if (error != TOXAV_ERR_CALL_CONTROL_OK) {
        throw exception(error);
    }
}

void av::set_bit_rate(contactNr nr,
                      int32_t audio_bit_rate,
                      int32_t video_bit_rate) {
    if (!m_av) {
        return;
    }

    TOXAV_ERR_BIT_RATE_SET error;
    toxav_bit_rate_set(m_av,
                       nr,
                       audio_bit_rate,
                       video_bit_rate,
                       &error);
    if (!error != TOXAV_ERR_BIT_RATE_SET_OK) {
        throw exception(error);
    }
}

void av::send_audio_frame(contactNr nr,
                          const av::audio &ad) {
    if (!m_av) {
        return;
    }

    TOXAV_ERR_SEND_FRAME error;
    toxav_audio_send_frame(m_av,
                           nr,
                           ad.data(),
                           ad.sample_count(),
                           uint8_t(ad.channels()),
                           uint32_t(ad.sampling_rate()),
                           &error);
    if (error != TOXAV_ERR_SEND_FRAME_OK) {
        throw exception(error);
    }
}

void av::send_video_frame(contactNr nr,
                          const av::image &img) {
    std::clog << "send frame" << std::endl;
    if (!m_av) {
        return;
    }

    std::vector<uint8_t> y(img.width() * img.height());
    std::vector<uint8_t> u((img.width() / 2) * (img.height() / 2));
    std::vector<uint8_t> v((img.width() / 2) * (img.height() / 2));

    for (int cy = 0; cy < img.height(); ++cy) {
        for (int cx = 0; cx < img.width(); ++cx) {
            img[{cx, cy}].as_yuv(
                        y.at(cx + cy * img.width()),
                        u.at((cx / 2) + (cy / 2) * (img.width() / 2)),
                        v.at((cx / 2) + (cy / 2) * (img.width() / 2)));
        }
    }

    TOXAV_ERR_SEND_FRAME error;
    toxav_video_send_frame(m_av,
                           nr,
                           img.width(),
                           img.height(),
                           y.data(),
                           u.data(),
                           v.data(),
                           &error);
    if (error != TOXAV_ERR_SEND_FRAME_OK) {
        throw exception(error);
    }
}

av::~av() {
    m_update_interval.disconnect();
    if (m_av) {
        toxav_kill(m_av);
        m_av = nullptr;
    }
}

bool av::update() {
    m_update_interval.disconnect();

    if (m_av) {
        iter_id += 1;
        toxav_iterate(m_av);
    }

    //next round:
    m_update_interval = Glib::signal_timeout()
                        .connect(sigc::mem_fun(this, &av::update),
                                 update_optimal_interval(),
                                 Glib::PRIORITY_DEFAULT_IDLE);
    return false;
}

std::shared_ptr<toxmm::core> av::core() {
    return m_core.lock();
}

uint32_t av::update_optimal_interval() {
    return toxav_iteration_interval(m_av);
}
