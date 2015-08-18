#include <cxxtest/TestSuite.h>

#include "../types.h"
#include "../core.h"
#include "../storage.h"
#include "../contact/manager.h"
#include "../contact/contact.h"
#include <giomm.h>
#include <thread>
#include <chrono>

#include "global_fixture.t.h"

class TestCore : public CxxTest::TestSuite
{
    public:
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

        void test_wait_online() {
            gfix.wait_while([]() {
                return gfix.core_a->property_connection() == TOX_CONNECTION_NONE ||
                       gfix.core_b->property_connection() == TOX_CONNECTION_NONE;
            });
            TS_ASSERT_DIFFERS(gfix.core_a->property_connection(), TOX_CONNECTION_NONE);
            TS_ASSERT_DIFFERS(gfix.core_b->property_connection(), TOX_CONNECTION_NONE);
        }

        void test_add_contact() {
            toxmm::contactAddrPublic request_addr;
            std::string              request_message;
            bool                     request_finish = false;
            auto con = gfix.core_a->contact_manager()->signal_request().connect([&](toxmm::contactAddrPublic addr, Glib::ustring message) {
                request_addr    = addr;
                request_message = message;
                request_finish = true;
            });
            gfix.core_b->contact_manager()->add_contact(gfix.core_a->property_addr().get_value(), "Test Message");
            gfix.wait_while([&]() {
                return !request_finish;
            });
            con->disconnect();
            TS_ASSERT_EQUALS(request_finish, true);
            TS_ASSERT_EQUALS(std::string(gfix.core_b->property_addr_public().get_value()), std::string(request_addr));
            TS_ASSERT_EQUALS(request_message, "Test Message");
        }

        void test_accept_contact() {
            gfix.core_a->contact_manager()->add_contact(gfix.core_b->property_addr_public());
            gfix.contact_a = gfix.core_b->contact_manager()->find(gfix.core_a->property_addr_public());
            gfix.contact_b = gfix.core_a->contact_manager()->find(gfix.core_b->property_addr_public());
            gfix.wait_while([]() {
                return gfix.contact_a->property_connection() == TOX_CONNECTION_NONE ||
                       gfix.contact_b->property_connection() == TOX_CONNECTION_NONE;
            });
            TS_ASSERT_DIFFERS(gfix.contact_a->property_connection().get_value(), TOX_CONNECTION_NONE);
            TS_ASSERT_DIFFERS(gfix.contact_b->property_connection().get_value(), TOX_CONNECTION_NONE);
        }

        void test_name_change() {
            auto new_name = "TEST_NAME_CHANGE";
            gfix.core_a->property_name() = new_name;
            gfix.wait_while([&]() {
                return gfix.contact_a->property_name().get_value() != new_name;
            });
            TS_ASSERT_EQUALS(std::string(gfix.contact_a->property_name().get_value()), new_name);
        }

        void test_statusmessage_change() {
            auto new_name = "TEST_STATUS_CHANGE";
            gfix.core_a->property_status_message() = new_name;
            gfix.wait_while([&]() {
                return gfix.contact_a->property_status_message().get_value() != new_name;
            });
            TS_ASSERT_EQUALS(std::string(gfix.contact_a->property_status_message().get_value()), new_name);
        }

        void test_status_change() {
            for (auto status : {TOX_USER_STATUS_BUSY, TOX_USER_STATUS_AWAY, TOX_USER_STATUS_NONE}) {
                gfix.core_a->property_status() = status;
                gfix.wait_while([&]() {
                    return gfix.contact_a->property_status() != status;
                });
                TS_ASSERT_EQUALS(gfix.contact_a->property_status().get_value(), status);
            }
        }
};
