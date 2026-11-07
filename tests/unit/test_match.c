
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "harness/harness.h"

static FILE *
open_path (const char *dir, const char *file, const char *mode)
{
  FILE *f = NULL;
  char *path = calloc (strlen (dir) + strlen (file) + 2, sizeof (*path));
  sprintf (path, "%s/%s", dir, file);
  ER_CHECK_MSG (f = fopen (path, mode), "%s", path);
  free (path);
  return f;
}

static double
max_err (const char *dir1, const char *dir2, const char *name)
{

  FILE *f1, *f2;
  double v1, v2, err, max, max_err = 0.0;
  int ok1 = 1, ok2 = 1, oops = 0;

  ER_CHECK (f1 = open_path (dir1, name, "r"));
  ER_CHECK (f2 = open_path (dir2, name, "r"));

  while (ok1 != EOF && ok2 != EOF && oops++ < 100)
  {
    ok1 = fscanf (f1, "%lf", &v1);
    ok2 = fscanf (f2, "%lf", &v2);
    ER_CHECK (ok1 == ok2);
    if (ok1 != EOF)
    {
      max = fabs (v1);
      max = fabs (v2) > max ? fabs (v2) : max;
      err = fabs (v1 - v2) / max;
      max_err = err > max_err ? err : max_err;
      printf ("%f %f = %5.2f%%\n", v1, v2, 100 * err);
    }
  }
  printf ("max error for %s: %f%%\n", name, max_err * 100);

  fclose (f1);
  fclose (f2);
  return max_err;
}

#define BSIZE 4096

static void
cp_file (const char *from_dir, const char *to_dir, const char *name)
{

  char buffer[BSIZE];
  FILE *f1, *f2;
  int n1, n2;

  ER_CHECK (f1 = open_path (from_dir, name, "r"));
  ER_CHECK (f2 = open_path (to_dir, name, "w"));

  while ((n1 = fread (buffer, sizeof (char), BSIZE, f1)) > 0)
  {
    n2 = fwrite (buffer, sizeof (char), n1, f2);
    ER_CHECK (n1 == n2);
  }
  printf ("copied %s from %s to %s\n", name, from_dir, to_dir);

  fclose (f1);
  fclose (f2);
}

static void
test_match (void)
{

  char tmpdir[ER_PATH_MAX];
  const char *data = er_data_dir ();
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL, err;
  evalresp_response *response = NULL;

  /* the work directory receives the copies and the generated files */
  ER_CHECK (er_getcwd (tmpdir, sizeof (tmpdir)));

  cp_file (data, tmpdir, "RESP.IU.ANMO.00.BHZ");
  cp_file (data, tmpdir, "RESP.IU.ANMO.10.BHZ");
  cp_file (data, tmpdir, "RESP.IU.ANMO..BHZ");

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  ER_CHECK (freqs = calloc (n_freq, sizeof (*freqs)));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  ER_CHECK (NULL != (response = evresp_itp ("ANMO", "BHZ", "IU", "??", "2015,1,00:00:00", "VEL",
                                            tmpdir, freqs, n_freq, "AP", "-v", 0, 99, 0, 0, 0, 0, 0, 0.1, 0)));
  print_resp (freqs, n_freq, response, "AP", 0);

  // reference files take from evalresp
  err = max_err (data, tmpdir, "AMP.IU.ANMO.00.BHZ");
  ER_CHECK_MSG (err <= 0.01, "Error > 1%%");
  err = max_err (data, tmpdir, "PHASE.IU.ANMO.00.BHZ");
  ER_CHECK_MSG (err <= 0.01, "Error > 1%%");
  err = max_err (data, tmpdir, "AMP.IU.ANMO.10.BHZ");
  ER_CHECK_MSG (err <= 0.01, "Error > 1%%");
  err = max_err (data, tmpdir, "PHASE.IU.ANMO.10.BHZ");
  ER_CHECK_MSG (err <= 0.01, "Error > 1%%");
}

static void
test_match_subdir (void)
{
  char tmpdir[ER_PATH_MAX], subdir[ER_PATH_MAX];
  double freqs[1] = {1.0};
  evalresp_response *response = NULL;

  /* the matched files live outside the current directory */
  ER_CHECK (er_getcwd (tmpdir, sizeof (tmpdir)));
  ER_CHECK (strlen (tmpdir) + 8 < sizeof (subdir));
  sprintf (subdir, "%s/subdir", tmpdir);
  ER_CHECK (!er_mkdir_p (subdir));
  cp_file (er_data_dir (), subdir, "RESP.IU.ANMO.00.BHZ");

  ER_CHECK (NULL != (response = evresp_itp ("ANMO", "BHZ", "IU", "??", "2015,1,00:00:00", "VEL",
                                            subdir, freqs, 1, "AP", "-v", 0, 99, 0, 0, 0, 0, 0, 0.1, 0)));
  evalresp_free_response (&response);
}

static void
test_match_seedresp (void)
{
  char tmpdir[ER_PATH_MAX], seeddir[ER_PATH_MAX];
  double freqs[1] = {1.0};
  evalresp_response *response = NULL;

  /* nothing matches in the current directory, so the SEEDRESP directory is searched */
  ER_CHECK (er_getcwd (tmpdir, sizeof (tmpdir)));
  ER_CHECK (strlen (tmpdir) + 9 < sizeof (seeddir));
  sprintf (seeddir, "%s/seedresp", tmpdir);
  ER_CHECK (!er_mkdir_p (seeddir));
  cp_file (er_data_dir (), seeddir, "RESP.IU.ANMO.00.BHZ");
  ER_CHECK (!er_setenv ("SEEDRESP", seeddir));

  response = evresp_itp ("ANMO", "BHZ", "IU", "00", "2015,1,00:00:00", "VEL",
                         NULL, freqs, 1, "AP", "-v", 0, 99, 0, 0, 0, 0, 0, 0.1, 0);
  er_setenv ("SEEDRESP", NULL);
  ER_CHECK (NULL != response);
  evalresp_free_response (&response);
}

const er_test unit_match_tests[] = {
    {"match", test_match, NULL},
    {"match_subdir", test_match_subdir, NULL},
    {"match_seedresp", test_match_seedresp, NULL},
    {NULL, NULL, NULL},
};
