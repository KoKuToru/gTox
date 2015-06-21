#include "gStreamerHelper.h"

namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

//TODO: Make this platform independent

std::vector<gStreamerHelper::Device> gStreamerHelper::probe_devices(const char* classes) {
    //INFO: There is no C++ - version of this..
    //TODO: Memory Leak check ?
    GstDeviceMonitor *monitor = gst_device_monitor_new();

    //Filter
    gst_device_monitor_add_filter(monitor, classes, nullptr);

    //Get all devices
    decltype(probe_devices()) result;
    GList *list = gst_device_monitor_get_devices(monitor);
    GList *it = list;
    while (it != nullptr)
    {
        auto device = (GstDevice*)it->data;
        auto device_name  = gst_device_get_display_name(device);
        result.push_back(Device(device_name, device));
        g_free(device_name);
        it = it->next;
    }

    return result;
}

gStreamerHelper::Device gStreamerHelper::probe_device_by_name(Glib::ustring name) {
    for(auto x : probe_devices()) {
        if (x.name == name) {
            return x;
        }
    }
    throw std::runtime_error("Gst Device not found");
}

const std::shared_ptr<gStreamerVideo> gStreamerHelper::create(std::string uri) {
    std::lock_guard<std::recursive_mutex> lg(m_mutex);

    auto iter = m_videos.find(uri);
    if (iter != m_videos.end()) {
        return iter->second.lock();
    }

    auto tmp = std::shared_ptr<gStreamerVideo>(new gStreamerVideo(uri), [this, uri](const gStreamerVideo* ori){
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        auto iter = m_videos.find(uri);
        if (iter != m_videos.end()) {
            m_videos.erase(iter);
        }
        delete ori;
    });
    m_videos.insert({uri, tmp});
    return tmp;
}

gStreamerHelper::~gStreamerHelper() {
    //make sure m_videos is empty ?
}
