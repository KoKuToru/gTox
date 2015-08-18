#ifndef GLOBAL_FIXTURE
#define GLOBAL_FIXTURE

#include <cxxtest/TestSuite.h>
#include <cxxtest/GlobalFixture.h>
#include "../types.h"
#include "../core.h"
#include "../storage.h"
#include "../contact/manager.h"
#include "../contact/contact.h"
#include <chrono>

class GlobalFixture : public CxxTest::GlobalFixture
{
public:
    std::shared_ptr<toxmm::core> core_a;
    std::shared_ptr<toxmm::core> core_b;
    std::shared_ptr<toxmm::contact> contact_a;
    std::shared_ptr<toxmm::contact> contact_b;

    Glib::RefPtr<Glib::MainLoop> main_loop;
    sigc::connection update_connection;

    const int MAX_WAIT_SEC_OFFLINE = 600;
    const int MAX_WAIT_SEC_ONLINE  = 15;
    const int UPDATE_DELAY_MS = 10;

    class MockStorage: public toxmm::storage {
        private:
            std::map<std::string, std::vector<uint8_t>> m_mem;
            std::string m_prefix;
        public:
            MockStorage() {}
            ~MockStorage() {}
            void set_prefix_key(const std::string& prefix) override {
                m_prefix = prefix;
            }
            void save(const std::initializer_list<std::string>& key, const std::vector<uint8_t>& data) override {
                std::string str_key = m_prefix;
                for(auto v : key) {
                    str_key += "_" + v;
                }
                auto iter = m_mem.find(str_key);
                if (iter != m_mem.end()) {
                    TS_TRACE("MOCKSTORAGE STORE " + str_key + " OVERWRITE " + std::to_string(data.size()) + " BYTES");
                    iter->second = data;
                    return;
                }
                TS_TRACE("MOCKSTORAGE STORE " + str_key + " WRITE " + std::to_string(data.size()) + " BYTES");
                m_mem.insert({str_key, data});
            }
            void load(const std::initializer_list<std::string>& key, std::vector<uint8_t>& data) override {
                std::string str_key = m_prefix;
                for(auto v : key) {
                    str_key += "_" + v;
                }
                auto iter = m_mem.find(str_key);
                if (iter != m_mem.end()) {
                    TS_TRACE("MOCKSTORAGE LOAD " + str_key + " RETURN " + std::to_string(iter->second.size()) + " BYTES");
                    data = iter->second;
                    return;
                }
                TS_TRACE("MOCKSTORAGE LOAD " + str_key + " RETURN EMPTY");
                data.clear();
            }
    };

    std::shared_ptr<MockStorage> mock_storage_a;
    std::shared_ptr<MockStorage> mock_storage_b;

    GlobalFixture() {
        auto init = [](std::string path, std::string name) {
            toxmm::contactAddrPublic addr;
            auto state = toxmm::core::create_state(name,
                                                   "testing..",
                                                   addr);
            auto file = Gio::File::create_for_path(path);
            try {
                file->remove();
            } catch (...) {
                //ignore
            }
            auto stream = file->create_file();
            auto bytes = Glib::Bytes::create((void*)state.data(), state.size());
            stream->write_bytes(bytes);
            stream->close();
        };

        init("/tmp/gtox_core_a", "Test A");
        init("/tmp/gtox_core_b", "Test B");

        mock_storage_a = std::make_shared<MockStorage>();
        mock_storage_b = std::make_shared<MockStorage>();

        reset();
    }

