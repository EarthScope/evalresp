#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/constants.h"
#include "evalresp/input.h"
#include "evalresp/private.h"
#include "evalresp/public_api.h"
#include "harness/harness.h"

static void
test_slurp_line (void)
{
  const char *input = "line 1\nline 2\nline 3";
  const char *start = input;
  char line[MAXLINELEN];
  slurp_line (&start, line, MAXLINELEN);
  ER_CHECK_MSG (!strcmp (line, "line 1\n"), "Slurped [%s] (size %d)", line, (int)strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  ER_CHECK_MSG (!strcmp (line, "line 2\n"), "Slurped [%s] (size %d)", line, (int)strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  ER_CHECK_MSG (!strcmp (line, "line 3"), "Slurped [%s] (size %d)", line, (int)strlen (line));
  ER_CHECK_MSG (!(*start), "Start is [%s]", start);
  slurp_line (&start, line, MAXLINELEN);
  ER_CHECK_MSG (!strcmp (line, ""), "Slurped [%s] (size %d)", line, (int)strlen (line));
  ER_CHECK_MSG (!(*start), "Start is [%s]", start);
}

static void
test_slurp_line_buffer (void)
{
  const char *input = "\n1\n12\n123\n";
  const char *start = input;
  char line[3];
  slurp_line (&start, line, 3);
  ER_CHECK_MSG (!strcmp (line, "\n"), "Slurped [%s] (size %d)", line, (int)strlen (line));
  slurp_line (&start, line, 3);
  ER_CHECK_MSG (!strcmp (line, "1\n"), "Slurped [%s] (size %d)", line, (int)strlen (line));
  slurp_line (&start, line, 3);
  ER_CHECK_MSG (!strcmp (line, "12"), "Slurped [%s] (size %d)", line, (int)strlen (line));
}

static void
test_find_line (void)
{
  const char *input = "B999F99 name1: value1 \nB666F66 name2: value2", *start;
  char line[MAXLINELEN];
  start = input;
  ER_CHECK (!find_line (NULL, &start, ":", 999, 99, line));
  ER_CHECK_MSG (!strcmp (line, "value1 "), "'%s'", line);
  ER_CHECK (!find_line (NULL, &start, ":", 666, 66, line));
  ER_CHECK_MSG (!strcmp (line, "value2"), "'%s'", line);
  start = input;
  ER_CHECK (!find_line (NULL, &start, ":", 666, 66, line));
  ER_CHECK_MSG (!strcmp (line, "value2"), "'%s'", line);
}

static void
test_find_field (void)
{
  const char *input = "B999F99 name1: a b c \nB666F66 name2: value2", *start;
  char field[MAXLINELEN];
  start = input;
  ER_CHECK (!find_field (NULL, &start, ":", 999, 99, 0, field));
  ER_CHECK_MSG (!strcmp (field, "a"), "'%s'", field);
  ER_CHECK (!find_field (NULL, &start, ":", 666, 66, 0, field));
  ER_CHECK_MSG (!strcmp (field, "value2"), "'%s'", field);
  start = input;
  ER_CHECK (!find_field (NULL, &start, ":", 999, 99, 2, field));
  ER_CHECK_MSG (!strcmp (field, "c"), "'%s'", field);
}

static void
test_file_to_char (void)
{
  char *seed = NULL;
  FILE *in = NULL;
  ER_CHECK (!open_file (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), &in));
  ER_CHECK (!file_to_char (NULL, in, &seed));
  int len = strlen (seed);
  ER_CHECK_MSG (!(len != 192590), "wrong length: %d", len); // value checks with wc
  char start[40];
  int i;
  for (i = 0; i < 39; ++i)
    start[i] = seed[i];
  start[39] = '\0';
  ER_CHECK_MSG (!strncmp (seed, "#\t\t<< IRIS SEED Reader, Release 4.16 >>", 39), "Bad start: '%s'", start);
  char end[41];
  for (i = 0; i < 40; ++i)
    end[i] = seed[i + 192590 - 40];
  end[40] = '\0';
  ER_CHECK_MSG (!strncmp (seed + 192590 - 40, "of calibrations:                0\r\n#\t\t\r\n", 40), "Bad end: '%s'", end);
}

static void
test_filename_to_channels (void)
{
  evalresp_channels *channels = NULL;
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, NULL,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 2), "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);
}

static int
single_letter_prefix (char *string, int length)
{
  int i;
  if (*string == ' ')
    return 0;
  if (strlen (string) != (size_t)length)
    return 0;
  for (i = 1; i < length; ++i)
  {
    if (string[i] != ' ')
      return 0;
  }
  return 1;
}

