#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/constants.h"
#include "evalresp/public_api.h"
#include "evalresp/public_compat.h"
#include "evalresp_log/examples/to_file.h"
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

static void
test_b55_limits (void)
{
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;

  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->b55_interpolate = 1;
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IM.ATTU..BHE"), options, NULL,
                                            &channels));

  /* 10 Hz is the last frequency of the blockette */
  ER_CHECK (!evalresp_set_frequency (NULL, options, "0.001", "10", "100"));
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK_MSG (response->nfreqs == 75, "nfreqs: %d", response->nfreqs);
  ER_CHECK_MSG (fabs (response->freqs[response->nfreqs - 1] - 10) < 1e-6, "last: %f",
                response->freqs[response->nfreqs - 1]);
  evalresp_free_response (&response);

  /* rounding error above the limit does not drop the value */
  options->lin_freq = 1;
  ER_CHECK (!evalresp_set_frequency (NULL, options, "9", "10.000000000000002", "2"));
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK_MSG (response->nfreqs == 2, "nfreqs: %d", response->nfreqs);
  evalresp_free_response (&response);

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

static void
check_gain_only (const char *file, int station_xml)
{
  int i;
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;

  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->station_xml = station_xml;
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path (file), options, NULL, &channels));
  ER_CHECK (channels && channels->nchannels == 1);
  ER_CHECK (!evalresp_set_frequency (NULL, options, "0.1", "10", "5"));
  ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &response));
  ER_CHECK_MSG (response->nfreqs == 5, "nfreqs: %d", response->nfreqs);
  for (i = 0; i < response->nfreqs; ++i)
  {
    ER_CHECK_MSG (fabs (response->rvec[i].real - 1.0) < 1e-12, "%s real[%d]: %g", file, i,
                  response->rvec[i].real);
    ER_CHECK_MSG (fabs (response->rvec[i].imag) < 1e-12, "%s imag[%d]: %g", file, i,
                  response->rvec[i].imag);
  }

  evalresp_free_response (&response);
  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

static void
test_gain_only (void)
{
  check_gain_only ("station-gain-only.xml", 1);
  check_gain_only ("RESP.UW.LON9.T1.HAX", 0);
}

static void
test_log_no_filename (void)
{
  char text[8192], name[64];
  size_t n;
  FILE *fd;
  evalresp_logger *log;
  evalresp_channels *channels = NULL;
  evalresp_options *options = NULL;

  ER_CHECK (NULL != (fd = fopen ("channel.log", "w+")));
  ER_CHECK (NULL != (log = evalresp_logger_alloc (evalresp_log_to_file, (void *)fd)));
  ER_CHECK (!evalresp_new_options (NULL, &options));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), options, NULL, &channels));
  ER_CHECK (!evalresp_channel_to_log (log, options, channels->channels[0]));
  fflush (fd);
  rewind (fd);
  n = fread (text, 1, sizeof (text) - 1, fd);
  text[n] = '\0';

  snprintf (name, sizeof (name), "RESP.%s.%s.%s.%s", channels->channels[0]->network,
            channels->channels[0]->staname, channels->channels[0]->locid, channels->channels[0]->chaname);
  ER_CHECK_MSG (strstr (text, name), "%s missing from log: %s", name, text);
  ER_CHECK_MSG (!strstr (text, "(null)"), "null filename in log: %s", text);

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
  evalresp_logger_free (log);
  fclose (fd);
}

static void
test_arbitrary_freqs (void)
{
  const double freqs[] = {0.77, 0.013, 5.0, 0.1, 2.5};
  const int nfreqs = sizeof (freqs) / sizeof (freqs[0]);
  int i;
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL, *single = NULL;
  evalresp_options *options = NULL;
  char freq[32];

  ER_CHECK (!evalresp_new_options (NULL, &options));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), options, NULL,
                                            &channels));
  ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, freqs, nfreqs, &response));
  ER_CHECK_MSG (response->nfreqs == nfreqs, "nfreqs: %d", response->nfreqs);
  for (i = 0; i < nfreqs; ++i)
  {
    ER_CHECK_MSG (response->freqs[i] == freqs[i], "Freq %d: %f", i, response->freqs[i]);
    /* a single-frequency evaluation through the options must agree */
    snprintf (freq, sizeof (freq), "%.17g", freqs[i]);
    ER_CHECK (!evalresp_set_frequency (NULL, options, freq, freq, "1"));
    ER_CHECK (!evalresp_channel_to_response (NULL, channels->channels[0], options, &single));
    ER_CHECK_MSG (fabs (response->rvec[i].real - single->rvec[0].real) <= 1e-9 * fabs (single->rvec[0].real),
                  "Real %d: %f vs %f", i, response->rvec[i].real, single->rvec[0].real);
    ER_CHECK_MSG (fabs (response->rvec[i].imag - single->rvec[0].imag) <= 1e-9 * fabs (single->rvec[0].imag),
                  "Imag %d: %f vs %f", i, response->rvec[i].imag, single->rvec[0].imag);
    evalresp_free_response (&single);
  }
  evalresp_free_response (&response);

  /* invalid requests are rejected */
  ER_CHECK (evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, freqs, 0, &response));
  ER_CHECK (evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, NULL, nfreqs, &response));
  {
    const double negative[] = {1.0, -2.0};
    ER_CHECK (evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, negative, 2, &response));
  }

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

