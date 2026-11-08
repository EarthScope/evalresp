/*
 * Malformed and hostile input must be rejected with an error, never crash,
 * overrun a buffer or take the process down.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public.h"
#include "harness/harness.h"

/* ---- helpers */

static char *
read_fixture (const char *name)
{
  FILE *fd;
  char *text;
  long n;

  ER_CHECK_MSG (NULL != (fd = fopen (er_data_path (name), "rb")), "cannot open %s", name);
  ER_CHECK (!fseek (fd, 0, SEEK_END));
  ER_CHECK ((n = ftell (fd)) >= 0);
  ER_CHECK (!fseek (fd, 0, SEEK_SET));
  ER_CHECK (NULL != (text = malloc (n + 1)));
  ER_CHECK ((long)fread (text, 1, n, fd) == n);
  text[n] = '\0';
  fclose (fd);
  return text;
}

/* a copy of text with the first from replaced by to */
static char *
replace_once (const char *text, const char *from, const char *to)
{
  const char *at = strstr (text, from);
  char *out;

  ER_CHECK_MSG (at, "'%s' is not in the text", from);
  ER_CHECK (NULL != (out = malloc (strlen (text) + strlen (to) + 1)));
  memcpy (out, text, at - text);
  strcpy (out + (at - text), to);
  strcpy (out + (at - text) + strlen (to), at + strlen (from));
  return out;
}

static int
load_resp (const char *text, evalresp_channels **channels)
{
  *channels = NULL;
  return evalresp_char_to_channels (NULL, text, NULL, NULL, channels);
}

static int
load_xml (const char *text, evalresp_channels **channels)
{
  evalresp_options *options = NULL;
  int status;

  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->station_xml = 1;
  *channels = NULL;
  status = evalresp_char_to_channels (NULL, text, options, NULL, channels);
  evalresp_free_options (&options);
  return status;
}

/* ---- RESP input */

static void
test_resp_long_field (void)
{
  char station[128], line[160];
  char *text, *bad;
  evalresp_channels *channels;

  text = read_fixture ("RESP.IU.ANMO.00.BHZ");
  memset (station, 'A', 100);
  station[100] = '\0';
  snprintf (line, sizeof (line), "Station:     %s", station);
  bad = replace_once (text, "Station:     ANMO", line);
  ER_CHECK (load_resp (bad, &channels) != EVALRESP_OK);
  evalresp_free_channels (&channels);
  free (bad);
  free (text);
}

static void
test_resp_long_denominator_field (void)
{
  char value[400], line[480];
  char *text, *bad;
  evalresp_channels *channels;

  text = read_fixture ("RESP.IU.ANMO.00.BHZ");
  memset (value, '9', 300);
  value[300] = '\0';
  snprintf (line, sizeof (line), "Number of denominators:%s", value);
  bad = replace_once (text, "Number of denominators:                0", line);
  ER_CHECK (load_resp (bad, &channels) != EVALRESP_OK);
  evalresp_free_channels (&channels);
  free (bad);
  free (text);
}

static void
test_resp_dates_terminated (void)
{
  char *text, *bad, *bad2;
  evalresp_channels *channels;

  /* 23 or more characters, which fills the date buffer; the first epoch of the file only */
  text = read_fixture ("RESP.IU.ANMO.00.BHZ");
  *strstr (strstr (text, "B050F03") + 1, "B050F03") = '\0';
  bad = replace_once (text, "Start date:  1998,299,20", "Start date:  1998,299,20:00:00          ");
  bad2 = replace_once (bad, "End date:    2000,293,16", "End date:    2000,293,16:00:00          ");
  ER_CHECK (!load_resp (bad2, &channels));
  ER_CHECK (channels && channels->nchannels == 1);
  ER_CHECK_MSG (!strcmp (channels->channels[0]->beg_t, "1998,299,20:00:00"), "beg_t: '%s'", channels->channels[0]->beg_t);
  ER_CHECK_MSG (!strcmp (channels->channels[0]->end_t, "2000,293,16:00:00"), "end_t: '%s'", channels->channels[0]->end_t);
  evalresp_free_channels (&channels);
  free (bad2);
  free (bad);
  free (text);
}

