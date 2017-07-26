
#include <check.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evalresp/private.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"

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

double
max_err (const char *dir1, const char *file1, const char *dir2, const char *file2)
{

  FILE *f1, *f2;
  double v1, v2, err, max, max_err = 0.0;
  int ok1 = 1, ok2 = 1, oops = 0;

  fail_if (!(f1 = open_path (dir1, file1)));
  fail_if (!(f2 = open_path (dir2, file2)));

  while (ok1 != EOF && ok2 != EOF && oops++ < 100)
  {
    ok1 = fscanf (f1, "%lf", &v1);
    ok2 = fscanf (f2, "%lf", &v2);
    fail_if (ok1 != ok2);
    if (ok1 != EOF)
    {
      max = fabs (v1);
      max = fabs (v2) > max ? fabs (v2) : max;
      err = fabs (v1 - v2) / max;
      max_err = err > max_err ? err : max_err;
      printf ("%f %f = %5.2f%%\n", v1, v2, 100 * err);
    }
  }
  printf ("max error for %s: %f%%\n", file1, max_err * 100);

  fclose (f1);
  fclose (f2);
  return max_err;
}

START_TEST (test_response)
{

  char cwd[1000];
  char data[1000];
  char tmpdir[] = "/tmp/check_response_XXXXXX";
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL, err;
  evalresp_response *response = NULL;

  fail_if (!mkdtemp (tmpdir));
  fail_if (!getcwd (cwd, 1000));
  fail_if (chdir (tmpdir));
  sprintf (data, "%s/%s", cwd, "data/station-1.xml");
  printf ("input from %s\n", data);
  printf ("output to %s\n", tmpdir);

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  fail_if (!(freqs = calloc (n_freq, sizeof (*freqs))));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  fail_if (!(response = evresp ("ANMO", "BH1", "IU", "00", "2015,1,00:00:00", "VEL", data, freqs, n_freq,
                                "AP", "-v", 0, 99, 0, 0, 0.1, 1)));
  print_resp (freqs, n_freq, response, "AP", 0);

  err = max_err (cwd, "data/AMP.IU.ANMO.00.BH1", tmpdir, "AMP.IU.ANMO.00.BH1");
  fail_if (err > 0.1, "Error > 10%");
  err = max_err (cwd, "data/PHASE.IU.ANMO.00.BH1", tmpdir, "PHASE.IU.ANMO.00.BH1");
  fail_if (err > 0.1, "Error > 10%");
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_response);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-response.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
