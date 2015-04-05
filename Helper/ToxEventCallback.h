#ifndef TOXEVENTCALLBACK_H
#define TOXEVENTCALLBACK_H

#include <mutex>
#include <functional>
#include "Tox/Tox.h"

/**
 * @brief Installs a callback for tox events
 */
class ToxEventCallback {
  public:
    typedef std::function<void(const ToxEvent&)> EFunc;

    ToxEventCallback();
    ToxEventCallback(const EFunc& func);
    ToxEventCallback(const ToxEventCallback& o);
    ~ToxEventCallback();

    void operator=(const EFunc& func);
    void operator=(const ToxEventCallback& o);

    /**
     * @brief Call all installed callbacks with the event data
     * @param data
     */
    static void notify(const ToxEvent& data);

  private:
    /**
     * @brief Protects the m_callback_list
     */
    static std::recursive_mutex m_mutex;
    /**
     * @brief List of all installed event callbacks
     */
    static std::vector<EFunc*> m_callback_list;
    EFunc m_callback;

    /**
     * @brief Adds the callback to the m_callback_list
     */
    void install();
    /**
     * @brief Removes the callback fromt he m_callback_list
     */
    void uninstall();
};

#endif