static void
test_resp_bad_counts (void)
{
  const char *counts[] = {"-1", "2000000000", "99999999999"};
  char line[128];
  char *text, *bad;
  evalresp_channels *channels;
  size_t i;

  text = read_fixture ("RESP.IU.ANMO.00.BHZ");
  for (i = 0; i < sizeof (counts) / sizeof (counts[0]); ++i)
  {
    snprintf (line, sizeof (line), "Number of zeroes:                      %s", counts[i]);
    bad = replace_once (text, "Number of zeroes:                      2", line);
    ER_CHECK_MSG (load_resp (bad, &channels) != EVALRESP_OK, "zero count %s accepted", counts[i]);
    evalresp_free_channels (&channels);
    free (bad);
  }
  free (text);
}

static void
test_resp_truncated (void)
{
  char *text;
  evalresp_channels *channels;

  /* the input ends inside the zeros of the first stage */
  text = read_fixture ("RESP.IU.ANMO.00.BHZ");
  *strstr (text, "B053F10-13    1") = '\0';
  ER_CHECK (load_resp (text, &channels) != EVALRESP_OK);
  evalresp_free_channels (&channels);
  free (text);
}

/* ---- StationXML input */

static const char xml_head[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<FDSNStationXML xmlns=\"http://www.fdsn.org/xml/station/1\" schemaVersion=\"1.0\">"
    "<Source>T</Source><Network code=\"XX\"><Station code=\"STA\">"
    "<Latitude>0</Latitude><Longitude>0</Longitude><Elevation>0</Elevation><Site><Name>S</Name></Site>"
    "<Channel code=\"BHZ\" locationCode=\"\" startDate=\"2000-01-01T00:00:00\">"
    "<Latitude>0</Latitude><Longitude>0</Longitude><Elevation>0</Elevation>"
    "<Depth>0</Depth><Azimuth>0</Azimuth><Dip>0</Dip><SampleRate>100</SampleRate><Response>";
static const char xml_tail[] = "</Response></Channel></Station></Network></FDSNStationXML>\n";

#define UNITS_VOLTS                           \
  "<InputUnits><Name>M/S</Name></InputUnits>" \
  "<OutputUnits><Name>V</Name></OutputUnits>"
#define UNITS_COUNTS                             \
  "<InputUnits><Name>COUNTS</Name></InputUnits>" \
  "<OutputUnits><Name>COUNTS</Name></OutputUnits>"
#define DECIMATION                                                       \
  "<Decimation><InputSampleRate>100</InputSampleRate><Factor>1</Factor>" \
  "<Offset>0</Offset><Delay>0</Delay><Correction>0</Correction></Decimation>"
#define GAIN(value) "<StageGain><Value>" value "</Value><Frequency>1</Frequency></StageGain>"

static char *
xml_document (const char *response)
{
  char *text = malloc (sizeof (xml_head) + strlen (response) + sizeof (xml_tail));

  ER_CHECK (text != NULL);
  strcpy (text, xml_head);
  strcat (text, response);
  strcat (text, xml_tail);
  return text;
}

static int
load_xml_response (const char *response, evalresp_channels **channels)
{
  char *text = xml_document (response);
  int status = load_xml (text, channels);

  free (text);
  return status;
}

static void
check_xml_rejected (const char *what, const char *response)
{
  evalresp_channels *channels;

  ER_CHECK_MSG (load_xml_response (response, &channels) != EVALRESP_OK, "%s was accepted", what);
  evalresp_free_channels (&channels);
}

#define PZ_STAGE(pole_real, gain)                                                                  \
  "<Stage number=\"1\"><PolesZeros>" UNITS_VOLTS                                                   \
  "<PzTransferFunctionType>LAPLACE (RADIANS/SECOND)</PzTransferFunctionType>"                      \
  "<NormalizationFactor>1</NormalizationFactor><NormalizationFrequency>1</NormalizationFrequency>" \
  "<Zero number=\"0\"><Real>0</Real><Imaginary>0</Imaginary></Zero>"                               \
  "<Pole number=\"0\"><Real>" pole_real "</Real><Imaginary>0</Imaginary></Pole>"                   \
  "</PolesZeros>" GAIN (gain) "</Stage>"

static void
test_xml_control (void)
{
  evalresp_channels *channels;

  /* the documents used below are valid apart from the one defect in each */
  ER_CHECK (!load_xml_response (PZ_STAGE ("-1", "1"), &channels));
  ER_CHECK (channels && channels->nchannels == 1);
  evalresp_free_channels (&channels);
}

static void
test_xml_bad_numbers (void)
{
  check_xml_rejected ("blank pole", PZ_STAGE (" ", "1"));
  check_xml_rejected ("NaN gain", PZ_STAGE ("-1", "nan"));
  check_xml_rejected ("infinite pole", PZ_STAGE ("inf", "1"));
  check_xml_rejected ("text gain", PZ_STAGE ("-1", "abc"));
  check_xml_rejected ("blank stage number",
                      "<Stage number=\"\"><PolesZeros>" UNITS_VOLTS
                      "<PzTransferFunctionType>LAPLACE (RADIANS/SECOND)</PzTransferFunctionType>"
                      "<NormalizationFactor>1</NormalizationFactor><NormalizationFrequency>1</NormalizationFrequency>"
                      "</PolesZeros>" GAIN ("1") "</Stage>");
}

static void
test_xml_coefficients_no_numerator (void)
{
  check_xml_rejected ("denominator without numerator",
                      "<Stage number=\"1\"><Coefficients>" UNITS_COUNTS
                      "<CfTransferFunctionType>DIGITAL</CfTransferFunctionType>"
                      "<Denominator>1.0</Denominator></Coefficients>" DECIMATION GAIN ("1") "</Stage>");
}

static void
test_xml_empty_list (void)
{
  check_xml_rejected ("empty response list",
                      "<Stage number=\"1\"><ResponseList>" UNITS_VOLTS "</ResponseList>" GAIN ("1") "</Stage>");
}

static void
test_xml_filters_one_stage (void)
{
  /* a stage number used twice joins the stages, which cannot hold two filters */
  check_xml_rejected ("pole-zero and list in one stage",
                      PZ_STAGE ("-1", "1") "<Stage number=\"1\"><ResponseList>" UNITS_VOLTS
                                           "<ResponseListElement><Frequency>1</Frequency><Amplitude>1</Amplitude><Phase>0</Phase></ResponseListElement>"
                                           "</ResponseList>" GAIN ("1") "</Stage>");
}

#define LIST_STAGE(rows) "<Stage number=\"1\"><ResponseList>" UNITS_VOLTS rows "</ResponseList>" GAIN ("1") "</Stage>"
#define LIST_ROW(f, a) "<ResponseListElement><Frequency>" f "</Frequency><Amplitude>" a "</Amplitude><Phase>0</Phase></ResponseListElement>"

/* Interpolating a list that cannot be interpolated is an error, not an exit. */
static void
check_list_not_interpolated (const char *what, const char *response, const double *freqs, int nfreqs)
{
  evalresp_channels *channels;
  evalresp_options *options = NULL;
  evalresp_response *response_out = NULL;
  int status;

  ER_CHECK_MSG (!load_xml_response (response, &channels), "%s: cannot load", what);
  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->station_xml = 1;
  options->b55_interpolate = 1;
  status = evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, freqs, nfreqs, &response_out);
  ER_CHECK_MSG (status == EVALRESP_INP, "%s: status %d", what, status);
  ER_CHECK (!response_out);
  evalresp_free_options (&options);
  evalresp_free_channels (&channels);
}

