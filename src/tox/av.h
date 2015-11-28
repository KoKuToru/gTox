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
#ifndef TOXMM_AV_H
#define TOXMM_AV_H

#include "types.h"
#include <tox/toxav.h>

namespace toxmm {
    class av : public Glib::Object, public std::enable_shared_from_this<av> {
            friend class core;
        public:
            class pixel {
                private:
                    uint8_t m_r;
                    uint8_t m_g;
                    uint8_t m_b;
                public:
                    static pixel from_yuv(uint8_t y, uint8_t u, uint8_t v) {
                        int r = y + 1.4065 * (v - 128);
                        int g = y - 0.3455 * (u - 128) - 0.7169 * (v - 128);
                        int b = y + 1.7790 * (u - 128);
                        r = std::max(0, std::min(r, 255));
                        g = std::max(0, std::min(g, 255));
                        b = std::max(0, std::min(b, 255));
                        return pixel(r, g, b);
                    }
                    void as_yuv(uint8_t& y, uint8_t& u, uint8_t& v) const {
                        int y_tmp = m_r *  0.299000 + m_g *  0.587000 + m_b *  0.114000;
                        int u_tmp = m_r * -0.168736 + m_g * -0.331264 + m_b *  0.500000 + 128;
                        int v_tmp = m_r *  0.500000 + m_g * -0.418688 + m_b * -0.081312 + 128;
                        y = std::max(0, std::min(y_tmp, 255));
                        u = std::max(0, std::min(u_tmp, 255));
                        v = std::max(0, std::min(v_tmp, 255));
                    }

                    pixel() {}
                    pixel(uint8_t r, uint8_t g, uint8_t b)
                        : m_r(r), m_g(g), m_b(b) {}
                    unsigned char red() {
                        return m_r;
                    }
                    unsigned char green() {
                        return m_g;
                    }
                    unsigned char blue() {
                        return m_b;
                    }
            };

            class image {
                private:
                    int m_w;
                    int m_h;
                    std::vector<pixel> m_pixels;
                public:
                    image(int w, int h)
                        : m_pixels(w*h) {}
                    pixel& operator[](const std::pair<int, int>& pos) {
                        return m_pixels.at(pos.first + pos.second * m_w);
                    }
                    const pixel& operator[](const std::pair<int, int>& pos) const {
                        return m_pixels.at(pos.first + pos.second * m_w);
                    }
                    int width() const {
                        return m_w;
                    }
                    int height() const {
                        return m_h;
                    }
                    pixel* data() {
                        return m_pixels.data();
                    }
                    const pixel* data() const {
                        return m_pixels.data();
                    }
                    size_t size() const {
                        return m_pixels.size();
                    }
            };

            class audio {
                public:
                    enum sr {
                        SAMPLING_RATE_8000 = 8000,
                        SAMPLING_RATE_12000 = 12000,
                        SAMPLING_RATE_16000 = 16000,
                        SAMPLING_RATE_24000 = 24000,
                        SAMPLING_RATE_48000 = 48000
                    };
                    enum ch {
                        CHANNELS_MONO = 1,
                        CHANNELS_STEREO = 2
                    };
                    enum al {
                        AUDIO_2500_US,
                        AUDIO_5000_US,
                        AUDIO_10000_US,
                        AUDIO_20000_US,
                        AUDIO_40000_US,
                        AUDIO_60000_US
                    };

                private:
                    al m_audio_length;
                    ch m_channels;
                    sr m_sampling_rate;
                    std::vector<int16_t> m_data;
                public:
                    audio(al audio_length,
                          ch channels,
                          sr sampling_rate)
                        : m_audio_length(audio_length),
                          m_channels(channels),
                          m_sampling_rate(sampling_rate),
                          m_data(sampling_rate * channels * audio_length / 1000) {}
                    // sample, channel
                    int16_t& operator[](std::pair<size_t, size_t> pos) {
                        return m_data.at(pos.first * 2 + pos.second);
                    }
                    // sample, channel
                    const int16_t& operator[](std::pair<size_t, size_t> pos) const {
                        return m_data.at(pos.first * 2 + pos.second);
                    }
                    al audio_length() const {
                        return m_audio_length;
                    }
                    ch channels() const {
                        return m_channels;
                    }
                    sr sampling_rate() const {
                        return m_sampling_rate;
                    }
                    size_t sample_count() const {
                        return m_data.size() / m_channels;
                    }
                    int16_t* data() {
                        return m_data.data();
                    }
                    const int16_t* data() const {
                        return m_data.data();
                    }
                    size_t size() const {
                        return m_data.size();
                    }
            };

            //signals
            using type_signal_call                = sigc::signal<void, contactNr, bool, bool>;
            using type_signal_call_state          = sigc::signal<void, contactNr, uint32_t>;
            using type_signal_bit_rate_status     = sigc::signal<void, contactNr, uint32_t, uint32_t>;
            using type_signal_audio_receive_frame = sigc::signal<void, contactNr, const audio&>;
            using type_signal_video_receive_frame = sigc::signal<void, contactNr, const image&>;

            type_signal_call                signal_call();
            type_signal_call_state          signal_call_state();
            type_signal_bit_rate_status     signal_bit_rate_status();
            type_signal_audio_receive_frame signal_audio_receive_frame();
            type_signal_video_receive_frame signal_video_receive_frame();

            std::shared_ptr<toxmm::core> core();

            void call(contactNr nr,
                      uint32_t audio_bit_rate,
                      uint32_t video_bit_rate);
            void answer(contactNr nr,
                        uint32_t audio_bit_rate,
                        uint32_t video_bit_rate);
            void call_control(contactNr nr,
                              TOXAV_CALL_CONTROL control);
            void set_bit_rate(contactNr nr,
                              int32_t audio_bit_rate,
                              int32_t video_bit_rate);
            void send_audio_frame(contactNr nr,
                                  const audio &ad);
            void send_video_frame(contactNr nr,
                                  const image &img);

            ~av();

        private:
            ToxAV* m_av;
            std::weak_ptr<toxmm::core> m_core;
            sigc::connection m_update_interval;

            type_signal_call                m_signal_call;
            type_signal_call_state          m_signal_call_state;
            type_signal_bit_rate_status     m_signal_bit_rate_status;
            type_signal_audio_receive_frame m_signal_audio_receive_frame;
            type_signal_video_receive_frame m_signal_video_receive_frame;

            av(const std::shared_ptr<toxmm::core>& core);
            av(const av&) = delete;
            void operator=(const av&) = delete;

            void init();

            bool update();
            uint32_t update_optimal_interval();
    };
}

#endif
