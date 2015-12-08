#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <memory>
#include <glibmm.h>

#ifndef SIGC_CPP11_HACK
#define SIGC_CPP11_HACK
namespace sigc {
    SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}
#endif

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
                    template<typename T> sigc::connection emit(T f) const {
                        std::weak_ptr<bool> weak = m_exists;
                        return Glib::signal_idle().connect([f, weak]() {
                            auto strong = weak.lock();
                            if (strong) {
                                f();
                            }
                            return false;
                        });
                    }
            };

            friend class dispatcher::ref;
        private:
            std::shared_ptr<bool> m_exists = std::make_shared<bool>(true);
        public:
            template<typename T> sigc::connection emit(T f) const {
                std::weak_ptr<bool> weak = m_exists;
                return Glib::signal_idle().connect([f, weak]() {
                    auto strong = weak.lock();
                    //make sure our dispatcher still exists !
                    if (strong) {
                        f();
                    }
                    return false;
                });
            }
            dispatcher() {}
            dispatcher(const dispatcher&) = delete;
            void operator=(const dispatcher&) = delete;
    };
}
#endif
