#ifndef GSTREAMERVIDEO_H
#define GSTREAMERVIDEO_H

#include <vector>
#include <glibmm.h>
#include <gstreamermm.h>
#include <gstreamermm/playbin.h>
#include <gstreamermm/pipeline.h>
#include <gstreamermm/appsink.h>
#include <mutex>

class gStreamerVideo {
    public:
        enum State {
            UNKNOW,
            PLAY,
            PAUSE,
            STOP
        };

        typedef sigc::signal<void, int, int, const std::vector<unsigned char>&> type_signal_update;
        type_signal_update signal_update() {
            return m_update;
        }
        typedef sigc::signal<void, std::string> type_signal_error;
        type_signal_error signal_error() {
            return m_error;
        }

        gStreamerVideo(std::string uri);

        void play();
        void pause();
        void stop();

        bool get_progress(gint64& position, gint64& duration);

        void emit_update_signal();

        ~gStreamerVideo();

    private:
        std::shared_ptr<bool> m_alive;
        Glib::RefPtr<Gst::PlayBin>  m_playbin;
        Glib::RefPtr<Gst::Pipeline> m_pipeline;
        Glib::RefPtr<Gst::AppSink>  m_appsink;

        int m_width;
        int m_height;

        std::vector<unsigned char> m_lastframe;
        std::recursive_mutex m_mutex;
        std::recursive_mutex m_mutex_block_destroy;

        State m_state = STOP;

        void init();
        void init_bus(Glib::RefPtr<Gst::Bus> bus);
        void set_state(Gst::State state);

    protected:
        type_signal_update m_update;
        type_signal_error m_error;

};

#endif
