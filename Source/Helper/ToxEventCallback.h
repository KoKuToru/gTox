/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#ifndef TOXEVENTCALLBACK_H
#define TOXEVENTCALLBACK_H

#include <mutex>
#include <functional>
#include "Tox/Toxmm.h"

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

    void reset();

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
