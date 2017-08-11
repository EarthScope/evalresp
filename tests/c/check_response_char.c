
#include <check.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evalresp/private.h"
#include "evalresp/public_api.h"
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
max_err (const char *check_dir, const char *check_file, const char *test_string)
{

  FILE *check_fd;
  double check_num, test_num, err, max, max_err = 0.0;
  int ok1 = 1, ok2 = 1, oops = 0;
  int test_read_offset, test_read_len;

  ck_assert (NULL != (check_fd = open_path (check_dir, check_file)));

  for (oops = 0, test_read_offset = 0; ok1 != EOF && ok2 != EOF && oops < 100; oops++, test_read_offset += test_read_len)
  {
    ok1 = fscanf (check_fd, "%lf", &check_num);
    ok2 = sscanf (test_string + test_read_offset, "%lf%n", &test_num, &test_read_len);
    ck_assert (ok1 == ok2);
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

evalresp_response *
get_response (char *cwd, char *input, char *stalst,
              char *chalst, char *net_code, char *locidlst,
              char *date_time)
{
  char data[1000];
  int n_freq = 19, i;
  double lof = 0.1, hif = 10, delta, *freqs = NULL;
  evalresp_response *response = NULL;

  sprintf (data, "%s/%s", cwd, "data/station-1.xml");
  printf ("input from %s\n", data);

  delta = (log10 (hif) - log10 (lof)) / (n_freq - 1);
  ck_assert (NULL != (freqs = calloc (n_freq, sizeof (*freqs))));
  for (i = 0; i < n_freq; ++i)
  {
    freqs[i] = pow (10.0, log10 (lof) + i * delta);
  }

  ck_assert (NULL != (response = evresp (stalst, chalst, net_code, locidlst, date_time, "VEL", data, freqs, n_freq,
                                         "AP", "-v", 0, 99, 0, 0, 0.1, 1)));
  return response;
}

START_TEST (test_response_char_amp)
{

  char cwd[1000];
  char *test_string = NULL;
  double err;
  evalresp_response *response = NULL;
  evalresp_logger *log = NULL;

  ck_assert (NULL != getcwd (cwd, 1000));

  response = get_response (cwd, "data/station-1.xml", "ANMO", "BH1", "IU", "00", "2015,1,00:00:00");

  ck_assert_msg (evalresp_response_to_char (log, response, 0, evalresp_amplitude_file_format, &test_string) == EVALRESP_OK,
                 "Failed to create char");

  err = max_err (cwd, "data/AMP.IU.ANMO.00.BH1", test_string);
  ck_assert_msg (err <= 0.1, "Error > 10%");
}
END_TEST

START_TEST (test_response_char_phase)
{
  char cwd[1000];
  char *test_string = NULL;
  double err;
  evalresp_response *response = NULL;
  evalresp_logger *log = NULL;

  ck_assert (NULL != getcwd (cwd, 1000));

  response = get_response (cwd, "data/station-1.xml", "ANMO", "BH1", "IU", "00", "2015,1,00:00:00");

  ck_assert_msg (evalresp_response_to_char (log, response, 0, evalresp_phase_file_format, &test_string) == EVALRESP_OK,
                 "Failed to create char");

  err = max_err (cwd, "data/PHASE.IU.ANMO.00.BH1", test_string);
  ck_assert_msg (err <= 0.1, "Error > 10%");
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_response_char_amp);
  tcase_add_test (tc, test_response_char_phase);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-response-char.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