static void
test_splits (void)
{
  int i;
  evalresp_filter *filter = NULL;
  ER_CHECK (!evalresp_new_filter (NULL, &filter));
  ER_CHECK (!evalresp_add_sncl_all (NULL, filter, "A", " B, C ", " D ,  E,  F", "G,H,I ,J"));
  ER_CHECK (!(filter->sncls->nscn != 1 * 2 * 3 * 4));
  for (i = 0; i < 1 * 2 * 3 * 4; ++i)
  {
    ER_CHECK_MSG (!(!single_letter_prefix (filter->sncls->scn_vec[i]->network, 1)), "'%s'", filter->sncls->scn_vec[i]->network);
    ER_CHECK_MSG (!(!single_letter_prefix (filter->sncls->scn_vec[i]->station, 1)), "'%s'", filter->sncls->scn_vec[i]->station, "'%s'", filter->sncls->scn_vec[i]->network);
    ER_CHECK_MSG (!(!single_letter_prefix (filter->sncls->scn_vec[i]->locid, 1)), "'%s'", filter->sncls->scn_vec[i]->locid);
    ER_CHECK_MSG (!(!single_letter_prefix (filter->sncls->scn_vec[i]->channel, 1)), "'%s'", filter->sncls->scn_vec[i]->channel);
  }
}

static void
test_filter (void)
{
  evalresp_channels *channels = NULL;
  evalresp_filter *filter = NULL;
  ER_CHECK (!evalresp_new_filter (NULL, &filter));

  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, filter,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 2), "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  ER_CHECK (!(!evalresp_set_year (NULL, filter, "1990x")));
  ER_CHECK (!evalresp_set_year (NULL, filter, "1990"));
  ER_CHECK (!(filter->datetime->year != 1990));
  ER_CHECK (!evalresp_set_julian_day (NULL, filter, "100"));
  ER_CHECK (!(filter->datetime->jday != 100));
  ER_CHECK (!evalresp_set_time (NULL, filter, "1:2:3.4"));
  ER_CHECK (!(filter->datetime->hour != 1));
  ER_CHECK (!(filter->datetime->min != 2));
  ER_CHECK (!(fabs (filter->datetime->sec - 3.4) > 0.0001));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, filter,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 1), "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  ER_CHECK (!evalresp_add_sncl_text (NULL, filter, "IU", "ANMO", NULL, "BHZ"));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, filter,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 1), "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);
  evalresp_free_filter (&filter);

  ER_CHECK (!evalresp_new_filter (NULL, &filter));
  ER_CHECK (!evalresp_add_sncl_text (NULL, filter, "IU", "ANMO", NULL, "BHN"));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, filter,
                                            &channels));
  ER_CHECK_MSG (!(channels->nchannels != 0), "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  evalresp_free_filter (&filter);
}

static void
test_julian_day (void)
{
  evalresp_filter *filter = NULL;
  evalresp_new_filter (NULL, &filter);
  evalresp_set_year (NULL, filter, "2001");
  evalresp_set_julian_day (NULL, filter, "235");
  evalresp_set_time (NULL, filter, "08:16:02");
  time_t epoch = to_epoch (filter->datetime);
  // https://www.epochconverter.com/
  ER_CHECK_MSG (!(epoch != 998554562), "Bad epoch: %lld", epoch);
  evalresp_free_filter (&filter);
}

static void
test_reference_without_responses (void)
{
  const char *resp =
      "B050F03     Station:     ANMO\n"
      "B050F16     Network:     IU\n"
      "B052F03     Location:    00\n"
      "B052F04     Channel:     BHZ\n"
      "B052F22     Start date:  1998,299,20\n"
      "B052F23     End date:    2000,293,16\n"
      "B060F03     Number of stages:     1\n"
      "B060F04     Stage sequence number: 1\n"
      "B060F05     Number of responses:   0\n";
  evalresp_channels *channels = NULL;
  int status = evalresp_char_to_channels (NULL, resp, NULL, NULL, &channels);
  ER_CHECK_MSG (status == EVALRESP_PAR, "Unexpected status: %d", status);
  evalresp_free_channels (&channels);
}

const er_test unit_input_tests[] = {
    {"slurp_line", test_slurp_line, NULL},
    {"slurp_line_buffer", test_slurp_line_buffer, NULL},
    {"find_line", test_find_line, NULL},
    {"find_field", test_find_field, NULL},
    {"file_to_char", test_file_to_char, NULL},
    {"filename_to_channels", test_filename_to_channels, NULL},
    {"splits", test_splits, NULL},
    {"filter", test_filter, NULL},
    {"julian_day", test_julian_day, NULL},
    {"reference_without_responses", test_reference_without_responses, NULL},
    {NULL, NULL, NULL},
};
