
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public_api.h"
#include "evalresp/public_compat.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "harness/harness.h"

static double
max_err (const char *check_file, const char *test_string)
{
  FILE *check_fd;
  double check_num, test_num, err, max, max_err = 0.0;
  int ok1 = 1, ok2 = 1, oops = 0;
  int test_read_offset, test_read_len;

  ER_CHECK_MSG (NULL != (check_fd = fopen (check_file, "r")), "%s", check_file);

  for (oops = 0, test_read_offset = 0; ok1 != EOF && ok2 != EOF && oops < 100; oops++, test_read_offset += test_read_len)
  {
    ok1 = fscanf (check_fd, "%lf", &check_num);
    ok2 = sscanf (test_string + test_read_offset, "%lf%n", &test_num, &test_read_len);
    ER_CHECK (ok1 == ok2);
    if (ok1 != EOF)
    {
      max = fabs (check_num);
      max = fabs (test_num) > max ? fabs (test_num) : max;
      err = fabs (check_num - test_num) / max;
      max_err = err > max_err ? err : max_err;
      printf ("%f %f = %5.2f%%\n", check_num, test_num, 100 * err);
    }
  }
  printf ("max error for %s: %f%%\n", check_file, max_err * 100);

  fclose (check_fd);
  return max_err;
}

static evalresp_response *
get_response (char *stalst, char *chalst, char *net_code, char *locidlst, char *date_time)
{
  const char *data = er_data_path ("station-1.xml");
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL;
  evalresp_response *response = NULL;

  printf ("input from %s\n", data);

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  ER_CHECK (NULL != (freqs = calloc (n_freq, sizeof (*freqs))));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  ER_CHECK (NULL != (response = evresp (stalst, chalst, net_code, locidlst, date_time, "VEL", (char *)data, freqs, n_freq,
                                        "AP", "-v", 0, 99, 0, 0, 0.1, 1)));
  return response;
}

static void
test_response_char_amp (void)
{
  char *test_string = NULL;
  double err;
  evalresp_response *response = NULL;
  evalresp_logger *log = NULL;

  response = get_response ("ANMO", "BH1", "IU", "00", "2015,1,00:00:00");

  ER_CHECK_MSG (evalresp_response_to_char (log, response, 0, evalresp_amplitude_file_format, &test_string) == EVALRESP_OK, "Failed to create char");

  err = max_err (er_data_path ("AMP.IU.ANMO.00.BH1"), test_string);
  ER_CHECK_MSG (err <= 0.1, "Error > 10%%");
}

static void
test_response_char_phase (void)
{
  char *test_string = NULL;
  double err;
  evalresp_response *response = NULL;
  evalresp_logger *log = NULL;

  response = get_response ("ANMO", "BH1", "IU", "00", "2015,1,00:00:00");

  ER_CHECK_MSG (evalresp_response_to_char (log, response, 0, evalresp_phase_file_format, &test_string) == EVALRESP_OK, "Failed to create char");

  err = max_err (er_data_path ("PHASE.IU.ANMO.00.BH1"), test_string);
  ER_CHECK_MSG (err <= 0.1, "Error > 10%%");
}

const er_test unit_response_char_tests[] = {
    {"amp", test_response_char_amp, NULL},
    {"phase", test_response_char_phase, NULL},
    {NULL, NULL, NULL},
};
