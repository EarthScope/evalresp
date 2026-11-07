
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "harness/harness.h"

static double
max_err (const char *file1, const char *file2)
{
  FILE *f1, *f2;
  double v1, v2, err, max, max_err = 0.0;
  int ok1 = 1, ok2 = 1, oops = 0;

  ER_CHECK_MSG (f1 = fopen (file1, "r"), "%s", file1);
  ER_CHECK_MSG (f2 = fopen (file2, "r"), "%s", file2);

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
  printf ("max error for %s: %f%%\n", file1, max_err * 100);

  fclose (f1);
  fclose (f2);
  return max_err;
}

static void
test_response (void)
{
  const char *data = er_data_path ("station-1.xml");
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL, err;
  evalresp_response *response = NULL;

  printf ("input from %s\n", data);

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  ER_CHECK (freqs = calloc (n_freq, sizeof (*freqs)));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  ER_CHECK (response = evresp ("ANMO", "BH1", "IU", "00", "2015,1,00:00:00", "VEL", (char *)data, freqs, n_freq,
                               "AP", "-v", 0, 99, 0, 0, 0.1, 1));
  print_resp (freqs, n_freq, response, "AP", 0);

  /* print_resp wrote its files to the work directory */
  err = max_err (er_data_path ("AMP.IU.ANMO.00.BH1"), "AMP.IU.ANMO.00.BH1");
  ER_CHECK_MSG (err <= 0.1, "Error > 10%%");
  err = max_err (er_data_path ("PHASE.IU.ANMO.00.BH1"), "PHASE.IU.ANMO.00.BH1");
  ER_CHECK_MSG (err <= 0.1, "Error > 10%%");
}

const er_test unit_response_tests[] = {
    {"response", test_response, NULL},
    {NULL, NULL, NULL},
};
