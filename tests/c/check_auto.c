
#include <check.h>
#include <fcntl.h>
#include <stdio.h>

#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"

void
run_test (char *input, char *response)
{
  FILE *in = NULL, *out = NULL;
  evalresp_logger *log = NULL;
  ck_assert (NULL != (in = fopen (input, "r")));
  //fail_if (x2r_xml2resp_auto (&in, log));
  ck_assert (EVALRESP_OK == evalresp_xml_stream_to_resp_stream_auto (log, in, NULL, &out));
  ck_assert_msg (out != NULL, "Should not get here, auto detection returned okay but no output fd");
  if (in != out)
  {
    fclose (in);
    in = NULL;
  }
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
               response, input, line, a, b);
  }
}

START_TEST (test_auto_1)
{
  run_test ("data/station-1.xml", "data/response-1");
}
END_TEST

START_TEST (test_auto_2)
{
  run_test ("data/response-1", "data/response-1");
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_auto_1);
  tcase_add_test (tc, test_auto_2);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-auto.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
