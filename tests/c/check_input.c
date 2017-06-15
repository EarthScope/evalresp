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

START_TEST (test_find_line)
{
  char *input = "B999F99 name1: value1 \nB666F66 name2: value2", *start;
  char line[MAXLINELEN];
  start = input;
  fail_if (!find_line (NULL, &start, ":", 999, 99, line));
  fail_if (strcmp (line, "value1 "), "'%s'", line);
  fail_if (!find_line (NULL, &start, ":", 666, 66, line));
  fail_if (strcmp (line, "value2"), "'%s'", line);
  start = input;
  fail_if (!find_line (NULL, &start, ":", 666, 66, line));
  fail_if (strcmp (line, "value2"), "'%s'", line);
}
END_TEST

START_TEST (test_find_field)
{
  char *input = "B999F99 name1: a b c \nB666F66 name2: value2", *start;
  char field[MAXLINELEN];
  start = input;
  fail_if (!find_field (NULL, &start, ":", 999, 99, 0, field));
  fail_if (strcmp (field, "a"), "'%s'", field);
  fail_if (!find_field (NULL, &start, ":", 666, 66, 0, field));
  fail_if (strcmp (field, "value2"), "'%s'", field);
  start = input;
  fail_if (!find_field (NULL, &start, ":", 999, 99, 2, field));
  fail_if (strcmp (field, "c"), "'%s'", field);
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
  tcase_add_test (tc, test_find_line);
  tcase_add_test (tc, test_find_field);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-log.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
