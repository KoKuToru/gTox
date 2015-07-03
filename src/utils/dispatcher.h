#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <memory>
#include <glibmm.h>

namespace utils {
    /**
     * @brief The Dispatcher, executes given function on gtk main loop.
     *
     * Makes sure the class-instance with the dispatcher still exists.
     * Simplifies multithreading programming a lot.
     */
    class dispatcher {
        public:
            /**
             * Makes a weak reference to a Dispatcher.
             * Needed when you don't join threads in the class-instance with the dispatcher.
             */
            class ref {
                private:
                    std::weak_ptr<bool> m_exists;
                public:
                    ref(const dispatcher& o): m_exists(o.m_exists) {}
                    template<typename T> void emit(T f) const {
                        std::weak_ptr<bool> weak = m_exists;
                        Glib::signal_idle().connect_once([f, weak]() {
                            auto strong = weak.lock();
                            if (strong) {
                                f();
                            }
                        });
                    }
            };

            friend class dispatcher::ref;
        private:
            std::shared_ptr<bool> m_exists = std::make_shared<bool>(true);
        public:
            template<typename T> void emit(T f) const {
                std::weak_ptr<bool> weak = m_exists;
                Glib::signal_idle().connect_once([f, weak]() {
                    auto strong = weak.lock();
                    //make sure our dispatcher still exists !
                    if (strong) {
                        f();
                    }
                });
            }
            dispatcher() {}
            dispatcher(const dispatcher&) = delete;
            void operator=(const dispatcher&) = delete;
    };
}
#endif
