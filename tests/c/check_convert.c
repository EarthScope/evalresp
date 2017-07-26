
#include <check.h>
#include <fcntl.h>
#include <stdio.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "evalresp_log/log.h"

void
run_test (char *station, char *response)
{
  FILE *in;
  fail_if (!(in = fopen (station, "r")));
  FILE *out;
  fail_if (!(out = tmpfile ()));
  evalresp_logger *log = NULL;
  /*XXX x2r_log *log;
    fail_if(x2r_alloc_log(X2R_DEBUG, stderr, &log)); */
  x2r_fdsn_station_xml *root = NULL;
  fail_if (x2r_station_service_load (log, in, &root));
  fail_if (x2r_resp_util_write (log, out, root));
  rewind (out);
  FILE *expect;
  fail_if (!(expect = fopen (response, "r")));
  char a[1000], b[1000];
  int line = 0, aok = 1, bok = 1;
  while (aok && bok)
  {
    aok = (fgets (a, 1000, expect) != NULL);
    bok = (fgets (b, 1000, out) != NULL);
    line++;
    fail_if (aok != bok, "Different file lengths after line %d", line);
    if (aok)
      fail_if (strcmp (a, b), "%s/%s: line %d differs (expected, generated):\n%s%s",
               response, station, line, a, b);
  }
  fail_if (x2r_free_fdsn_station_xml (root, X2R_OK));
  /*XXX fail_if(x2r_free_log(log, X2R_OK)); */
}

START_TEST (test_convert_1)
{
  run_test ("data/station-1.xml", "data/response-1");
}
END_TEST

START_TEST (test_convert_2)
{
  run_test ("data/station-2.xml", "data/response-2");
}
END_TEST

START_TEST (test_convert_3)
{
  // this has "no data" in a stage, but still has a channel gain
  run_test ("data/station-3.xml", "data/response-3");
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_convert_1);
  tcase_add_test (tc, test_convert_2);
  tcase_add_test (tc, test_convert_3);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-convert.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
