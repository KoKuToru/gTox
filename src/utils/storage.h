#ifndef STORAGE_H
#define STORAGE_H
#include <string>
#include "tox/storage.h"

namespace utils {
    class storage : public toxmm::storage {
            friend class main;
        private:
            std::string m_prefix;

        public:
            storage();

        protected:
            void set_prefix_key(const std::string& prefix) override;
            void load(const std::initializer_list<std::string>& key, std::vector<uint8_t>& data) override;
            void save(const std::initializer_list<std::string>& key, const std::vector<uint8_t>& data) override;

            std::string get_path_for_key(const std::initializer_list<std::string>& key);
    };
}

#endif