static void
test_legacy_arbitrary_freqs (void)
{
  double freqs[] = {0.77, 0.013, 5.0, 0.1, 2.5};
  const int nfreqs = sizeof (freqs) / sizeof (freqs[0]);
  int i;
  evalresp_response *response = NULL;

  ER_CHECK (NULL != (response = evresp ("ANMO", "BH1", "IU", "00", "2015,1,00:00:00", "VEL",
                                        (char *)er_data_path ("station-1.xml"), freqs, nfreqs,
                                        "AP", "", 0, 99, 0, 0, 0.1, 1)));
  ER_CHECK_MSG (response->nfreqs == nfreqs, "nfreqs: %d", response->nfreqs);
  for (i = 0; i < nfreqs; ++i)
  {
    ER_CHECK_MSG (response->freqs[i] == freqs[i], "Freq %d: %f", i, response->freqs[i]);
  }
  evalresp_free_response (&response);
}

/* h0 * prod(s - zero) / prod(s - pole) at s = i*w, with the roots on the real axis */
static void
pz_real_roots (double h0, const double *zeros, int nz, const double *poles, int np, double w,
               double *re, double *im)
{
  int i;
  double nr = 1.0, ni = 0.0, dr = 1.0, di = 0.0, t, m;

  for (i = 0; i < nz; ++i)
  {
    t = nr * (0.0 - zeros[i]) - ni * w;
    ni = nr * w + ni * (0.0 - zeros[i]);
    nr = t;
  }
  for (i = 0; i < np; ++i)
  {
    t = dr * (0.0 - poles[i]) - di * w;
    di = dr * w + di * (0.0 - poles[i]);
    dr = t;
  }
  m = dr * dr + di * di;
  *re = h0 * (nr * dr + ni * di) / m;
  *im = h0 * (ni * dr - nr * di) / m;
}

static void
check_pz_freqs (evalresp_channel *channel, evalresp_options *options, const double *freqs, int nfreqs,
                double h0, const double *zeros, int nz, const double *poles, int np, double gain)
{
  int i;
  double re, im, tol;
  evalresp_response *response = NULL;

  ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channel, options, freqs, nfreqs, &response));
  ER_CHECK_MSG (response->nfreqs == nfreqs, "nfreqs: %d", response->nfreqs);
  for (i = 0; i < nfreqs; ++i)
  {
    pz_real_roots (h0, zeros, nz, poles, np, 2.0 * M_PI * freqs[i], &re, &im);
    re *= gain;
    im *= gain;
    tol = 1e-9 * sqrt (re * re + im * im);
    ER_CHECK_MSG (fabs (response->rvec[i].real - re) <= tol, "%s %g Hz real: %g vs %g",
                  channel->chaname, freqs[i], response->rvec[i].real, re);
    ER_CHECK_MSG (fabs (response->rvec[i].imag - im) <= tol, "%s %g Hz imag: %g vs %g",
                  channel->chaname, freqs[i], response->rvec[i].imag, im);
  }
  evalresp_free_response (&response);
}

