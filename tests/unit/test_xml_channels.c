#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/input.h"
#include "evalresp/private.h"
#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "harness/harness.h"

/* the RESP text is rounded to 5 or 6 significant digits */
#define REL_TOL 1e-4

static char where[256];

static void
check_near (const char *what, double a, double b)
{
  double scale = fabs (a) > fabs (b) ? fabs (a) : fabs (b);
  if (!(fabs (a - b) <= REL_TOL * scale))
    ER_FAIL ("%s %s: %.12g differs from %.12g", where, what, a, b);
}

static void
check_all_near (const char *what, int n, const double *a, const double *b)
{
  int i;
  for (i = 0; i < n; ++i)
    check_near (what, a[i], b[i]);
}

static void
check_int (const char *what, int a, int b)
{
  if (a != b)
    ER_FAIL ("%s %s: %d differs from %d", where, what, a, b);
}

static void
check_text (const char *what, const char *a, const char *b)
{
  if (a == NULL || b == NULL)
  {
    if (a != b)
      ER_FAIL ("%s %s: one is missing", where, what);
  }
  else if (strcmp (a, b))
    ER_FAIL ("%s %s: '%s' differs from '%s'", where, what, a, b);
}

static void
compare_blkt (const evalresp_blkt *a, const evalresp_blkt *b)
{
  check_int ("blockette type", a->type, b->type);
  switch (a->type)
  {
  case LAPLACE_PZ:
  case ANALOG_PZ:
  case IIR_PZ:
  {
    const evalresp_pole_zero *x = &a->blkt_info.pole_zero, *y = &b->blkt_info.pole_zero;
    int i;
    check_int ("nzeros", x->nzeros, y->nzeros);
    check_int ("npoles", x->npoles, y->npoles);
    check_near ("a0", x->a0, y->a0);
    check_near ("a0_freq", x->a0_freq, y->a0_freq);
    for (i = 0; i < x->nzeros; ++i)
    {
      check_near ("zero real", x->zeros[i].real, y->zeros[i].real);
      check_near ("zero imag", x->zeros[i].imag, y->zeros[i].imag);
    }
    for (i = 0; i < x->npoles; ++i)
    {
      check_near ("pole real", x->poles[i].real, y->poles[i].real);
      check_near ("pole imag", x->poles[i].imag, y->poles[i].imag);
    }
    break;
  }
  case IIR_COEFFS:
  {
    const evalresp_coeff *x = &a->blkt_info.coeff, *y = &b->blkt_info.coeff;
    check_int ("nnumer", x->nnumer, y->nnumer);
    check_int ("ndenom", x->ndenom, y->ndenom);
    check_all_near ("numer", x->nnumer, x->numer, y->numer);
    check_all_near ("denom", x->ndenom, x->denom, y->denom);
    check_near ("h0", x->h0, y->h0);
    break;
  }
  case FIR_ASYM:
  case FIR_SYM_1:
  case FIR_SYM_2:
  {
    const evalresp_fir *x = &a->blkt_info.fir, *y = &b->blkt_info.fir;
    check_int ("ncoeffs", x->ncoeffs, y->ncoeffs);
    check_all_near ("coeffs", x->ncoeffs, x->coeffs, y->coeffs);
    check_near ("h0", x->h0, y->h0);
    break;
  }
  case LIST:
  {
    const evalresp_list *x = &a->blkt_info.list, *y = &b->blkt_info.list;
    check_int ("nresp", x->nresp, y->nresp);
    check_all_near ("freq", x->nresp, x->freq, y->freq);
    check_all_near ("amp", x->nresp, x->amp, y->amp);
    check_all_near ("phase", x->nresp, x->phase, y->phase);
    break;
  }
  case DECIMATION:
  {
    const evalresp_decimation *x = &a->blkt_info.decimation, *y = &b->blkt_info.decimation;
    check_near ("sample_int", x->sample_int, y->sample_int);
    check_int ("deci_fact", x->deci_fact, y->deci_fact);
    check_int ("deci_offset", x->deci_offset, y->deci_offset);
    check_near ("estim_delay", x->estim_delay, y->estim_delay);
    check_near ("applied_corr", x->applied_corr, y->applied_corr);
    break;
  }
  case GAIN:
    check_near ("gain", a->blkt_info.gain.gain, b->blkt_info.gain.gain);
    check_near ("gain_freq", a->blkt_info.gain.gain_freq, b->blkt_info.gain.gain_freq);
    break;
  case POLYNOMIAL:
  {
    const evalresp_polynomial *x = &a->blkt_info.polynomial, *y = &b->blkt_info.polynomial;
    check_int ("approximation_type", x->approximation_type, y->approximation_type);
    check_int ("frequency_units", x->frequency_units, y->frequency_units);
    check_near ("lower_freq_bound", x->lower_freq_bound, y->lower_freq_bound);
    check_near ("upper_freq_bound", x->upper_freq_bound, y->upper_freq_bound);
    check_near ("lower_approx_bound", x->lower_approx_bound, y->lower_approx_bound);
    check_near ("upper_approx_bound", x->upper_approx_bound, y->upper_approx_bound);
    check_near ("max_abs_error", x->max_abs_error, y->max_abs_error);
    check_int ("ncoeffs", x->ncoeffs, y->ncoeffs);
    check_all_near ("coeffs", x->ncoeffs, x->coeffs, y->coeffs);
    check_all_near ("coeffs_err", x->ncoeffs, x->coeffs_err, y->coeffs_err);
    break;
  }
  default:
    break;
  }
}

