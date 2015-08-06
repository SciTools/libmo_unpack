#include <stdio.h>
#include <stdlib.h>

#include <check.h>

#include "../src/rlencode.h"


// libmo_unpack needs this symbol defined ... *rolls eyes*
void MO_syslog(int value, char *message, const function *const caller)
{
}


START_TEST(test_compress)
{
    float fatvec[5] = {3, 6, 6, 6, 9};
    int fatlen = 5;
    float thinvec[10];
    int thinlen = 10;
    float bmdi = 6;
    function *parent = NULL;
    int rc;

    rc = runlen_encode(fatvec, fatlen, thinvec, &thinlen, bmdi, parent);

    ck_assert_int_eq(rc, 0);
    ck_assert_int_eq(thinlen, 4);
}
END_TEST


START_TEST(test_compress_result_larger)
{
    float fatvec[5] = {0, 2, 0, 4, 0};
    int fatlen = 5;
    float thinvec[5];
    int thinlen = 5;
    float bmdi = 0;
    function *parent = NULL;
    int rc;

    rc = runlen_encode(fatvec, fatlen, thinvec, &thinlen, bmdi, parent);

    ck_assert_int_eq(rc, 1);
}
END_TEST


START_TEST(test_decompress)
{
    float fatvec[5];
    int fatlen = 5;
    float thinvec[4] = {3, 6, 3, 9};
    int thinlen = 4;
    float bmdi = 6;
    function *parent = NULL;
    int rc;

    rc = runlen_decode(fatvec, fatlen, thinvec, thinlen, bmdi, parent);

    ck_assert_int_eq(rc, 0);
    ck_assert(fatvec[0] == 3);
    ck_assert(fatvec[1] == 6);
}
END_TEST


Suite *rle_suite()
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("RLE");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_compress);
    tcase_add_test(tc_core, test_compress_result_larger);
    tcase_add_test(tc_core, test_decompress);
    suite_add_tcase(s, tc_core);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = rle_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