    void reset() {
        core_a.reset();
        core_b.reset();

        core_a = toxmm::core::create("/tmp/gtox_core_a", mock_storage_a);
        core_b = toxmm::core::create("/tmp/gtox_core_b", mock_storage_b);

        core_a->property_download_path() = "/tmp/";
        core_b->property_download_path() = "/tmp/";

        core_a->property_avatar_path() = "/tmp/avatar_a/";
        core_b->property_avatar_path() = "/tmp/avatar_b/";

        //speed up testing ?
        auto tox_a = core_a->toxcore();
        auto tox_b = core_b->toxcore();

        toxmm::contactAddrPublic  dht_a, dht_b;
        tox_self_get_dht_id(tox_a, dht_a);
        tox_self_get_dht_id(tox_a, dht_b);
        TOX_ERR_GET_PORT error;
        auto port_a = tox_self_get_udp_port(tox_a, &error);
        TS_ASSERT_EQUALS(error, TOX_ERR_GET_PORT_OK);
        auto port_b = tox_self_get_udp_port(tox_b, &error);
        TS_ASSERT_EQUALS(error, TOX_ERR_GET_PORT_OK);
        TOX_ERR_BOOTSTRAP bootstrap_error;
        tox_bootstrap(tox_a, "127.0.0.1", port_b, dht_b, &bootstrap_error);
        TS_ASSERT_EQUALS(bootstrap_error, TOX_ERR_BOOTSTRAP_OK);
        tox_bootstrap(tox_b, "127.0.0.1", port_a, dht_a, &bootstrap_error);
        TS_ASSERT_EQUALS(bootstrap_error, TOX_ERR_BOOTSTRAP_OK);

        update_connection = Glib::signal_timeout().connect(sigc::bind_return([this]() {
            core_a->update();
            core_b->update();
        },true), UPDATE_DELAY_MS);
        contact_a = core_b->contact_manager()->find(core_a->property_addr_public());
        contact_b = core_a->contact_manager()->find(core_b->property_addr_public());
    }

    void wait_for_online() {
        wait_while([this]() {
            return core_a->property_connection() == TOX_CONNECTION_NONE ||
                   core_b->property_connection() == TOX_CONNECTION_NONE;
        });
        TS_ASSERT_DIFFERS(core_a->property_connection(), TOX_CONNECTION_NONE);
        TS_ASSERT_DIFFERS(core_b->property_connection(), TOX_CONNECTION_NONE);
    }

    void wait_for_contact() {
        if (!contact_a || !contact_b) {
            auto need_to_wait = true;
            auto con = core_a->contact_manager()->signal_request().connect([&](toxmm::contactAddrPublic, Glib::ustring) {
                core_a->contact_manager()->add_contact(core_b->property_addr_public());
                need_to_wait = false;
            });
            core_b->contact_manager()->add_contact(core_a->property_addr().get_value(), "Test Message");
            wait_while([&]() {
                return need_to_wait;
            });
            con->disconnect();
            contact_a = core_b->contact_manager()->find(core_a->property_addr_public());
            contact_b = core_a->contact_manager()->find(core_b->property_addr_public());
        }
        TS_ASSERT(contact_a != nullptr);
        TS_ASSERT(contact_b != nullptr);
        wait_while([this]() {
            return contact_a->property_connection() == TOX_CONNECTION_NONE ||
                   contact_b->property_connection() == TOX_CONNECTION_NONE;
        });
        TS_ASSERT_DIFFERS(contact_a->property_connection().get_value(), TOX_CONNECTION_NONE);
        TS_ASSERT_DIFFERS(contact_b->property_connection().get_value(), TOX_CONNECTION_NONE);
    }

    ~GlobalFixture() {
        update_connection.disconnect();
    }

    template<typename T>
    void wait_while(T f) {
        auto start = std::chrono::system_clock::now();
        int last_15_sec = 0;
        TS_TRACE("WAITING START");
        while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start)
               < std::chrono::seconds(
                       ((contact_a && contact_a->property_connection() != TOX_CONNECTION_NONE) &&
                        (contact_b && contact_b->property_connection() != TOX_CONNECTION_NONE))
                   ?MAX_WAIT_SEC_ONLINE
                   :MAX_WAIT_SEC_OFFLINE) &&
               f()) {
            Glib::MainContext::get_default()->iteration(true);
            auto new_15_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() / 15 * 15;
            if (new_15_sec != last_15_sec) {
                last_15_sec = new_15_sec;
                TS_TRACE("WAITED FOR " + std::to_string(last_15_sec) + "SEC");
            }
        }
        TS_TRACE("WAITING END");
        if (f()) {
            TS_FAIL("TIMEOUT");
        }
    }
};

static GlobalFixture gfix;
#endif
