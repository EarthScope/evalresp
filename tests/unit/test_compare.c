#include <stdio.h>
#include <string.h>

#include "harness/harness.h"

static void
write_file (const char *name, const char *text)
{
  FILE *f = fopen (name, "wb");
  ER_CHECK_MSG (f != NULL, "cannot write %s", name);
  fputs (text, f);
  fclose (f);
}

/* compares "result" with "target" in the work directory */
static int
compare (const char *result, const char *target, int ncols, double tol, int absolute, int log_text)
{
  char msg[1024];
  er_cmp o;
  memset (&o, 0, sizeof (o));
  o.ncols = ncols;
  o.tol = tol;
  o.absolute = absolute;
  o.log_text = log_text;
  write_file ("result", result);
  write_file ("target", target);
  return er_compare_files ("result", "target", &o, msg, sizeof (msg));
}

static void
test_compare_identical (void)
{
  ER_CHECK (0 == compare ("title\n1.0 2.0\n3.0 4.0\n", "title\n1.0 2.0\n3.0 4.0\n", 2, 0, 0, 0));
  ER_CHECK (0 == compare ("", "", 2, 0, 0, 0));
}

static void
test_compare_relative (void)
{
  /* default tolerance is 1e-5 */
  ER_CHECK (0 == compare ("1 100.0005\n", "1 100.0\n", 2, 0, 0, 0));
  ER_CHECK (0 != compare ("1 100.01\n", "1 100.0\n", 2, 0, 0, 0));
  ER_CHECK (0 == compare ("1 100.01\n", "1 100.0\n", 2, 1e-3, 0, 0));
  /* below the tolerance in magnitude the test is absolute */
  ER_CHECK (0 == compare ("1 1e-9\n", "1 2e-9\n", 2, 0, 0, 0));
}

static void
test_compare_absolute (void)
{
  ER_CHECK (0 == compare ("1 100\n", "1 -200\n", 2, 360, 1, 0));
  ER_CHECK (0 != compare ("1 100\n", "1 -300\n", 2, 360, 1, 0));
}

static void
test_compare_text_lines (void)
{
  /* lines that are not numbers must be identical */
  ER_CHECK (0 != compare ("title A\n1 2\n", "title B\n1 2\n", 2, 0, 0, 0));
  /* so must lines with the wrong number of columns */
  ER_CHECK (0 != compare ("1 2 3\n", "1 2 4\n", 2, 0, 0, 0));
  ER_CHECK (0 == compare ("1 2 3\n", "1 2 3\n", 2, 0, 0, 0));
  /* nan only matches nan */
  ER_CHECK (0 != compare ("1 nan\n", "1 2\n", 2, 0, 0, 0));
}

static void
test_compare_length (void)
{
  ER_CHECK (0 != compare ("1 2\n1 2\n", "1 2\n", 2, 0, 0, 0));
  ER_CHECK (0 != compare ("1 2\n", "1 2\n1 2\n", 2, 0, 0, 0));
  ER_CHECK (0 != compare ("a\n", "a\n\n", 0, 0, 0, 0));
}

static void
test_compare_line_endings (void)
{
  ER_CHECK (0 == compare ("a\r\n1 2\r\n", "a\n1 2\n", 2, 0, 0, 0));
  ER_CHECK (0 == compare ("a\n1 2", "a\n1 2\n", 2, 0, 0, 0));
}

static void
test_compare_log_text (void)
{
  /* the timestamp and the version of the banner are masked */
  ER_CHECK (0 == compare ("Sat Nov  7 11:16:57 2026 [INFO] << EVALRESP RESPONSE OUTPUT V5.0.1 >>\n"
                          "Sat Nov  7 11:16:57 2026 [WARN] a [b] c\n"
                          "continuation\n",
                          "INFO] << EVALRESP RESPONSE OUTPUT VROBOT >>\nWARN] a [b] c\ncontinuation\n", 0, 0, 0, 1));
  ER_CHECK (0 != compare ("Sat Nov  7 11:16:57 2026 [INFO] one\n", "INFO] two\n", 0, 0, 0, 1));
  /* without log_text nothing is masked */
  ER_CHECK (0 != compare ("2026 [INFO] one\n", "INFO] one\n", 0, 0, 0, 0));
}

const er_test unit_compare_tests[] = {
    {"identical", test_compare_identical, NULL},
    {"relative", test_compare_relative, NULL},
    {"absolute", test_compare_absolute, NULL},
    {"text_lines", test_compare_text_lines, NULL},
    {"length", test_compare_length, NULL},
    {"line_endings", test_compare_line_endings, NULL},
    {"log_text", test_compare_log_text, NULL},
    {NULL, NULL, NULL},
};
