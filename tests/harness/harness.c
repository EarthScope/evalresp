#include "harness.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char data_dir[ER_PATH_MAX] = "data";
static char expected_dir[ER_PATH_MAX] = "expected";
static char bin_dir[ER_PATH_MAX] = "../src";
static char work_dir[ER_PATH_MAX] = "work";

static jmp_buf fail_jmp;
static char fail_msg[2048];
static const void *current_ctx = NULL;

void
er_fail (const char *file, int line, const char *fmt, ...)
{
  va_list ap;
  int n = snprintf (fail_msg, sizeof (fail_msg), "%s:%d: ", file, line);
  va_start (ap, fmt);
  if (n > 0 && (size_t)n < sizeof (fail_msg))
    vsnprintf (fail_msg + n, sizeof (fail_msg) - n, fmt, ap);
  va_end (ap);
  longjmp (fail_jmp, 1);
}

const void *
er_test_ctx (void)
{
  return current_ctx;
}

static const char *
join_path (const char *dir, const char *name)
{
  static char bufs[4][ER_PATH_MAX];
  static int next = 0;
  char *buf = bufs[next];
  next = (next + 1) % 4;
  if (strlen (dir) + strlen (name) + 2 > ER_PATH_MAX)
    er_fail (__FILE__, __LINE__, "path too long: %s/%s", dir, name);
  sprintf (buf, "%s/%s", dir, name);
  return buf;
}

const char *
er_data_path (const char *name)
{
  return join_path (data_dir, name);
}

const char *
er_data_dir (void)
{
  return data_dir;
}

const char *
er_expected_path (const char *name)
{
  return join_path (expected_dir, name);
}

const char *
er_bin_path (const char *program)
{
  char exe[ER_PATH_MAX];
  er_exe_name (program, exe, sizeof (exe));
  return join_path (bin_dir, exe);
}

void
er_assert_files_match (const char *result, const char *target, const er_cmp *opts)
{
  char msg[1024];
  if (er_compare_files (result, target, opts, msg, sizeof (msg)))
    er_fail (__FILE__, __LINE__, "%s", msg);
}

static int
set_dir (char *dest, const char *path)
{
  char tmp[ER_PATH_MAX];
  if (strlen (path) >= sizeof (tmp))
  {
    fprintf (stderr, "path too long: %s\n", path);
    return -1;
  }
  strcpy (tmp, path);
  if (er_abspath (tmp, dest, ER_PATH_MAX))
  {
    fprintf (stderr, "path too long: %s\n", tmp);
    return -1;
  }
  return 0;
}

static int
matches (const char *full, int nfilters, char **filters)
{
  int i;
  if (nfilters == 0)
    return 1;
  for (i = 0; i < nfilters; ++i)
    if (strstr (full, filters[i]))
      return 1;
  return 0;
}

static void
usage (const char *prog)
{
  fprintf (stderr,
           "usage: %s [-v] [--list] [--data DIR] [--expected DIR] [--bin DIR] [--work DIR] [PATTERN...]\n"
           "Runs the tests whose \"suite.test\" name contains any PATTERN (default: all).\n",
           prog);
}

int
main (int argc, char **argv)
{
  char start_dir[ER_PATH_MAX], test_dir[ER_PATH_MAX], full[256];
  char *filters[64];
  int nfilters = 0, verbose = 0, list = 0, i;
  int ran = 0, failed = 0;
  const er_suite *suite;
  const char *dirs[4] = {NULL, NULL, NULL, NULL}; /* data, expected, bin, work */

  for (i = 1; i < argc; ++i)
  {
    if (!strcmp (argv[i], "-v"))
      verbose = 1;
    else if (!strcmp (argv[i], "--list"))
      list = 1;
    else if (!strcmp (argv[i], "--data") && i + 1 < argc)
      dirs[0] = argv[++i];
    else if (!strcmp (argv[i], "--expected") && i + 1 < argc)
      dirs[1] = argv[++i];
    else if (!strcmp (argv[i], "--bin") && i + 1 < argc)
      dirs[2] = argv[++i];
    else if (!strcmp (argv[i], "--work") && i + 1 < argc)
      dirs[3] = argv[++i];
    else if (argv[i][0] == '-' || nfilters == 64)
    {
      usage (argv[0]);
      return 2;
    }
    else
      filters[nfilters++] = argv[i];
  }

  /* paths are made absolute because tests change directory */
  if (set_dir (data_dir, dirs[0] ? dirs[0] : data_dir) || set_dir (expected_dir, dirs[1] ? dirs[1] : expected_dir) ||
      set_dir (bin_dir, dirs[2] ? dirs[2] : bin_dir) || set_dir (work_dir, dirs[3] ? dirs[3] : work_dir) ||
      !er_getcwd (start_dir, sizeof (start_dir)))
    return 2;

  for (suite = er_get_suites (); suite->name; ++suite)
  {
    const er_test *t;
    for (t = suite->tests; t->name; ++t)
    {
      snprintf (full, sizeof (full), "%s.%s", suite->name, t->name);
      if (!matches (full, nfilters, filters))
        continue;
      if (list)
      {
        printf ("%s\n", full);
        continue;
      }

      ++ran;
      fail_msg[0] = '\0';
      if ((size_t)snprintf (test_dir, sizeof (test_dir), "%s/%s/%s", work_dir, suite->name, t->name) >= sizeof (test_dir))
      {
        printf ("FAIL %s: work path too long\n", full);
        ++failed;
        continue;
      }
      if (er_mkdir_p (test_dir) || er_remove_contents (test_dir) || er_chdir (test_dir))
      {
        printf ("FAIL %s: cannot prepare %s\n", full, test_dir);
        ++failed;
        continue;
      }
      if (verbose)
        printf ("RUN  %s\n", full);
      fflush (stdout);

      current_ctx = t->ctx;
      if (setjmp (fail_jmp) == 0)
      {
        t->fn ();
        printf ("ok   %s\n", full);
      }
      else
      {
        printf ("FAIL %s: %s\n", full, fail_msg);
        ++failed;
      }
      fflush (stdout);
      if (er_chdir (start_dir))
      {
        printf ("cannot return to %s\n", start_dir);
        return 2;
      }
    }
  }

  if (list)
    return 0;
  if (ran == 0)
  {
    fprintf (stderr, "no tests match\n");
    return 2;
  }
  printf ("%d tests, %d failed\n", ran, failed);
  return failed ? 1 : 0;
}
