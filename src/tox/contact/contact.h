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
#ifndef TOXMM_CONTACT_H
#define TOXMM_CONTACT_H
#include <tox/tox.h>
#include <glibmm.h>
#include <memory>
#include "types.h"
#include <giomm.h>
#include "av.h"

namespace toxmm {
    class file_manager;
    class receipt;
    class call;
    class contact : public Glib::Object, public std::enable_shared_from_this<contact> {
            friend class contact_manager;
            friend class file_manager;
        public:
            //signals
            using type_signal_receipt            = sigc::signal<void, receiptNr>;
            using type_signal_recv_message       = sigc::signal<void, Glib::ustring>;
            using type_signal_recv_action        = sigc::signal<void, Glib::ustring>;
            using type_signal_send_message       = sigc::signal<void, Glib::ustring, std::shared_ptr<receipt>>;
            using type_signal_send_action        = sigc::signal<void, Glib::ustring, std::shared_ptr<receipt>>;
            using type_signal_send_file_chunk_rq = sigc::signal<void, fileNr, uint64_t, size_t>;
            using type_signal_recv_file          = sigc::signal<void, fileNr, TOX_FILE_KIND, size_t, Glib::ustring>;
            using type_signal_recv_file_chunk    = sigc::signal<void, fileNr, uint64_t, const std::vector<uint8_t>&>;
            using type_signal_recv_file_control  = sigc::signal<void, fileNr, TOX_FILE_CONTROL>;
            using type_signal_send_av_video_frame = sigc::signal<void, const toxmm::av::image&>;
            using type_signal_recv_av_video_frame = sigc::signal<void, const toxmm::av::image&>;
            using type_signal_send_av_audio_frame = sigc::signal<void, const toxmm::av::audio&>;
            using type_signal_recv_av_audio_frame = sigc::signal<void, const toxmm::av::audio&>;
            using type_signal_send_av_call        = sigc::signal<void, int, int>;
            using type_signal_recv_av_call        = sigc::signal<void, bool, bool>;
            using type_signal_send_av_answer      = sigc::signal<void, int, int>;
            using type_signal_send_av_control     = sigc::signal<void, TOXAV_CALL_CONTROL>;
            using type_signal_recv_av_error       = sigc::signal<void>;
            using type_signal_recv_av_finish      = sigc::signal<void>;
            using type_signal_recv_av_state       = sigc::signal<void, int>;
            using type_signal_recv_av_bit_rate_status = sigc::signal<void, int, int>;

            type_signal_receipt            signal_receipt();
            type_signal_recv_message       signal_recv_message();
            type_signal_recv_action        signal_recv_action();
            type_signal_send_message       signal_send_message();
            type_signal_send_action        signal_send_action();
            type_signal_send_file_chunk_rq signal_send_file_chunk_request();
            type_signal_recv_file          signal_recv_file();
            type_signal_recv_file_chunk    signal_recv_file_chunk();
            type_signal_recv_file_control  signal_recv_file_control();
            type_signal_send_av_video_frame signal_send_av_video_frame();
            type_signal_recv_av_video_frame signal_recv_av_video_frame();
            type_signal_send_av_audio_frame signal_send_av_audio_frame();
            type_signal_recv_av_audio_frame signal_recv_av_audio_frame();
            type_signal_send_av_call        signal_send_av_call();
            type_signal_recv_av_call        signal_recv_av_call();
            type_signal_send_av_answer      signal_send_av_answer();
            type_signal_send_av_control     signal_send_av_control();
            type_signal_recv_av_error       signal_recv_av_error();
            type_signal_recv_av_finish      signal_recv_av_finish();
            type_signal_recv_av_state       signal_recv_av_state();
            type_signal_recv_av_bit_rate_status signal_recv_av_bit_rate_status();

            //props
            Glib::PropertyProxy_ReadOnly<contactNr>         property_nr();
            Glib::PropertyProxy_ReadOnly<contactAddrPublic> property_addr_public();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_name_or_addr();
            Glib::PropertyProxy_ReadOnly<Glib::ustring>     property_status_message();
            Glib::PropertyProxy_ReadOnly<TOX_USER_STATUS>   property_status();
            Glib::PropertyProxy_ReadOnly<TOX_CONNECTION>    property_connection();
            Glib::PropertyProxy<bool>                       property_typing();
            Glib::PropertyProxy_ReadOnly<bool>              property_remote_typing();
            Glib::PropertyProxy<bool>                       property_av_sending_audio();
            Glib::PropertyProxy<bool>                       property_av_sending_video();
            Glib::PropertyProxy<bool>                       property_av_accepting_audio();
            Glib::PropertyProxy<bool>                       property_av_accepting_video();
            Glib::PropertyProxy_ReadOnly<bool>              property_av_remote_sending_audio();
            Glib::PropertyProxy_ReadOnly<bool>              property_av_remote_sending_video();
            Glib::PropertyProxy_ReadOnly<bool>              property_av_remote_accepting_audio();
            Glib::PropertyProxy_ReadOnly<bool>              property_av_remote_accepting_video();