static void
test_list_interpolation_rejected (void)
{
  const double freqs[] = {1.0, 5.0, 10.0};

  check_list_not_interpolated ("repeated frequency",
                               LIST_STAGE (LIST_ROW ("1", "1") LIST_ROW ("1", "2") LIST_ROW ("10", "3")), freqs, 3);
  check_list_not_interpolated ("decreasing frequency",
                               LIST_STAGE (LIST_ROW ("1", "1") LIST_ROW ("10", "2") LIST_ROW ("5", "3")), freqs, 3);
  check_list_not_interpolated ("single row", LIST_STAGE (LIST_ROW ("1", "1")), freqs, 1);
}

static void
test_zero_sum_fir (void)
{
  const double freqs[] = {0.5, 1.0, 2.0};
  evalresp_channels *channels;
  evalresp_options *options = NULL;
  evalresp_response *response = NULL;
  int i;

  /* coefficients that sum to zero are not normalized */
  ER_CHECK (!load_xml_response ("<Stage number=\"1\"><Coefficients>" UNITS_COUNTS
                                "<CfTransferFunctionType>DIGITAL</CfTransferFunctionType>"
                                "<Numerator>1.0</Numerator><Numerator>-1.0</Numerator></Coefficients>" DECIMATION GAIN ("1") "</Stage>",
                                &channels));
  ER_CHECK (!evalresp_new_options (NULL, &options));
  ER_CHECK (!evalresp_set_unit (NULL, options, "def"));
  options->station_xml = 1;
  ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, freqs, 3, &response));
  for (i = 0; i < response->nfreqs; ++i)
  {
    ER_CHECK_MSG (isfinite (response->rvec[i].real) && isfinite (response->rvec[i].imag), "row %d: %g %g", i,
                  response->rvec[i].real, response->rvec[i].imag);
  }
  evalresp_free_response (&response);
  evalresp_free_options (&options);
  evalresp_free_channels (&channels);
}

