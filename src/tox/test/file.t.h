#include <cxxtest/TestSuite.h>

#include "../types.h"
#include "../core.h"
#include "../storage.h"
#include "../contact/manager.h"
#include "../contact/contact.h"
#include "../contact/receipt.h"
#include "../contact/file/file.h"
#include "../contact/file/manager.h"
#include <giomm.h>

#include "global_fixture.t.h"

class TestFile : public CxxTest::TestSuite
{
    public:
        void test_file() {
            auto file = Gio::File::create_for_path("/tmp/send_test_filet.txt");
            auto stream = file->append_to();
            stream->truncate(0);
            stream->write("Dummy File");
            stream->close();

            int count = 0;
            std::string send_path;
            int send_size;
            TOX_FILE_KIND send_kind;
            bool send_complete;
            std::string recv_name;
            int recv_size;
            TOX_FILE_KIND recv_kind;
            bool recv_complete;
            sigc::connection con3, con4;
            sigc::connection con1 = gfix.contact_b->file_manager()->signal_recv_file().connect([&](std::shared_ptr<toxmm::file>& file) {
                recv_name = file->property_name().get_value();
                recv_size = file->property_size().get_value();
                recv_kind = file->property_kind().get_value();
                ++count;
                //start download
                con3 = file->property_complete().signal_changed().connect([&]() {
                    recv_complete = true;
                    ++count;
                });
                file->property_state() = TOX_FILE_CONTROL_RESUME;
            });
            sigc::connection con2 = gfix.contact_a->file_manager()->signal_send_file().connect([&](std::shared_ptr<toxmm::file>& file) {
                send_path = file->property_path().get_value();
                send_size = file->property_size().get_value();
                send_kind = file->property_kind().get_value();
                ++count;
                con4 = file->property_complete().signal_changed().connect([&]() {
                    send_complete = true;
                    ++count;
                });
            });
            gfix.contact_a->file_manager()->send_file(file->get_path());
            gfix.wait_while([&]() {
                return count < 4;
            });
            con1.disconnect();
            con2.disconnect();
            con3.disconnect();
            con4.disconnect();
            TS_ASSERT_EQUALS(count, 4);
            TS_ASSERT_EQUALS(send_path, file->get_path());
            TS_ASSERT_EQUALS(recv_name, file->get_basename());
            TS_ASSERT_EQUALS(send_size, file->query_info()->get_size());
            TS_ASSERT_EQUALS(recv_size, file->query_info()->get_size());
            TS_ASSERT_EQUALS(send_kind, TOX_FILE_KIND_DATA);
            TS_ASSERT_EQUALS(recv_kind, TOX_FILE_KIND_DATA);
            TS_ASSERT_EQUALS(send_complete, true);
            TS_ASSERT_EQUALS(recv_complete, true);
            //check if same content
            auto rstream = file->read();
            char buffer[1024];
            gsize size;
            rstream->read_all((void*)buffer, sizeof(buffer), size);
            TS_ASSERT_EQUALS("Dummy File", std::string(buffer, size));
        }

        void test_file_abort() {
            auto file = Gio::File::create_for_path("/tmp/send_test_filet.txt");
            int count = 0;
            bool recv_complete;
            bool send_complete;
            std::string recv_path;
            sigc::connection con3, con4;
            sigc::connection con1 = gfix.contact_b->file_manager()->signal_recv_file().connect([&](std::shared_ptr<toxmm::file>& file) {
                ++count;
                recv_path = file->property_path().get_value();
                //abort download
                con3 = file->property_complete().signal_changed().connect([&]() {
                    recv_complete = true;
                    ++count;
                });
                file->property_state() = TOX_FILE_CONTROL_RESUME;
            });
            sigc::connection con2 = gfix.contact_a->file_manager()->signal_send_file().connect([&](std::shared_ptr<toxmm::file>& file) {
                ++count;
                con4 = file->property_complete().signal_changed().connect([&]() {
                    send_complete = true;
                    ++count;
                });
            });
            gfix.contact_a->file_manager()->send_file(file->get_path());
            gfix.wait_while([&]() {
                return count < 4;
            });
            con1.disconnect();
            con2.disconnect();
            con3.disconnect();
            con4.disconnect();
            TS_ASSERT_EQUALS(count, 4);
            TS_ASSERT_EQUALS(send_complete, true);
            TS_ASSERT_EQUALS(recv_complete, true);
            TS_ASSERT_EQUALS(file->query_exists(), true);
            TS_ASSERT_EQUALS(Gio::File::create_for_path(recv_path)->query_exists(), true);
        }
};
