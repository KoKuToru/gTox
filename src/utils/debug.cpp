#include "debug.h"
#include "assert.h"
#include <algorithm>
#include <ctype.h>
#include <iostream>
#include <thread>
#include <iomanip>
#include <unistd.h>
#include <glibmm.h>
#include <cxxabi.h>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

using namespace utils::debug;

std::vector<char> parameter::m_mem;
std::vector<std::pair<int, int>> parameter::m_mem_holes;
int parameter::m_mem_offset;
std::recursive_mutex parameter::m_mem_mtx;

parameter::parameter() {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    m_offset_start = m_mem_offset;
    m_offset_end = m_mem_offset;
}

parameter::parameter(const parameter& o) {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    m_offset_start = m_mem_offset;
    m_mem.insert(m_mem.begin() + m_mem_offset,
                 m_mem.begin() + o.m_offset_start,
                 m_mem.begin() + o.m_offset_end);
    m_mem_offset += o.m_offset_end - o.m_offset_start;
    m_offset_end = m_mem_offset;
}

parameter::parameter(const std::string& value, char quote)  {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    m_offset_start = m_mem_offset;
     //resize for the worst case scenario
    m_mem.reserve(m_mem.size() + value.size() * 2 + 2);
    if (quote) {
        m_mem.push_back(quote);
        m_mem_offset += 1;
    }
    for (char c: value) {
        switch (c) {
            case '\a':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'a';
                break;
            case '\b':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'b';
                break;
            case '\f':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'f';
                break;
            case '\n':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'n';
                break;
            case '\r':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'r';
                break;
            case '\t':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 't';
                break;
            case '\v':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = 'v';
                break;
            case '\\':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = '\\';
                break;
            case '\'':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = '\'';
                break;
            case '\"':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = '"';
                break;
            case '\?':
                m_mem.push_back('\\');
                m_mem_offset += 1;
                c = '?';
                break;
            default:
                if (!isprint(c)) {
                    m_mem.push_back('\\');
                    m_mem.push_back('x');
                    int ic = int((unsigned char)c);
                    m_mem.push_back("012345789ABCDEF"[(ic & 0xF0) >> 4]);
                    m_mem.push_back("012345789ABCDEF"[(ic & 0x0F)]);
                    m_mem_offset += 4;
                    continue;
                }
                break;
        }
        m_mem.push_back(c);
        m_mem_offset += 1;
    }
    if (quote) {
        m_mem.push_back(quote);
        m_mem_offset += 1;
    }
    m_offset_end = m_mem_offset;
}

parameter::parameter(const std::vector<parameter>& value) {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    m_offset_start = m_mem_offset;
    m_mem.push_back('{');
    m_mem_offset += 1;
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            m_mem.push_back(',');
            m_mem.push_back(' ');
            m_mem_offset += 2;
        }
        std::string str = value[i];
        m_mem.insert(m_mem.begin() + m_mem_offset, str.begin(), str.end());
        m_mem_offset += str.size();
    }
    m_mem.push_back('}');
    m_mem_offset += 1;
    m_offset_end = m_mem_offset;
}

parameter::~parameter() {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    //this find_if could be opimized
    //is it even guaranteed to go from index 0 to n ?
    if (m_offset_start != m_offset_end) {
        auto iter = std::find_if(m_mem_holes.begin(), m_mem_holes.end(), [this](const std::pair<int, int>& o) {
            return o.second >= m_offset_start;
        });
        if (iter != m_mem_holes.end() && iter->first < m_offset_start) {
            ++iter;
        }
        m_mem_holes.insert(iter, {m_offset_start, m_offset_end});
    }
    while (!m_mem_holes.empty() && m_mem_offset == m_mem_holes.back().second) {
        m_mem_offset = m_mem_holes.back().first;
        m_mem_holes.pop_back();
    }
    m_mem.resize(m_mem_offset);
}

void parameter::operator=(const parameter& o) {
    std::lock_guard<std::recursive_mutex> lg(m_mem_mtx);
    this->~parameter();
    m_offset_start = m_mem_offset;
    m_mem.insert(m_mem.begin() + m_mem_offset,
                 m_mem.begin() + o.m_offset_start,
                 m_mem.begin() + o.m_offset_end);
    m_mem_offset += o.m_offset_end - o.m_offset_start;
    m_offset_end = m_mem_offset;
}

thread_local int scope_log::m_depth = 0;

scope_log::scope_log(const char* tag,
                     int debug_level,
                     const char* file,
                     const int line,
                     const char* function,
                     std::initializer_list<parameter> params) {
    static auto is_terminal = isatty(2) == 1;
    static auto env_debug_level = std::stoi("0" + Glib::getenv("GTOX_DBG_LVL"));
    if (debug_level > env_debug_level) {
        return;
    }

    m_depth += 1;

    std::clog << std::setfill('-')
              << std::setw(m_depth)
              << ">"
              << std::setfill(' ');

    std::clog << "[";
    if (is_terminal) {
        std::clog << KMAG;
    }
    std::clog << std::setw(sizeof(void*)*2)
              << std::setfill('0')
              << std::right
              << std::hex
              << std::this_thread::get_id()
              << std::dec
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog << "] ";
    std::clog << tag << ": ";

    if (is_terminal) {
        std::clog << KRED;
    }
    std::clog << file
              << "@"
              << line
              << " ";

    if (is_terminal) {
        std::clog << KYEL;
    }
    std::clog << function;
    if (params.begin() == params.end()) {
        if (is_terminal) {
            std::clog << KNRM;
        }
        std::clog << std::endl;
        return;
    }
    std::clog << '\n';

    std::clog << std::setfill(' ')
              << std::setw(m_depth)
              << " "
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KCYN;
    }
    std::clog << " (";
    bool first = true;
    for (const auto& v: params) {
        if (first) {
            first = false;
        } else {
            std::clog << ", ";
        }
        std::clog << (std::string)v;
    }
    std::clog << ")";
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog << std::endl;
}

