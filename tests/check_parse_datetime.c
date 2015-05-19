
#include <stdio.h>
#include <check.h>
#include <fcntl.h>

#include "x2r.h"
#include "x2r_log.h"
#include "x2r_xml.h"


START_TEST (test_parse_datetime)
{
    x2r_log *log;
    fail_if(x2r_alloc_log(X2R_DEBUG, stderr, &log));
    time_t epoch, target = 946684800;
    fail_if(x2r_parse_iso_datetime(log, "2000-01-01T00:00:00", &epoch));
    fail_if(epoch != target, "Bad epoch: %ld (error %d)", epoch, epoch - target);
    fail_if(x2r_free_log(log, X2R_OK));
}
END_TEST


int main (void) {
    int number_failed;
    Suite *s = suite_create("suite");
    TCase *tc = tcase_create ("case");
    tcase_add_test(tc, test_parse_datetime);
    suite_add_tcase(s, tc);
    SRunner *sr = srunner_create(s);
    srunner_set_xml(sr, "check-parse_datetime.xml");
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