static void
compare_stage (const evalresp_stage *a, const evalresp_stage *b)
{
  const evalresp_blkt *x, *y;
  check_int ("sequence_no", a->sequence_no, b->sequence_no);
  check_int ("input_units", a->input_units, b->input_units);
  check_int ("output_units", a->output_units, b->output_units);
  check_text ("input_units_str", a->input_units_str, b->input_units_str);
  check_text ("output_units_str", a->output_units_str, b->output_units_str);
  for (x = a->first_blkt, y = b->first_blkt; x && y; x = x->next_blkt, y = y->next_blkt)
    compare_blkt (x, y);
  if (x || y)
    ER_FAIL ("%s: different numbers of blockettes", where);
}

static void
compare_channel (const evalresp_channel *a, const evalresp_channel *b)
{
  const evalresp_stage *x, *y;
  check_text ("staname", a->staname, b->staname);
  check_text ("network", a->network, b->network);
  check_text ("locid", a->locid, b->locid);
  check_text ("chaname", a->chaname, b->chaname);
  check_text ("beg_t", a->beg_t, b->beg_t);
  check_text ("end_t", a->end_t, b->end_t);
  check_text ("first_units", a->first_units, b->first_units);
  check_text ("last_units", a->last_units, b->last_units);
  check_near ("unit_scale_fact", a->unit_scale_fact, b->unit_scale_fact);
  check_near ("sensit", a->sensit, b->sensit);
  check_near ("sensfreq", a->sensfreq, b->sensfreq);
  check_near ("estim_delay", a->estim_delay, b->estim_delay);
  check_near ("applied_corr", a->applied_corr, b->applied_corr);
  check_near ("sint", a->sint, b->sint);
  check_int ("nstages", a->nstages, b->nstages);
  for (x = a->first_stage, y = b->first_stage; x && y; x = x->next_stage, y = y->next_stage)
  {
    snprintf (where + strlen (where), sizeof (where) - strlen (where), " stage %d:", x->sequence_no);
    compare_stage (x, y);
  }
  if (x || y)
    ER_FAIL ("%s: different numbers of stages", where);
}

