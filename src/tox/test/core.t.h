#include <cxxtest/TestSuite.h>

#include "../types.h"
#include "../core.h"
#include "../storage.h"
#include "../contact/manager.h"
#include "../contact/contact.h"
#include <giomm.h>
#include <thread>
#include <chrono>

class TestCore : public CxxTest::TestSuite
{
    private:
        std::shared_ptr<toxmm::core> core_a;
        std::shared_ptr<toxmm::core> core_b;
        std::shared_ptr<toxmm::contact> contact_a;
        std::shared_ptr<toxmm::contact> contact_b;

        const int MAX_WAIT_MIN = 2;
        const int UPDATE_DELAY_MS = 10;

        class MockStorage: public toxmm::storage {
            public:
                MockStorage() {}
                ~MockStorage() {}
                void set_prefix_key(const std::string&) override {}
                void save(const std::initializer_list<std::string>&, const std::vector<uint8_t>&) override {}
                void load(const std::initializer_list<std::string>&, std::vector<uint8_t>&) override {}
        };

        std::shared_ptr<MockStorage> mock_storage;

        Glib::RefPtr<Glib::MainLoop> main_loop;
        sigc::connection update_connection;

    public:
        ~TestCore() {
            update_connection.disconnect();
        }

        void test_to_hex() {
            TS_ASSERT_EQUALS(
                        std::string(toxmm::core::to_hex((const uint8_t[]){1, 2, 3, 0xFF, 0xFE}, 5)),
                        "010203FFFE");
        }

        void test_from_hex() {
            std::vector<uint8_t> v;
            v.push_back(1);
            v.push_back(2);
            v.push_back(3);
            v.push_back(0xFF);
            v.push_back(0xFE);
            TS_ASSERT_EQUALS(toxmm::core::from_hex("010203FFFE"), v);
        }

        void test_init() {
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

            mock_storage = std::make_shared<MockStorage>();
            core_a = toxmm::core::create("/tmp/gtox_core_a", mock_storage);
            core_b = toxmm::core::create("/tmp/gtox_core_b", mock_storage);

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
        }

        void test_wait_online() {
            int time = 0;
            while (std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                   (core_a->property_connection() == TOX_CONNECTION_NONE ||
                    core_b->property_connection() == TOX_CONNECTION_NONE)) {
                core_a->update();
                core_b->update();
                std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_DELAY_MS));
                time += UPDATE_DELAY_MS;
            }
            TS_ASSERT(core_a->property_connection() != TOX_CONNECTION_NONE);
            TS_ASSERT(core_b->property_connection() != TOX_CONNECTION_NONE);
            update_connection = Glib::signal_timeout().connect(sigc::bind_return([&]() {
                core_a->update();
                core_b->update();
            },true), UPDATE_DELAY_MS);
        }

        void test_add_contact() {
            toxmm::contactAddrPublic request_addr;
            std::string              request_message;
            bool                     request_finish = false;
            auto con = core_a->contact_manager()->signal_request().connect([&](toxmm::contactAddrPublic addr, Glib::ustring message) {
                request_addr    = addr;
                request_message = message;
                request_finish = true;
            });
            core_b->contact_manager()->add_contact(core_a->property_addr().get_value(), "Test Message");
            int time = 0;
            while(std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                  !request_finish) {
                Glib::MainContext::get_default()->iteration(true);
                time += UPDATE_DELAY_MS;
            }
            con->disconnect();
            TS_ASSERT_EQUALS(request_finish, true);
            TS_ASSERT_EQUALS(std::string(core_b->property_addr_public().get_value()), std::string(request_addr));
            TS_ASSERT_EQUALS(request_message, "Test Message");
        }

        void test_accept_contact() {
            core_a->contact_manager()->add_contact(core_b->property_addr_public());
            contact_a = core_b->contact_manager()->find(core_a->property_addr_public());
            contact_b = core_a->contact_manager()->find(core_b->property_addr_public());
            int time = 0;
            while (std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                   (contact_a->property_connection() == TOX_CONNECTION_NONE ||
                    contact_b->property_connection() == TOX_CONNECTION_NONE)) {
                Glib::MainContext::get_default()->iteration(true);
                time += UPDATE_DELAY_MS;
            }
            TS_ASSERT(contact_a->property_connection() != TOX_CONNECTION_NONE);
            TS_ASSERT(contact_b->property_connection() != TOX_CONNECTION_NONE);
        }

        void test_name_change() {
            auto new_name = "TEST_NAME_CHANGE";
            core_a->property_name() = new_name;
            int time = 0;
            while (std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                   (contact_a->property_name().get_value() != new_name)) {
                Glib::MainContext::get_default()->iteration(true);
                time += UPDATE_DELAY_MS;
            }
            TS_ASSERT_EQUALS(std::string(contact_a->property_name().get_value()), new_name);
        }

        void test_statusmessage_change() {
            auto new_name = "TEST_STATUS_CHANGE";
            core_a->property_status_message() = new_name;
            int time = 0;
            while (std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                   (contact_a->property_status_message().get_value() != new_name)) {
                Glib::MainContext::get_default()->iteration(true);
                time += UPDATE_DELAY_MS;
            }
            TS_ASSERT_EQUALS(std::string(contact_a->property_status_message().get_value()), new_name);
        }

        void test_status_change() {
            for (auto status : {TOX_USER_STATUS_BUSY, TOX_USER_STATUS_AWAY, TOX_USER_STATUS_NONE}) {
                core_a->property_status() = status;
                int time = 0;
                while (std::chrono::milliseconds(time) < std::chrono::minutes(MAX_WAIT_MIN) &&
                       (contact_a->property_status().get_value() != status)) {
                    Glib::MainContext::get_default()->iteration(true);
                    time += UPDATE_DELAY_MS;
                }
                TS_ASSERT_EQUALS(contact_a->property_status().get_value(), status);
            }
        }
};
