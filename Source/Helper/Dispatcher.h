#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <memory>
#include <glibmm.h>

/**
 * @brief The Dispatcher, executes given function on gtk main loop.
 *
 * Makes sure the class-instance with the dispatcher still exists.
 * Simplifies multithreading programming a lot.
 */
class Dispatcher {
    public:
        /**
         * Makes a weak reference to a Dispatcher.
         * Needed when you don't join threads in the class-instance with the dispatcher.
         */
        class Ref {
            private:
                std::weak_ptr<bool> m_exists;
            public:
                Ref(const Dispatcher& o): m_exists(o.m_exists) {}
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

        friend class Dispatcher::Ref;
    private:
        std::shared_ptr<bool> m_exists = std::make_shared<bool>(true);
    public:
        template<typename T> void emit(T f) const {
            std::weak_ptr<bool> weak = m_exists;
            Glib::signal_idle().connect_once([f, weak]() {
                auto strong = weak.lock();
                if (strong) {
                    f();
                }
            });
        }
        Dispatcher() {}
        Dispatcher(const Dispatcher&) = delete;
        void operator=(const Dispatcher&) = delete;
};

#endif