/* the channels of the document, read directly and via the RESP text */
static void
compare_routes (const char *xml_path)
{
  evalresp_options *direct_options = NULL, *resp_options = NULL;
  evalresp_channels *direct = NULL, *via_resp = NULL;
  char *xml = NULL, *resp = NULL;
  FILE *in;
  int i;

  ER_CHECK_MSG (NULL != (in = fopen (xml_path, "rb")), "%s", xml_path);
  ER_CHECK (EVALRESP_OK == file_to_char (NULL, in, &xml));
  fclose (in);

  ER_CHECK (EVALRESP_OK == evalresp_new_options (NULL, &direct_options));
  direct_options->station_xml = 1;
  ER_CHECK (EVALRESP_OK == evalresp_char_to_channels (NULL, xml, direct_options, NULL, &direct));

  ER_CHECK (EVALRESP_OK == evalresp_xml_to_char (NULL, 1, xml, &resp));
  ER_CHECK (EVALRESP_OK == evalresp_new_options (NULL, &resp_options));
  ER_CHECK (EVALRESP_OK == evalresp_char_to_channels (NULL, resp, resp_options, NULL, &via_resp));

  ER_CHECK (direct->nchannels > 0);
  ER_CHECK_MSG (direct->nchannels == via_resp->nchannels, "%d channels directly but %d via RESP",
                direct->nchannels, via_resp->nchannels);
  for (i = 0; i < direct->nchannels; ++i)
  {
    snprintf (where, sizeof (where), "%s channel %d:", xml_path, i);
    compare_channel (direct->channels[i], via_resp->channels[i]);
  }

  evalresp_free_channels (&direct);
  evalresp_free_channels (&via_resp);
  evalresp_free_options (&direct_options);
  evalresp_free_options (&resp_options);
  free (xml);
  free (resp);
}

static void
test_same_as_resp_1 (void)
{
  compare_routes (er_data_path ("station-1.xml"));
}

static void
test_same_as_resp_2 (void)
{
  compare_routes (er_data_path ("station-2.xml"));
}

static void
test_same_as_resp_3 (void)
{
  compare_routes (er_data_path ("station-3.xml"));
}

static void
test_same_as_resp_list (void)
{
  compare_routes (er_data_path ("station-list.xml"));
}

static const evalresp_blkt *
find_blkt (const evalresp_stage *stage, int type)
{
  const evalresp_blkt *blkt;
  for (blkt = stage->first_blkt; blkt; blkt = blkt->next_blkt)
    if (blkt->type == type)
      return blkt;
  ER_FAIL ("no blockette of type %d in stage %d", type, stage->sequence_no);
}

#define CHECK_EXACT(value, text) ER_CHECK_MSG ((value) == strtod ((text), NULL), "%s = %.17g, expected %s", #value, (value), (text))

