
#include <stdio.h>
#include <check.h>
#include <fcntl.h>

#include "x2r.h"
#include "x2r_log.h"
#include "x2r_ws.h"


START_TEST (test_convert)
{
    FILE *in;
    fail_if(!(in = fopen("./data/station.xml", "r")));
    FILE *out;
    fail_if(!(out = tmpfile()));
    x2r_log *log;
    fail_if(x2r_alloc_log(X2R_DEBUG, stderr, &log));
    x2r_fdsn_station_xml *root = NULL;
    fail_if(x2r_station_service_load(log, in, &root));
    fail_if(x2r_resp_util_write(log, out, root));
    rewind(out);
    FILE *expect;
    fail_if(!(expect = fopen("./data/response", "r")));
    char a[1000], b[1000];
    int line = 0, aok = 1, bok = 1;
    while (aok && bok) {
        aok = (fgets(a, 1000, expect) != NULL);
        bok = (fgets(b, 1000, out) != NULL);
        line++;
        fail_if(aok != bok, "Different file lengths after line %d", line);
        if (aok) fail_if(strcmp(a, b), "Line %d differs (expected, generated):\n%s%s", line, a, b);
    }
    fail_if(x2r_free_fdsn_station_xml(root, X2R_OK));
    fail_if(x2r_free_log(log, X2R_OK));
}
END_TEST


int main (void) {
    int number_failed;
    Suite *s = suite_create("suite");
    TCase *tc = tcase_create ("case");
    tcase_add_test(tc, test_convert);
    suite_add_tcase(s, tc);
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
