#include "Canberra.h"
#include <iostream>
#include <canberra.h>

void Canberra::play(const std::string& name) {
    static ca_context* ca_instance = []() {
        ca_context* tmp;
        ca_context_create(&tmp);
        return tmp;
    }();
    if (ca_context_play(
            ca_instance, 0, CA_PROP_MEDIA_FILENAME, name.c_str(), nullptr)
        == 0) {
        return;
    }
    if (ca_context_play(ca_instance, 0, CA_PROP_EVENT_ID, name.c_str(), nullptr)
        == 0) {
        return;
    }
    // error:
    std::clog << "libcanberra: Couln't play sound \"" << name << "\""
              << std::endl;
}
