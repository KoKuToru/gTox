// MyTestSuite1.h
#include <cxxtest/TestSuite.h>

#include "../profile.h"
#include <giomm.h>
#include <iostream>

class TestProfile : public CxxTest::TestSuite
{
public:
    const char* test_path = "/tmp/gtox_test.bin";

    TestProfile() {
        Glib::init();
        Gio::init();
    }

    static void remove_file(const char* path) {
        auto file = Gio::File::create_for_path(path);

        try {
            file->remove();
        } catch (...) {
            //ignore
        }
    }

    static void init_file(const char* path) {
        remove_file(path);
        try {
            auto file = Gio::File::create_for_path(path);

            file->create_file()->write("test");
        } catch (Gio::Error err) {
            std::cerr << err.what();
            throw std::runtime_error("init_file failed");
        } catch (...) {
            throw std::runtime_error("init_file failed");
        }
    }

    void test_open_nonexisting_not_possible()
    {
        toxmm::profile p;

        //open the profile
        p.open("/who/the/hell/would/find/this");
        TS_ASSERT_EQUALS(p.can_read(),  false);
        TS_ASSERT_EQUALS(p.can_write(), false);

        //try to load data
        std::vector<uint8_t> state;
        TS_ASSERT_THROWS_ANYTHING(state = p.read());

        //try to save data
        TS_ASSERT_THROWS_ANYTHING(p.write(state));

        //try to move profile
        TS_ASSERT_THROWS_ANYTHING(p.move("/dummy"));
    }

    void test_open_nonexisting()
    {
        toxmm::profile p;

        remove_file(test_path);
        //non existsing files should be created
        p.open(test_path);
        TS_ASSERT_EQUALS(p.can_read(),  true);
        TS_ASSERT_EQUALS(p.can_write(), true);
    }

    void test_open_read_write() {
        init_file(test_path);

        //open the profile
        toxmm::profile p;
        p.open(test_path);
        TS_ASSERT_EQUALS(p.can_read(), true);
        TS_ASSERT_EQUALS(p.can_write(), true);

        //load data
        std::vector<uint8_t> state;
        TS_ASSERT_THROWS_NOTHING(state = p.read());
        {
            std::string test(state.begin(), state.end());
            TS_ASSERT_EQUALS(test, "test");
        }

        //change data, write & read
        state[0] = 'T';
        TS_ASSERT_THROWS_NOTHING(p.write(state));
        TS_ASSERT_THROWS_NOTHING(state = p.read());
        {
            std::string test(state.begin(), state.end());
            TS_ASSERT_EQUALS(test, "Test");
        }
    }
    void test_open_open() {
        init_file(test_path);

        //open the profile
        toxmm::profile p;
        p.open(test_path);
        TS_ASSERT_EQUALS(p.can_read(), true);
        TS_ASSERT_EQUALS(p.can_write(), true);

        //open the profile again
        toxmm::profile p2;
        p2.open(test_path);
        TS_ASSERT_EQUALS(p2.can_read(), true);
        TS_ASSERT_EQUALS(p2.can_write(), false);
    }
    void test_move() {
        init_file(test_path);

        //open the profile
        toxmm::profile p;
        p.open(test_path);
        TS_ASSERT_EQUALS(p.can_read(), true);
        TS_ASSERT_EQUALS(p.can_write(), true);

        //move the profile
        auto test_path_moved = std::string(test_path) + ".moved";
        p.move(test_path_moved);

        //check if old path is moved..
        TS_ASSERT_EQUALS(Glib::file_test(test_path, Glib::FILE_TEST_EXISTS),
                         false);

        //check if new path lock is working right..
        toxmm::profile p3;
        p3.open(test_path_moved);
        TS_ASSERT_EQUALS(p3.can_read(), true);
        TS_ASSERT_EQUALS(p3.can_write(), false);
    }
};
