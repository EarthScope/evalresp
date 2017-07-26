
#include <check.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evalresp/private.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/wrappers.h"

FILE *
open_path (const char *dir, const char *file)
{
  FILE *f = NULL;
  char *path = calloc (strlen (dir) + strlen (file) + 2, sizeof (*path));
  sprintf (path, "%s/%s", dir, file);
  fail_if (!(f = fopen (path, "r")), path);
  free (path);
  return f;
}

void
run_test (const char *file)
{
  char cwd[1000];
  char data[1000];
  char tmpdir[] = "/tmp/check_count_XXXXXX";
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL;
  evalresp_response *response = NULL;

  fail_if (!mkdtemp (tmpdir));
  fail_if (!getcwd (cwd, 1000));
  fail_if (chdir (tmpdir));
  sprintf (data, "%s/%s", cwd, file);
  printf ("input from %s\n", data);
  printf ("output to %s\n", tmpdir);

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  fail_if (!(freqs = calloc (n_freq, sizeof (*freqs))));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  fail_if (!(response = evresp ("HAW", "HHZ", "CO", "00", "2011,1,00:00:00", "VEL", data, freqs, n_freq,
                                "AP", "-v", 0, 99, 0, 0, 0.1, 0)));
  print_resp (freqs, n_freq, response, "AP", 0);

  // TODO - delete directory?
}

// the target file has COUNT rather than COUNTS as units.  so the test is simply whether
// the data can be parsed without error.
START_TEST (test_count)
{
  run_test ("data/RESP.HAW.CO.00.HHZ.count");
}
END_TEST

START_TEST (test_counts)
{
  run_test ("data/RESP.HAW.CO.00.HHZ.counts");
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_count);
  tcase_add_test (tc, test_counts);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-count.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