static void
test_zero_freq_pz (void)
{
  /* BDF: two zeros and one pole at zero, so the response vanishes at 0 Hz.
     BDG: one zero and one pole at zero cancel, leaving a finite response. */
  const double bdf_zeros[] = {0.0, 0.0, -0.17};
  const double bdf_poles[] = {0.0, -314.0, -0.188, -0.044};
  const double bdg_zeros[] = {0.0, -1.0};
  const double bdg_poles[] = {0.0, -2.0};
  const double freqs[] = {0.0, 0.1, 1.0, 10.0};
  const int nfreqs = sizeof (freqs) / sizeof (freqs[0]);
  int i;
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;
  evalresp_options *options = NULL;

  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->station_xml = 1;
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("station-pz-zero-freq.xml"), options, NULL,
                                            &channels));
  ER_CHECK (channels && channels->nchannels == 2);
  for (i = 0; i < channels->nchannels; ++i)
  {
    evalresp_channel *channel = channels->channels[i];

    if (!strcmp (channel->chaname, "BDF"))
    {
      ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channel, options, freqs, 1, &response));
      ER_CHECK_MSG (response->rvec[0].real == 0.0 && response->rvec[0].imag == 0.0,
                    "BDF 0 Hz: %g %g", response->rvec[0].real, response->rvec[0].imag);
      evalresp_free_response (&response);
      check_pz_freqs (channel, options, freqs + 1, nfreqs - 1, 314.1, bdf_zeros, 3, bdf_poles, 4,
                      0.4 * 611621.0);
    }
    else if (!strcmp (channel->chaname, "BDG"))
    {
      ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channel, options, freqs, 1, &response));
      ER_CHECK_MSG (response->rvec[0].real == 0.5 && response->rvec[0].imag == 0.0,
                    "BDG 0 Hz: %g %g", response->rvec[0].real, response->rvec[0].imag);
      evalresp_free_response (&response);
      check_pz_freqs (channel, options, freqs + 1, nfreqs - 1, 1.0, bdg_zeros, 2, bdg_poles, 2, 1.0);
    }
    else
    {
      ER_CHECK_MSG (0, "unexpected channel %s", channel->chaname);
    }
  }

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

static void
test_non_motion_units_not_converted (void)
{
  /* pressure input: displacement and acceleration requests leave the response as in file units */
  const double freqs[] = {0.1, 1.0, 10.0};
  const int nfreqs = sizeof (freqs) / sizeof (freqs[0]);
  const char *units[] = {"def", "dis", "acc"};
  evalresp_response *responses[3][2] = {{NULL}};
  int u, i, k;

  for (u = 0; u < 3; ++u)
  {
    evalresp_channels *channels = NULL;
    evalresp_options *options = NULL;

    ER_CHECK (!evalresp_new_options (NULL, &options));
    ER_CHECK (!evalresp_set_unit (NULL, options, units[u]));
    options->station_xml = 1;
    ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("station-pz-zero-freq.xml"), options, NULL,
                                              &channels));
    ER_CHECK (channels && channels->nchannels == 2);
    for (i = 0; i < channels->nchannels; ++i)
    {
      ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channels->channels[i], options, freqs, nfreqs,
                                                     &responses[u][i]));
    }
    evalresp_free_channels (&channels);
    evalresp_free_options (&options);
  }

  for (u = 1; u < 3; ++u)
  {
    for (i = 0; i < 2; ++i)
    {
      for (k = 0; k < nfreqs; ++k)
      {
        const evalresp_complex *a = &responses[0][i]->rvec[k], *b = &responses[u][i]->rvec[k];
        const double tol = 1e-9 * (fabs (a->real) + fabs (a->imag));

        ER_CHECK_MSG (fabs (a->real - b->real) <= tol && fabs (a->imag - b->imag) <= tol,
                      "-u %s channel %d %g Hz: %g %g vs %g %g", units[u], i, freqs[k],
                      b->real, b->imag, a->real, a->imag);
      }
    }
  }

  for (u = 0; u < 3; ++u)
    for (i = 0; i < 2; ++i)
      evalresp_free_response (&responses[u][i]);
}

const er_test unit_evaluation_tests[] = {
    {"zero_freq_pz", test_zero_freq_pz, NULL},
    {"non_motion_units_not_converted", test_non_motion_units_not_converted, NULL},
    {"arbitrary_freqs", test_arbitrary_freqs, NULL},
    {"legacy_arbitrary_freqs", test_legacy_arbitrary_freqs, NULL},
    {"no_options", test_no_options, NULL},
    {"start", test_start, NULL},
    {"freqs", test_freqs, NULL},
    {"b55_limits", test_b55_limits, NULL},
    {"gain_only", test_gain_only, NULL},
    {"log_no_filename", test_log_no_filename, NULL},
    {NULL, NULL, NULL},
};
