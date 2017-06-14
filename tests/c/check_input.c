#include <check.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "evalresp/input.h"
#include "evalresp/ugly.h"

START_TEST (test_slurp_line)
{
  char *input = "line 1\nline 2\nline 3";
  char *start = input;
  char line[MAXLINELEN];
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 1\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 2\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 3"), "Slurped [%s] (size %d)", line, strlen (line));
  fail_if (*start, "Start is [%s]", start);
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, ""), "Slurped [%s] (size %d)", line, strlen (line));
  fail_if (*start, "Start is [%s]", start);
}
END_TEST

START_TEST (test_slurp_line_buffer)
{
  char *input = "\n1\n12\n123\n";
  char *start = input;
  char line[3];
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "1\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "12"), "Slurped [%s] (size %d)", line, strlen (line));
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_slurp_line);
  tcase_add_test (tc, test_slurp_line_buffer);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-log.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
