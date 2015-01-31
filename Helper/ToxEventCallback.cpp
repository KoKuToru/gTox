#include "ToxEventCallback.h"
#include <algorithm>

std::vector<ToxEventCallback::EFunc*> ToxEventCallback::m_callback_list;
std::recursive_mutex ToxEventCallback::m_mutex;

ToxEventCallback::ToxEventCallback() {
}

ToxEventCallback::ToxEventCallback(const EFunc& func) : m_callback(func) {
    if (m_callback) {
        // add
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_callback_list.push_back(&m_callback);
    }
}

ToxEventCallback::ToxEventCallback(const ToxEventCallback& o)
    : ToxEventCallback(o.m_callback) {
}

void ToxEventCallback::operator=(const ToxEventCallback& o) {
    operator=(o.m_callback);
}

ToxEventCallback::~ToxEventCallback() {
    if (m_callback) {
        // remove
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_callback_list.erase(std::find(
            m_callback_list.begin(), m_callback_list.end(), &m_callback));
    }
}

void ToxEventCallback::operator=(
    const std::function<void(const Tox::SEvent&)>& func) {
    if (m_callback) {
        // remove
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_callback_list.erase(std::find(
            m_callback_list.begin(), m_callback_list.end(), &m_callback));
    }
    m_callback = func;
    if (m_callback) {
        // add
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_callback_list.push_back(&m_callback);
    }
}

void ToxEventCallback::notify(const Tox::SEvent& data) {
    // call everyone
    std::lock_guard<std::recursive_mutex> lg(m_mutex);
    // 1. make a copy
    auto callback_cpy = m_callback_list;
    // 2. iterate copy
    for (auto func : callback_cpy) {
        // 3. make sure it's still in the list
        if (std::find(m_callback_list.begin(), m_callback_list.end(), func)
            != m_callback_list.end()) {
            // 4. call callback function
            (*func)(data);
        }
    }
}
