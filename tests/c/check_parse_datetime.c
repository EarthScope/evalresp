
#include <check.h>
#include <fcntl.h>
#include <stdio.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/xml_to_dom.h"
#include "evalresp_log/log.h"

START_TEST (test_parse_datetime)
{
  evalresp_logger *log = NULL;
  time_t epoch, target = 946684800;
  fail_if (x2r_parse_iso_datetime (log, "2000-01-01T00:00:00", &epoch));
  fail_if (epoch != target, "Bad epoch: %ld (error %d)", epoch, epoch - target);
}
END_TEST

START_TEST (test_32bit_bug)
{
  evalresp_logger *log = NULL;
  time_t epoch, target = 4102444800;
  fail_if (x2r_parse_iso_datetime (log, "2100-01-01T00:00:00", &epoch));
  fail_if (epoch != target, "Bad epoch: %ld (error %d)", epoch, epoch - target);
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_parse_datetime);
  tcase_add_test (tc, test_32bit_bug);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-parse_datetime.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
