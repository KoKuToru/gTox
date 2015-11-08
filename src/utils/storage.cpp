#include "storage.h"
#include <glibmm.h>
#include <giomm.h>
using namespace utils;

storage::storage() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}

void storage::set_prefix_key(const std::string& prefix) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { prefix });
    m_prefix = prefix;
}

void storage::load(const std::initializer_list<std::string>& key, std::vector<uint8_t>& data) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { std::vector<std::string>(key), data });
   auto file_path = get_path_for_key(key);

    //open file
    auto file = Gio::File::create_for_path(file_path);
    //read file
    Glib::RefPtr<Gio::FileInputStream> stream;
    if (file->query_exists()) {
        try {
            stream = file->read();
        } catch (Gio::Error) {
            //couldn't read the file
            //lets ignore it for now..
        }
    }
    if (!stream) {
        //file not found
        data.clear();
        return;
    }
    data.resize(stream->query_info()->get_size());
    gsize size;
    stream->read_all((void*)data.data(), data.size(), size);
}

void storage::save(const std::initializer_list<std::string>& key, const std::vector<uint8_t>& data) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { std::vector<std::string>(key) });
    auto file_path = get_path_for_key(key);

    //open file
    auto file = Gio::File::create_for_path(file_path);
    auto parent = file->get_parent();
    //make sure folder exists
    if (parent) {
        if (!Glib::file_test(parent->get_path(), Glib::FILE_TEST_IS_DIR)) {
            parent->make_directory_with_parents();
        }
    }
    //replace file
    auto stream = file->replace();
    stream->truncate(0);
    stream->write_bytes(Glib::Bytes::create((gconstpointer)data.data(), data.size()));
    stream->close();
}

std::string storage::get_path_for_key(const std::initializer_list<std::string>& key) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), { std::vector<std::string>(key) });
    std::string file_path = Glib::build_filename(Glib::get_user_config_dir(),
                                                 "gtox");
    if (!m_prefix.empty()) {
        file_path = Glib::build_filename(file_path, m_prefix);
    }
    for(auto item : key) {
        file_path = Glib::build_filename(file_path, item);
    }
    file_path += ".bin";

    return file_path;
}

storage::~storage() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
}
