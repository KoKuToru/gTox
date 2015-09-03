#ifndef H_GTOX_DEBUG
#define H_GTOX_DEBUG

#include <string>
#include <vector>
#include <array>
#include <mutex>
#include <algorithm>
#include <iostream>

#define DBG_LVL_1(tag) tag, 1, __FILE__, __LINE__, __PRETTY_FUNCTION__
#define DBG_LVL_2(tag) tag, 2, __FILE__, __LINE__, __PRETTY_FUNCTION__
#define DBG_LVL_3(tag) tag, 3, __FILE__, __LINE__, __PRETTY_FUNCTION__
#define DBG_LVL_4(tag) tag, 4, __FILE__, __LINE__, __PRETTY_FUNCTION__
#define DBG_LVL_5(tag) tag, 5, __FILE__, __LINE__, __PRETTY_FUNCTION__

namespace utils {
    namespace debug {
        class parameter {
            private:
                static std::vector<char> m_mem;
                static std::vector<std::pair<int, int>> m_mem_holes;
                static std::recursive_mutex m_mem_mtx;
                static int m_mem_offset;

                int m_offset_start;
                int m_offset_end;

                template<typename T>
                std::vector<parameter> create_parameter_vector(T begin, T end, size_t size) {
                    std::vector<parameter> p(size);
                    std::transform(begin, end, p.begin(), [](const auto& v) {
                        return parameter(v);
                    });
                    return p;
                }
            public:
                parameter();
                parameter(const parameter& o);
                parameter(const std::string& value, char quote = '"');
                parameter(char value)
                    : parameter(std::string(1, value), '\'') {}
                parameter(unsigned char value)
                    : parameter(std::to_string(value), 0) {}
                parameter(short value)
                    : parameter(std::to_string(value), 0) {}
                parameter(unsigned short value)
                    : parameter(std::to_string(value), 0) {}
                parameter(int value)
                    : parameter(std::to_string(value), 0) {}
                parameter(unsigned int value)
                    : parameter(std::to_string(value), 0) {}
                parameter(long long value)
                    : parameter(std::to_string(value), 0) {}
                parameter(unsigned long long value)
                    : parameter(std::to_string(value), 0) {}
                parameter(float value)
                    : parameter(std::to_string(value) + "f", 0) {}
                parameter(double value)
                    : parameter(std::to_string(value) + "d", 0) {}
                parameter(const std::vector<parameter>& value);

                void operator=(const parameter& o);

                template<typename T>
                parameter(const std::vector<T>& value)
                    : parameter(create_parameter_vector(value.begin(),
                                                        value.end(),
                                                        value.size())) {}

                template<typename T, int size>
                parameter(const std::array<T, size>& value)
                    : parameter(create_parameter_vector(value.begin(),
                                                        value.end(),
                                                        value.size())) {}

                template<typename T>
                parameter(T* value, size_t size)
                    : parameter(create_parameter_vector(value,
                                                        value + size,
                                                        size)) {}

                ~parameter();

                operator std::string() const {
                    return std::string(m_mem.begin() + m_offset_start,
                                       m_mem.begin() + m_offset_end);
                }
        };

        class scope_log {
            private:
                static thread_local int m_depth;

            public:
                scope_log(const char* tag,
                          int debug_level,
                          const char* file,
                          const int line,
                          const char* function,
                          std::initializer_list<parameter> params);
                ~scope_log();
        };

        namespace internal {
            template<typename T>
            class tracker;
        }

        template<typename T>
        class track_obj: virtual public T, virtual public internal::tracker<T> {
                using T::T;
                virtual ~track_obj() {}
        };

        namespace internal {
            class tracker_impl {
                public:
                    tracker_impl(std::string name);
                    virtual ~tracker_impl() {}
            };

            template<typename T>
            class tracker {
                private:
                    tracker_impl m_impl;
                public:
                    tracker(): m_impl("demo") {}
                    virtual ~tracker() {}
            };
        }
    };
}

#endif
