#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/constants.h"
#include "evalresp/public_api.h"
#include "harness/harness.h"

static void
test_no_options (void)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, NULL,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 2), "Unexpected number of channels: %d", channels->nchannels);
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], NULL, &response));
  ER_CHECK (!(response->nfreqs != 1));
  ER_CHECK_MSG (!(fabs (response->freqs[0] - 1) > 1e-6), "Freq: %f", response->freqs[0]);
  /* changing value because of chek_channel being introduced */
  ER_CHECK_MSG (!(fabs (response->rvec[0].real - 918243620.549808) > 1e-3), "Real: %f", response->rvec[0].real);
  ER_CHECK_MSG (!(fabs (response->rvec[0].imag - -392298381.164822) > 1e-3), "Imag: %f", response->rvec[0].imag);
  // ER_CHECK_MSG (!(fabs (response->rvec[0].real - -998127849.638739) > 1e-3), "Real: %f", response->rvec[0].real);
  // ER_CHECK_MSG (!(fabs (response->rvec[0].imag - 63588947.373697) > 1e-3), "Imag: %f", response->rvec[0].imag);
  evalresp_free_channels (&channels);
  evalresp_free_response (&response);
}

static void
test_start (void)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;
  ER_CHECK (!evalresp_new_options (NULL, &options));
  ER_CHECK (!evalresp_set_start_stage (NULL, options, "2"));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), options, NULL,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 2), "Unexpected number of channels: %d", channels->nchannels);
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK (!(response->nfreqs != 1));
  ER_CHECK_MSG (!(fabs (response->freqs[0] - 1) > 1e-6), "Freq: %f", response->freqs[0]);
  ER_CHECK_MSG (!(fabs (response->rvec[0].real - 419430.000000) > 1e-3), "Real: %f", response->rvec[0].real);
  ER_CHECK_MSG (!(fabs (response->rvec[0].imag - 0.000000) > 1e-3), "Imag: %f", response->rvec[0].imag);
  evalresp_free_channels (&channels);
  evalresp_free_response (&response);
  evalresp_free_options (&options);
}

static void
test_freqs (void)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;

  ER_CHECK (!evalresp_new_options (NULL, &options));
  ER_CHECK (!evalresp_set_frequency (NULL, options, "2", "4", "3"));

  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), options, NULL,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 2), "Unexpected number of channels: %d", channels->nchannels);
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK (!(response->nfreqs != 3));
  ER_CHECK_MSG (!(fabs (response->freqs[0] - 2) > 1e-4), "Freq 0: %f", response->freqs[0]);
  ER_CHECK_MSG (!(fabs (response->freqs[1] - 2.828427) > 1e-4), "Freq 1: %f", response->freqs[1]);
  ER_CHECK_MSG (!(fabs (response->freqs[2] - 4) > 1e-4), "Freq 2: %f", response->freqs[2]);
  evalresp_free_response (&response);

  options->lin_freq = 1;
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK (!(response->nfreqs != 3));
  ER_CHECK_MSG (!(fabs (response->freqs[0] - 2) > 1e-4), "Freq 0: %f", response->freqs[0]);
  ER_CHECK_MSG (!(fabs (response->freqs[1] - 3) > 1e-4), "Freq 1: %f", response->freqs[1]);
  ER_CHECK_MSG (!(fabs (response->freqs[2] - 4) > 1e-4), "Freq 2: %f", response->freqs[2]);
  evalresp_free_response (&response);

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

const er_test unit_evaluation_tests[] = {
    {"no_options", test_no_options, NULL},
    {"start", test_start, NULL},
    {"freqs", test_freqs, NULL},
    {NULL, NULL, NULL},
};
