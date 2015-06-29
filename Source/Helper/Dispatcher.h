#ifndef DISPATCHER_H
#define DISPATCHER_H

class Dispatcher {
    private:
        std::shared_ptr<bool> m_exists = std::make_shared<bool>(true);
    public:
        template<typename T> void emit(T f) {
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
