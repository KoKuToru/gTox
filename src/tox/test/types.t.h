#include <cxxtest/TestSuite.h>

#include "../types.h"

class TestTypes : public CxxTest::TestSuite
{
    private:
        template<typename T, typename nT>
        void basic_checks(nT a, nT b, nT c, std::string b_as_s) {
            //check constructor
            {
                T tmp(b);
                TS_ASSERT_EQUALS(tmp.get(), b);
            }
            //check constructor by string
            {
                T tmp(b_as_s);
                TS_ASSERT_EQUALS(tmp.get(), b);
            }
            //check operator=
            {
                T tmp_a(b);
                T tmp_b(c);
                TS_ASSERT_EQUALS(tmp_a.get(), b);
                TS_ASSERT_EQUALS(tmp_b.get(), c);
                tmp_a = tmp_b;
                TS_ASSERT_EQUALS(tmp_a.get(), c);
            }
            //check operators
            {
                TS_ASSERT_EQUALS(T(b) == T(b), true);
                TS_ASSERT_EQUALS(T(b) == T(c), false);

                TS_ASSERT_EQUALS(T(b) != T(c), true);
                TS_ASSERT_EQUALS(T(b) != T(b), false);

                TS_ASSERT_EQUALS(T(b) < T(c), true);
                TS_ASSERT_EQUALS(T(b) < T(b), false);

                TS_ASSERT_EQUALS(T(b) <= T(c), true);
                TS_ASSERT_EQUALS(T(b) <= T(b), true);
                TS_ASSERT_EQUALS(T(b) <= T(a), false);

                TS_ASSERT_EQUALS(T(b) > T(a), true);
                TS_ASSERT_EQUALS(T(b) > T(b), false);

                TS_ASSERT_EQUALS(T(b) >= T(a), true);
                TS_ASSERT_EQUALS(T(b) >= T(b), true);
                TS_ASSERT_EQUALS(T(b) >= T(c), false);
            }
            //check cast to string
            {
                TS_ASSERT_EQUALS(std::string(T(b)), b_as_s);
            }
            //check cast to type
            {
                TS_ASSERT_EQUALS(nT(T(b)), b);
            }
        }

    public:
        void test_contactNr() {
            basic_checks<toxmm::contactNr, uint32_t>(1, 2, 3, "2");
        }

        void test_receiptNr() {
            basic_checks<toxmm::receiptNr, uint32_t>(1, 2, 3, "2");
        }

        void test_fileNr() {
            basic_checks<toxmm::fileNr, uint32_t>(1, 2, 3, "2");
        }

        void test_contactAddrPublic() {
            using T = std::array<uint8_t, TOX_PUBLIC_KEY_SIZE>;
            T a = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T b = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T c = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            basic_checks<toxmm::contactAddrPublic, T>(
                        a,
                        b,
                        c,
                        "0100000000000000000000000000000000000000000000000000000000000000");
            TS_ASSERT_EQUALS(toxmm::contactAddrPublic(b),
                             toxmm::contactAddrPublic((const uint8_t[]){1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
        }

        void test_contactAddr() {
            using T = std::array<uint8_t, TOX_ADDRESS_SIZE>;
            T a = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 1, 2};
            T b = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 1, 2};
            T c = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 1, 2};
            basic_checks<toxmm::contactAddr, T>(
                        a,
                        b,
                        c,
                        "0100000000000000000000000000000000000000000000000000000000000000010203040102");
            TS_ASSERT_EQUALS(toxmm::contactAddr(b),
                             toxmm::contactAddr((const uint8_t[]){1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 1, 2}));
        }

        void fileId() {
            using T = std::array<uint8_t, TOX_FILE_ID_LENGTH>;
            T a = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T b = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T c = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            basic_checks<toxmm::fileId, T>(
                        a,
                        b,
                        c,
                        "0100000000000000000000000000000000000000000000000000000000000000");
            TS_ASSERT_EQUALS(toxmm::fileId(b),
                             toxmm::fileId((const uint8_t[]){1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
        }

        void hash() {
            using T = std::array<uint8_t, TOX_HASH_LENGTH>;
            T a = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T b = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T c = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            basic_checks<toxmm::hash, T>(
                        a,
                        b,
                        c,
                        "0100000000000000000000000000000000000000000000000000000000000000");
            TS_ASSERT_EQUALS(toxmm::hash(b),
                             toxmm::hash((const uint8_t[]){1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
        }
};