/* values keep every digit of the StationXML */
static void
test_full_precision (void)
{
  evalresp_options *options = NULL;
  evalresp_channels *channels = NULL;
  const evalresp_channel *channel;
  const evalresp_stage *stage;
  const evalresp_blkt *blkt;

  ER_CHECK (EVALRESP_OK == evalresp_new_options (NULL, &options));
  ER_CHECK (EVALRESP_OK == evalresp_filename_to_channels (NULL, er_data_path ("station-precision.xml"), options, NULL, &channels));
  ER_CHECK (1 == channels->nchannels);
  channel = channels->channels[0];
  ER_CHECK (!strcmp (channel->staname, "PREC"));
  ER_CHECK (!strcmp (channel->network, "XX"));
  ER_CHECK (!strcmp (channel->locid, ""));
  ER_CHECK (!strcmp (channel->chaname, "BHZ"));
  ER_CHECK (!strcmp (channel->beg_t, "2013,311,17:00:00"));
  ER_CHECK (!strcmp (channel->end_t, "No Ending Time"));
  CHECK_EXACT (channel->sensit, "2500.12345678901");
  CHECK_EXACT (channel->sensfreq, "0.123456789012345");
  CHECK_EXACT (channel->estim_delay, "0.0123456789012345");
  CHECK_EXACT (channel->applied_corr, "0.0234567890123456");
  ER_CHECK (3 == channel->nstages);

  /* stage 0, then 1 and 2 as in the document */
  stage = channel->first_stage;
  for (; stage && stage->sequence_no != 1; stage = stage->next_stage)
    ;
  ER_CHECK (stage);
  blkt = find_blkt (stage, LAPLACE_PZ);
  CHECK_EXACT (blkt->blkt_info.pole_zero.a0, "1.23456789012345");
  CHECK_EXACT (blkt->blkt_info.pole_zero.a0_freq, "0.123456789012345");
  ER_CHECK (1 == blkt->blkt_info.pole_zero.nzeros);
  ER_CHECK (2 == blkt->blkt_info.pole_zero.npoles);
  CHECK_EXACT (blkt->blkt_info.pole_zero.zeros[0].real, "-0.00123456789012345");
  CHECK_EXACT (blkt->blkt_info.pole_zero.poles[0].real, "-0.0370239123456789");
  CHECK_EXACT (blkt->blkt_info.pole_zero.poles[0].imag, "0.0370239123456789");
  CHECK_EXACT (blkt->blkt_info.pole_zero.poles[1].imag, "-0.0370239123456789");
  blkt = find_blkt (stage, GAIN);
  CHECK_EXACT (blkt->blkt_info.gain.gain, "1500.12345678901");

  stage = stage->next_stage;
  ER_CHECK (stage && 2 == stage->sequence_no);
  blkt = find_blkt (stage, FIR_ASYM);
  ER_CHECK (4 == blkt->blkt_info.fir.ncoeffs);
  CHECK_EXACT (blkt->blkt_info.fir.coeffs[0], "0.123456789012345");
  CHECK_EXACT (blkt->blkt_info.fir.coeffs[1], "0.376543210987655");
  blkt = find_blkt (stage, DECIMATION);
  ER_CHECK (blkt->blkt_info.decimation.sample_int == 1.0 / strtod ("0.333333333333333", NULL));
  CHECK_EXACT (blkt->blkt_info.decimation.estim_delay, "0.0123456789012345");
  CHECK_EXACT (blkt->blkt_info.decimation.applied_corr, "0.0234567890123456");

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

/* StationXML text is detected and read without a file */
static void
test_char_detected (void)
{
  evalresp_options *options = NULL;
  evalresp_channels *channels = NULL;
  const evalresp_blkt *blkt;
  char *xml = NULL;
  FILE *in;

  ER_CHECK_MSG (NULL != (in = fopen (er_data_path ("station-list.xml"), "rb")), "station-list.xml");
  ER_CHECK (EVALRESP_OK == file_to_char (NULL, in, &xml));
  fclose (in);

  ER_CHECK (EVALRESP_OK == evalresp_new_options (NULL, &options));
  ER_CHECK (EVALRESP_OK == evalresp_char_to_channels (NULL, xml, options, NULL, &channels));
  ER_CHECK (1 == channels->nchannels);
  ER_CHECK (!strcmp (channels->channels[0]->chaname, "EHZ"));
  blkt = find_blkt (channels->channels[0]->first_stage, LIST);
  ER_CHECK (3 == blkt->blkt_info.list.nresp);
  ER_CHECK (1.0 == blkt->blkt_info.list.freq[1]);
  ER_CHECK (2.5 == blkt->blkt_info.list.amp[1]);
  ER_CHECK (20.0 == blkt->blkt_info.list.phase[1]);

  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
  free (xml);
}

/* RESP text is an error when StationXML is expected */
static void
test_forced_xml_rejects_resp (void)
{
  evalresp_options *options = NULL;
  evalresp_channels *channels = NULL;

  ER_CHECK (EVALRESP_OK == evalresp_new_options (NULL, &options));
  options->station_xml = 1;
  ER_CHECK (EVALRESP_OK != evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), options, NULL, &channels));
  ER_CHECK (NULL == channels);

  evalresp_free_options (&options);
}

const er_test unit_xml_channels_tests[] = {
    {"same_as_resp_1", test_same_as_resp_1, NULL},
    {"same_as_resp_2", test_same_as_resp_2, NULL},
    {"same_as_resp_3", test_same_as_resp_3, NULL},
    {"same_as_resp_list", test_same_as_resp_list, NULL},
    {"full_precision", test_full_precision, NULL},
    {"char_detected", test_char_detected, NULL},
    {"forced_xml_rejects_resp", test_forced_xml_rejects_resp, NULL},
    {NULL, NULL, NULL},
};
