
#include <check.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evalresp/input.h"
#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"

FILE *
open_path (const char *dir, const char *file)
{
  FILE *f = NULL;
  char *path = calloc (strlen (dir) + strlen (file) + 2, sizeof (*path));
  sprintf (path, "%s/%s", dir, file);
  ck_assert_msg (NULL != (f = fopen (path, "r")), path);
  free (path);
  return f;
}

void
run_xml_to_char_test (const char *xml_path, const char *resp_path)
{
  FILE *in_fd = NULL, *check_fd = NULL;
  char cwd[1000], *test_char = NULL, *test_xml = NULL, *check_char = NULL;
  evalresp_logger *log = NULL;

  ck_assert (NULL != getcwd (cwd, 1000));
  in_fd = open_path (cwd, xml_path);
  ck_assert (EVALRESP_OK == file_to_char (log, in_fd, &test_xml));
  fclose (in_fd);
  ck_assert (EVALRESP_OK == evalresp_xml_to_char (log, 1, test_xml, &test_char));
  free (test_xml);
  check_fd = open_path (cwd, resp_path);
  ck_assert (EVALRESP_OK == file_to_char (log, check_fd, &check_char));
  fclose (check_fd);
  ck_assert (0 == strcmp (test_char, check_char));
  free (check_char);
  free (test_char);
}

START_TEST (test_xml_to_char_1_flag_1)
{
  run_xml_to_char_test ("data/station-1.xml", "data/response-1");
}
END_TEST

START_TEST (test_xml_to_char_1_flag_2)
{
  run_xml_to_char_test ("data/station-2.xml", "data/response-2");
}
END_TEST

START_TEST (test_xml_to_char_1_flag_3)
{
  run_xml_to_char_test ("data/station-3.xml", "data/response-3");
}
END_TEST

START_TEST (test_xml_to_char_0_flag)
{
  char *test_xml = NULL, *test_char = NULL;
  evalresp_logger *log = NULL;
  ck_assert (EVALRESP_OK == evalresp_xml_to_char (log, 0, test_xml, &test_char));
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_xml_to_char_0_flag);
  tcase_add_test (tc, test_xml_to_char_1_flag_1);
  tcase_add_test (tc, test_xml_to_char_1_flag_2);
  tcase_add_test (tc, test_xml_to_char_1_flag_3);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-xml-to-char.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