/* ---- evaluation */

static void
test_infinite_response_units (void)
{
  const double freqs[] = {0.0, 1.0};
  evalresp_channels *channels = NULL;
  evalresp_options *options = NULL;
  evalresp_response *response = NULL;

  /* a velocity sensor with a pole at the origin is infinite at 0 Hz, whatever the units requested */
  ER_CHECK (!load_xml_response ("<Stage number=\"1\"><PolesZeros>" UNITS_VOLTS
                                "<PzTransferFunctionType>LAPLACE (RADIANS/SECOND)</PzTransferFunctionType>"
                                "<NormalizationFactor>1</NormalizationFactor><NormalizationFrequency>1</NormalizationFrequency>"
                                "<Pole number=\"0\"><Real>0</Real><Imaginary>0</Imaginary></Pole>"
                                "</PolesZeros>" GAIN ("1") "</Stage>",
                                &channels));
  ER_CHECK (!evalresp_new_options (NULL, &options));
  options->station_xml = 1;
  options->unit = evalresp_displacement_unit;
  ER_CHECK (!evalresp_channel_to_response_freqs (NULL, channels->channels[0], options, freqs, 2, &response));
  ER_CHECK_MSG (!isnan (response->rvec[0].real) && !isnan (response->rvec[0].imag), "0 Hz: %g %g", response->rvec[0].real,
                response->rvec[0].imag);
  ER_CHECK_MSG (isfinite (response->rvec[1].real) && isfinite (response->rvec[1].imag), "1 Hz: %g %g",
                response->rvec[1].real, response->rvec[1].imag);
  evalresp_free_response (&response);
  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
}

/* ---- stage chains */

