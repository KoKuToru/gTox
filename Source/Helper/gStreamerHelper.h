#ifndef GSTREAMER_HELPER
#define GSTREAMER_HELPER
#include <map>
#include <string>
#include <memory>
#include <glibmm.h>
#include <gstreamermm.h>
#include "Video/gStreamerVideo.h"

class gStreamerHelper {
    private:
        std::map<std::string, std::weak_ptr<gStreamerVideo>> m_videos;
        std::recursive_mutex m_mutex;

    public:
        class Device {
            public:
                Glib::ustring name;
                GstDevice* device;

                Device(const Glib::ustring& name, GstDevice* device): name(name), device((decltype(device))gst_object_ref(device)) {
                }
                Device(const Device& other): name(other.name), device((decltype(device))gst_object_ref(other.device)) {
                }
                void operator=(const Device& other) {
                    name = other.name;
                    auto tmp = device;
                    device = (decltype(device))gst_object_ref(other.device);
                    gst_object_unref(tmp);
                }
                ~Device() {
                    gst_object_unref(device);
                }
        };

        constexpr static const char* CLASS_VIDEO_INPUT = "Video/Source";
        constexpr static const char* CLASS_AUDIO_INPUT = "Audio/Source";
        constexpr static const char* CLASS_AUDIO_OUTPUT = "Audio/Sink";

        static std::vector<Device> probe_devices(const char* classes = CLASS_VIDEO_INPUT);
        static Device probe_device_by_name(Glib::ustring name);

        const std::shared_ptr<gStreamerVideo> create(std::string uri);

        ~gStreamerHelper();
};

#endif
