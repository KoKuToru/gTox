#ifndef TOXEVENTCALLBACK_H
#define TOXEVENTCALLBACK_H

#include <mutex>
#include <functional>
#include "Tox/Tox.h"

class ToxEventCallback {
  public:
    typedef std::function<void(const Tox::SEvent&)> EFunc;

    ToxEventCallback();
    ToxEventCallback(const EFunc& func);
    ToxEventCallback(const ToxEventCallback& o);
    ~ToxEventCallback();

    void operator=(const EFunc& func);
    void operator=(const ToxEventCallback& o);

    static void notify(const Tox::SEvent& data);

  private:
    static std::recursive_mutex m_mutex;
    static std::vector<EFunc*> m_callback_list;
    EFunc m_callback;

    void install();
    void uninstall();
};

#endif