scope_log::~scope_log() {
    m_depth -= 1;
}

internal::tracker_impl::tracker_impl(std::type_index type, const char* name)
    : m_type(type),
      m_map([this]() {
            static decltype (m_map) static_map = decltype (m_map)(new map_type());
            return static_map;
      }()),
      m_mtx([this]() {
            static decltype (m_mtx) static_mtx = decltype (m_mtx)(new mtx_type());
            return static_mtx;
      }()) {
    static auto env_debug = std::stoi("0" + Glib::getenv("GTOX_DBG_TRACKOBJ")) == 1;
    if (!env_debug) {
        return;
    }
    static auto is_terminal = isatty(2) == 1;

    if (name == nullptr) {
        return;
    }

    std::lock_guard<mtx_type> lg(*m_mtx);

    auto iter = m_map->find(type);
    if (iter != m_map->end()) {
        iter->second.second += 1;
    } else {
        int status;
        char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        if (status == 0) {
            name = demangled;
        }
        m_map->insert({type, {name, 1}});
        free(demangled);
        iter = m_map->find(type);
    }
    std::clog << "+";
    std::clog << "[";
    if (is_terminal) {
        std::clog << KMAG;
    }
    std::clog << std::setw(sizeof(void*)*2)
              << std::setfill('0')
              << std::right
              << std::hex
              << std::this_thread::get_id()
              << std::dec
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog << "] ";
    if (is_terminal) {
        std::clog << KYEL;
    }
    std::clog << std::setw(sizeof(void*)*2)
              << std::setfill('0')
              << std::right
              << std::hex
              << (uint64_t)(void*)this
              << std::dec
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KNRM;
    }
    if (is_terminal) {
        std::clog << KCYN;
    }
    std::clog << " " << iter->second.first;
    if (is_terminal) {
        std::clog << KRED;
    }
    std::clog << " " << iter->second.second;
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog <<  " objs" << std::endl;
}

internal::tracker_impl::~tracker_impl() {
    static auto env_debug = std::stoi("0" + Glib::getenv("GTOX_DBG_TRACKOBJ")) == 1;
    if (!env_debug) {
        return;
    }
    static auto is_terminal = isatty(2) == 1;

    std::lock_guard<mtx_type> log(*m_mtx);

    auto iter = m_map->find(m_type);

    if (iter == m_map->end()) {
        return;
    }
    iter->second.second -= 1;

    std::clog << "-";
    std::clog << "[";
    if (is_terminal) {
        std::clog << KMAG;
    }
    std::clog << std::setw(sizeof(void*)*2)
              << std::setfill('0')
              << std::right
              << std::hex
              << std::this_thread::get_id()
              << std::dec
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog << "] ";
    if (is_terminal) {
        std::clog << KYEL;
    }
    std::clog << std::setw(sizeof(void*)*2)
              << std::setfill('0')
              << std::right
              << std::hex
              << (uint64_t)(void*)this
              << std::dec
              << std::setfill(' ');
    if (is_terminal) {
        std::clog << KNRM;
    }
    if (is_terminal) {
        std::clog << KCYN;
    }
    std::clog << " " << iter->second.first;
    if (is_terminal) {
        std::clog << KRED;
    }
    std::clog << " " << iter->second.second;
    if (is_terminal) {
        std::clog << KNRM;
    }
    std::clog <<  " objs" << std::endl;
}

void internal::tracker_impl::print_leak() {
    static auto env_debug = std::stoi("0" + Glib::getenv("GTOX_DBG_TRACKOBJ")) == 1;
    if (!env_debug) {
        return;
    }
    static auto is_terminal = isatty(fileno(stderr)) == 1;
    //Display leaked memory
    int count = 0;
    for (auto& item: *m_map) {
        if (item.second.second == 0) {
            continue;
        }
        count += item.second.second;
    }
    if (count == 0) {
        return;
    }
    std::cerr << "\nLeaked memory: \n";
    for (auto& item: *m_map) {
        if (item.second.second != 0) {
            if (is_terminal) {
                std::clog << KCYN;
            }
            std::cerr << " " << item.second.first;
            if (is_terminal) {
                std::clog << KRED;
            }
            std::cerr << " " << item.second.second;
            if (is_terminal) {
                std::clog << KNRM;
            }
            std::cerr <<  " objs\n";
        }
    }
    if (is_terminal) {
        std::cerr << KRED;
    }
    std::cerr << count << " objs leaked !\n";
    if (is_terminal) {
        std::cerr << KNRM;
    }
    std::cerr << std::endl;
}

__attribute__((destructor))
static void destroy_app() {
    internal::tracker_impl dummy(std::type_index(typeid (void)), nullptr);
    dummy.print_leak();
}
