#include <cxxtest/TestSuite.h>

#include "../types.h"
#include "../core.h"
#include "../storage.h"
#include "../contact/manager.h"
#include "../contact/contact.h"
#include "../contact/receipt.h"
#include <giomm.h>
#include <thread>
#include <chrono>

#include "global_fixture.t.h"

//MUST BE AFTER TestCore !
class TestContact : public CxxTest::TestSuite
{
    public:
        void test_message() {
            Glib::ustring msg = "Test message";
            Glib::ustring recv_message;
            Glib::ustring send_message;
            bool receipt_okay = false;
            int count = 0;
            sigc::connection con1 = gfix.contact_b->signal_recv_message().connect([&](Glib::ustring s) {
                recv_message = s;
                ++count;
            });
            sigc::connection con3;
            sigc::connection con2 = gfix.contact_a->signal_send_message().connect([&](Glib::ustring s, std::shared_ptr<toxmm::receipt> r) {
                send_message = s;
                con3 = r->signal_okay().connect([&]() {
                    receipt_okay = true;
                    ++count;
                });
                ++count;
            });
            gfix.contact_a->send_message(msg);
            gfix.wait_while([&]() {
                return count < 3;
            });
            con1.disconnect();
            con2.disconnect();
            con3.disconnect();

            TS_ASSERT_EQUALS(std::string(recv_message), std::string(msg));
            TS_ASSERT_EQUALS(std::string(send_message), std::string(msg));
            TS_ASSERT_EQUALS(receipt_okay, true);
        }

        void test_action() {
            Glib::ustring msg = "Test message";
            Glib::ustring recv_message;
            Glib::ustring send_message;
            bool receipt_okay = false;
            int count = 0;
            sigc::connection con1 = gfix.contact_b->signal_recv_action().connect([&](Glib::ustring s) {
                recv_message = s;
                ++count;
            });
            sigc::connection con3;
            sigc::connection con2 = gfix.contact_a->signal_send_action().connect([&](Glib::ustring s, std::shared_ptr<toxmm::receipt> r) {
                send_message = s;
                con3 = r->signal_okay().connect([&]() {
                    receipt_okay = true;
                    ++count;
                });
                ++count;
            });
            gfix.contact_a->send_action(msg);
            gfix.wait_while([&]() {
                return count < 3;
            });
            con1.disconnect();
            con2.disconnect();
            con3.disconnect();

            TS_ASSERT_EQUALS(std::string(recv_message), std::string(msg));
            TS_ASSERT_EQUALS(std::string(send_message), std::string(msg));
            TS_ASSERT_EQUALS(receipt_okay, true);
        }
};
