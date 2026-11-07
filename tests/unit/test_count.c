#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "harness/harness.h"

/* print_resp writes its AMP and PHASE files to the current directory */
static void
run_test (const char *data)
{
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL;
  evalresp_response *response = NULL;

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  ER_CHECK (freqs = calloc (n_freq, sizeof (*freqs)));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  ER_CHECK (response = evresp ("HAW", "HHZ", "CO", "00", "2011,1,00:00:00", "VEL", (char *)data, freqs, n_freq, "AP", "-v", 0,
                               99, 0, 0, 0.1, 0));
  print_resp (freqs, n_freq, response, "AP", 0);
}

// the target file has COUNT rather than COUNTS as units.  so the test is simply whether
// the data can be parsed without error.
static void
test_count (void)
{
  run_test (er_data_path ("RESP.HAW.CO.00.HHZ.count"));
}

static void
test_counts (void)
{
  run_test (er_data_path ("RESP.HAW.CO.00.HHZ.counts"));
}

const er_test unit_count_tests[] = {
    {"count", test_count, NULL},
    {"counts", test_counts, NULL},
    {NULL, NULL, NULL},
};