static void
test_free_many_stages (void)
{
  evalresp_stage *head, *last, *stage;
  int i;

  /* freeing a response must not recurse once per stage */
  ER_CHECK (NULL != (head = alloc_stage (NULL)));
  last = head;
  for (i = 1; i < 400000; ++i)
  {
    ER_CHECK (NULL != (stage = alloc_stage (NULL)));
    last->next_stage = stage;
    last = stage;
  }
  free_stages (head);
}

/* ---- legacy interface */

static void
test_evresp_1_mismatched_freqs (void)
{
  /* the list in this response sets the frequencies, which are not the ones requested */
  double freqs[100], resp[200];
  int i, rc;

  for (i = 0; i < 100; ++i)
  {
    freqs[i] = 0.001 * pow (10000.0, i / 99.0);
  }
  rc = evresp_1 ("ATTU", "BHE", "IM", "", "1998,57,00:00:00", "VEL", (char *)er_data_path ("RESP.IM.ATTU..BHE"), freqs,
                 100, resp, "AP", "", 0, 99, 0, 0, 0.0, 0);
  ER_CHECK_MSG (rc == 2, "evresp_1 returned %d", rc);
}

static void
test_evresp_bad_arguments (void)
{
  char date[400];
  double freqs[] = {1.0, 2.0};
  const char *file = er_data_path ("RESP.IU.ANMO..BHZ");
  evalresp_response *response;

  /* the time of day is far longer than any buffer for it */
  memset (date, '0', sizeof (date));
  memcpy (date, "2015,1,", 7);
  date[sizeof (date) - 1] = '\0';
  response = evresp ("ANMO", "BHZ", "IU", "", date, "VEL", (char *)file, freqs, 2, "AP", "", 0, 99, 0, 0, 0.0, 0);
  evalresp_free_response (&response);

  ER_CHECK (NULL == evresp ("ANMO", "BHZ", "IU", "", NULL, "VEL", (char *)file, freqs, 2, "AP", "", 0, 99, 0, 0, 0.0, 0));
  ER_CHECK (NULL == evresp ("ANMO", "BHZ", "IU", "", "2015,1,00:00:00", "VEL", (char *)file, freqs, 0, "AP", "", 0, 99, 0,
                            0, 0.0, 0));
  ER_CHECK (NULL == evresp ("ANMO", "BHZ", "IU", "", "2015,1,00:00:00", "VEL", (char *)file, NULL, 2, "AP", "", 0, 99, 0,
                            0, 0.0, 0));
}

static void
test_print_chan_without_file (void)
{
  evalresp_channels *channels = NULL;

  /* curr_file is never set by the library */
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, NULL, &channels));
  print_chan (channels->channels[0], -1, 0, 0, 0, 0, 0);
  evalresp_free_channels (&channels);
}

const er_test unit_robust_tests[] = {
    {"resp_long_field", test_resp_long_field, NULL},
    {"resp_long_denominator_field", test_resp_long_denominator_field, NULL},
    {"resp_dates_terminated", test_resp_dates_terminated, NULL},
    {"resp_bad_counts", test_resp_bad_counts, NULL},
    {"resp_truncated", test_resp_truncated, NULL},
    {"xml_control", test_xml_control, NULL},
    {"xml_bad_numbers", test_xml_bad_numbers, NULL},
    {"xml_coefficients_no_numerator", test_xml_coefficients_no_numerator, NULL},
    {"xml_empty_list", test_xml_empty_list, NULL},
    {"xml_filters_one_stage", test_xml_filters_one_stage, NULL},
    {"list_interpolation_rejected", test_list_interpolation_rejected, NULL},
    {"zero_sum_fir", test_zero_sum_fir, NULL},
    {"infinite_response_units", test_infinite_response_units, NULL},
    {"free_many_stages", test_free_many_stages, NULL},
    {"evresp_1_mismatched_freqs", test_evresp_1_mismatched_freqs, NULL},
    {"evresp_bad_arguments", test_evresp_bad_arguments, NULL},
    {"print_chan_without_file", test_print_chan_without_file, NULL},
    {NULL, NULL, NULL},
};
