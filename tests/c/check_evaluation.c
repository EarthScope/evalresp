#include <check.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "evalresp/constants.h"
#include "evalresp/public_api.h"

START_TEST (test_no_options)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, NULL,
                                          &channels));
  fail_if (channels->nchannels != 2, "Unexpected number of channels: %d", channels->nchannels);
  fail_if (evalresp_channel_to_response (NULL, channels->channels[0], NULL, &response));
  fail_if (response->nfreqs != 1);
  fail_if (fabs (response->freqs[0] - 1) > 1e-6, "Freq: %f", response->freqs[0]);
  /* changing value because of chek_channel being introduced */
  fail_if (fabs (response->rvec[0].real - 918243620.549808) > 1e-3, "Real: %f", response->rvec[0].real);
  fail_if (fabs (response->rvec[0].imag - -392298381.164822) > 1e-3, "Imag: %f", response->rvec[0].imag);
  //fail_if (fabs (response->rvec[0].real - -998127849.638739) > 1e-3, "Real: %f", response->rvec[0].real);
  //fail_if (fabs (response->rvec[0].imag - 63588947.373697) > 1e-3, "Imag: %f", response->rvec[0].imag);
  evalresp_free_channels (&channels);
  evalresp_free_response (&response);
}
END_TEST

START_TEST (test_start)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;
  fail_if (evalresp_new_options (NULL, &options));
  fail_if (evalresp_set_start_stage (NULL, options, "2"));
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", options, NULL,
                                          &channels));
  fail_if (channels->nchannels != 2, "Unexpected number of channels: %d", channels->nchannels);
  fail_if (evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  fail_if (response->nfreqs != 1);
  fail_if (fabs (response->freqs[0] - 1) > 1e-6, "Freq: %f", response->freqs[0]);
  fail_if (fabs (response->rvec[0].real - 419430.000000) > 1e-3, "Real: %f", response->rvec[0].real);
  fail_if (fabs (response->rvec[0].imag - 0.000000) > 1e-3, "Imag: %f", response->rvec[0].imag);
  evalresp_free_channels (&channels);
  evalresp_free_response (&response);
  evalresp_free_options (&options);
}
END_TEST

START_TEST (test_freqs)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;

  fail_if (evalresp_new_options (NULL, &options));
  fail_if (evalresp_set_frequency (NULL, options, "2", "4", "3"));

  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", options, NULL,
                                          &channels));
  fail_if (channels->nchannels != 2, "Unexpected number of channels: %d", channels->nchannels);
  fail_if (evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  fail_if (response->nfreqs != 3);
  fail_if (fabs (response->freqs[0] - 2) > 1e-4, "Freq 0: %f", response->freqs[0]);
  fail_if (fabs (response->freqs[1] - 2.828427) > 1e-4, "Freq 1: %f", response->freqs[1]);
  fail_if (fabs (response->freqs[2] - 4) > 1e-4, "Freq 2: %f", response->freqs[2]);
  evalresp_free_response (&response);

  options->lin_freq = 1;
  fail_if (evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  fail_if (response->nfreqs != 3);
  fail_if (fabs (response->freqs[0] - 2) > 1e-4, "Freq 0: %f", response->freqs[0]);
  fail_if (fabs (response->freqs[1] - 3) > 1e-4, "Freq 1: %f", response->freqs[1]);
  fail_if (fabs (response->freqs[2] - 4) > 1e-4, "Freq 2: %f", response->freqs[2]);
  evalresp_free_response (&response);

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_no_options);
  tcase_add_test (tc, test_start);
  tcase_add_test (tc, test_freqs);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-evaluation.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