            //functions
            std::shared_ptr<receipt> send_message(const std::string& message);
            std::shared_ptr<receipt> send_action (const std::string& action);
            void send_av_call();
            void send_av_call_accept();
            void send_av_call_pause();
            void send_av_call_resume();
            void send_av_call_cancel();
            void send_av_video_frame(const toxmm::av::image& img);
            void send_av_audio_frame(const toxmm::av::audio& ad);
            void send_av_control(TOXAV_CALL_CONTROL control);

            std::shared_ptr<toxmm::core> core();
            std::shared_ptr<toxmm::contact_manager> contact_manager();
            std::shared_ptr<toxmm::file_manager> file_manager();
            std::shared_ptr<toxmm::call> call();

        private:
            std::weak_ptr<toxmm::contact_manager> m_contact_manager;
            std::shared_ptr<toxmm::file_manager>  m_file_manager;
            std::shared_ptr<toxmm::call>          m_call;

            Glib::Property<contactNr>         m_property_nr;
            Glib::Property<contactAddrPublic> m_property_addr;
            Glib::Property<Glib::ustring>     m_property_name;
            Glib::Property<Glib::ustring>     m_property_name_or_addr;
            Glib::Property<Glib::ustring>     m_property_status_message;
            Glib::Property<TOX_USER_STATUS>   m_property_status;
            Glib::Property<TOX_CONNECTION>    m_property_connection;
            Glib::Property<bool>              m_property_typing;
            Glib::Property<bool>              m_property_remote_typing;
            Glib::Property<bool>              m_property_av_sending_audio;
            Glib::Property<bool>              m_property_av_sending_video;
            Glib::Property<bool>              m_property_av_accepting_audio;
            Glib::Property<bool>              m_property_av_accepting_video;
            Glib::Property<bool>              m_property_av_remote_sending_audio;
            Glib::Property<bool>              m_property_av_remote_sending_video;
            Glib::Property<bool>              m_property_av_remote_accepting_audio;
            Glib::Property<bool>              m_property_av_remote_accepting_video;

            type_signal_receipt            m_signal_receipt;
            type_signal_recv_message       m_signal_recv_message;
            type_signal_recv_action        m_signal_recv_action;
            type_signal_send_message       m_signal_send_message;
            type_signal_send_action        m_signal_send_action;
            type_signal_send_file_chunk_rq m_signal_send_file_chunk_rq;
            type_signal_recv_file          m_signal_recv_file;
            type_signal_recv_file_chunk    m_signal_recv_file_chunk;
            type_signal_recv_file_control  m_signal_recv_file_control;
            type_signal_send_av_video_frame m_signal_send_av_video_frame;
            type_signal_recv_av_video_frame m_signal_recv_av_video_frame;
            type_signal_send_av_audio_frame m_signal_send_av_audio_frame;
            type_signal_recv_av_audio_frame m_signal_recv_av_audio_frame;
            type_signal_send_av_call        m_signal_send_av_call;
            type_signal_recv_av_call        m_signal_recv_av_call;
            type_signal_send_av_answer      m_signal_send_av_answer;
            type_signal_recv_av_error       m_signal_recv_av_error;
            type_signal_send_av_control     m_signal_send_av_control;
            type_signal_recv_av_finish      m_signal_recv_av_finish;
            type_signal_recv_av_state       m_signal_recv_av_state;
            type_signal_recv_av_bit_rate_status m_signal_recv_av_bit_rate_status;

            std::shared_ptr<toxmm::file>  m_avatar_send;
            Glib::RefPtr<Gio::FileMonitor> m_avatar_send_monitor;

            contact(std::shared_ptr<toxmm::contact_manager> manager, contactNr nr);
            contact(const contact&) = delete;
            void operator=(const contact&) = delete;

            void init();

            contactAddrPublic toxcore_get_addr();
            Glib::ustring     toxcore_get_name();
            Glib::ustring     toxcore_get_status_message();
            TOX_USER_STATUS   toxcore_get_status();
            TOX_CONNECTION    toxcore_get_connection();
    };

}

#endif
