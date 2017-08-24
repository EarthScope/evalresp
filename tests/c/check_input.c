#include <check.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "evalresp/constants.h"
#include "evalresp/input.h"
#include "evalresp/private.h"
#include "evalresp/public_api.h"

START_TEST (test_slurp_line)
{
  const char *input = "line 1\nline 2\nline 3";
  const char *start = input;
  char line[MAXLINELEN];
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 1\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 2\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, "line 3"), "Slurped [%s] (size %d)", line, strlen (line));
  fail_if (*start, "Start is [%s]", start);
  slurp_line (&start, line, MAXLINELEN);
  fail_if (strcmp (line, ""), "Slurped [%s] (size %d)", line, strlen (line));
  fail_if (*start, "Start is [%s]", start);
}
END_TEST

START_TEST (test_slurp_line_buffer)
{
  const char *input = "\n1\n12\n123\n";
  const char *start = input;
  char line[3];
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "1\n"), "Slurped [%s] (size %d)", line, strlen (line));
  slurp_line (&start, line, 3);
  fail_if (strcmp (line, "12"), "Slurped [%s] (size %d)", line, strlen (line));
}
END_TEST

START_TEST (test_find_line)
{
  const char *input = "B999F99 name1: value1 \nB666F66 name2: value2", *start;
  char line[MAXLINELEN];
  start = input;
  fail_if (find_line (NULL, &start, ":", 999, 99, line));
  fail_if (strcmp (line, "value1 "), "'%s'", line);
  fail_if (find_line (NULL, &start, ":", 666, 66, line));
  fail_if (strcmp (line, "value2"), "'%s'", line);
  start = input;
  fail_if (find_line (NULL, &start, ":", 666, 66, line));
  fail_if (strcmp (line, "value2"), "'%s'", line);
}
END_TEST

START_TEST (test_find_field)
{
  const char *input = "B999F99 name1: a b c \nB666F66 name2: value2", *start;
  char field[MAXLINELEN];
  start = input;
  fail_if (find_field (NULL, &start, ":", 999, 99, 0, field));
  fail_if (strcmp (field, "a"), "'%s'", field);
  fail_if (find_field (NULL, &start, ":", 666, 66, 0, field));
  fail_if (strcmp (field, "value2"), "'%s'", field);
  start = input;
  fail_if (find_field (NULL, &start, ":", 999, 99, 2, field));
  fail_if (strcmp (field, "c"), "'%s'", field);
}
END_TEST

START_TEST (test_file_to_char)
{
  char *seed = NULL;
  FILE *in = NULL;
  fail_if (open_file (NULL, "./data/RESP.IU.ANMO..BHZ", &in));
  fail_if (file_to_char (NULL, in, &seed));
  int len = strlen (seed);
  fail_if (len != 192590, "wrong length: %d", len); // value checks with wc
  char start[40];
  int i;
  for (i = 0; i < 39; ++i)
    start[i] = seed[i];
  start[40] = '\0';
  fail_if (strncmp (seed, "#\t\t<< IRIS SEED Reader, Release 4.16 >>", 39),
           "Bad start: '%s'", start);
  char end[40];
  for (i = 0; i < 40; ++i)
    end[i] = seed[i + 192590 - 40];
  fail_if (strncmp (seed + 192590 - 40, "of calibrations:                0\r\n#\t\t\r\n", 40),
           "Bad end: '%s'", end);
}
END_TEST

START_TEST (test_filename_to_channels)
{
  evalresp_channels *channels = NULL;
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, NULL,
                                          &channels));
  fail_if (channels->nchannels != 2, "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);
}
END_TEST

int
single_letter_prefix (char *string, int length)
{
  int i;
  if (*string == ' ')
    return 0;
  if (strlen (string) != length)
    return 0;
  for (i = 1; i < length; ++i)
  {
    if (string[i] != ' ')
      return 0;
  }
  return 1;
}

START_TEST (test_splits)
{
  int i;
  evalresp_filter *filter = NULL;
  fail_if (evalresp_new_filter (NULL, &filter));
  fail_if (evalresp_add_sncl_all (NULL, filter, "A", " B, C ", " D ,  E,  F", "G,H,I ,J"));
  fail_if (filter->sncls->nscn != 1 * 2 * 3 * 4);
  for (i = 0; i < 1 * 2 * 3 * 4; ++i)
  {
    fail_if (!single_letter_prefix (filter->sncls->scn_vec[i]->network, 1), "'%s'", filter->sncls->scn_vec[i]->network);
    fail_if (!single_letter_prefix (filter->sncls->scn_vec[i]->station, 1), "'%s'", filter->sncls->scn_vec[i]->station, "'%s'", filter->sncls->scn_vec[i]->network);
    fail_if (!single_letter_prefix (filter->sncls->scn_vec[i]->locid, 1), "'%s'", filter->sncls->scn_vec[i]->locid);
    fail_if (!single_letter_prefix (filter->sncls->scn_vec[i]->channel, 1), "'%s'", filter->sncls->scn_vec[i]->channel);
  }
}
END_TEST

START_TEST (test_filter)
{
  evalresp_channels *channels = NULL;
  evalresp_filter *filter = NULL;
  fail_if (evalresp_new_filter (NULL, &filter));

  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, filter,
                                          &channels));
  fail_if (channels->nchannels != 2, "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  fail_if (!evalresp_set_year (NULL, filter, "1990x"));
  fail_if (evalresp_set_year (NULL, filter, "1990"));
  fail_if (filter->datetime->year != 1990);
  fail_if (evalresp_set_julian_day (NULL, filter, "100"));
  fail_if (filter->datetime->jday != 100);
  fail_if (evalresp_set_time (NULL, filter, "1:2:3.4"));
  fail_if (filter->datetime->hour != 1);
  fail_if (filter->datetime->min != 2);
  fail_if (fabs (filter->datetime->sec - 3.4) > 0.0001);
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, filter,
                                          &channels));
  fail_if (channels->nchannels != 1, "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  fail_if (evalresp_add_sncl_text (NULL, filter, "IU", "ANMO", NULL, "BHZ"));
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, filter,
                                          &channels));
  fail_if (channels->nchannels != 1, "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);
  evalresp_free_filter (&filter);

  fail_if (evalresp_new_filter (NULL, &filter));
  fail_if (evalresp_add_sncl_text (NULL, filter, "IU", "ANMO", NULL, "BHN"));
  fail_if (evalresp_filename_to_channels (NULL, "./data/RESP.IU.ANMO..BHZ", NULL, filter,
                                          &channels));
  fail_if (channels->nchannels != 0, "Unexpected number of channels: %d", channels->nchannels);
  evalresp_free_channels (&channels);

  evalresp_free_filter (&filter);
}
END_TEST

START_TEST (test_julian_day)
{
  evalresp_filter *filter = NULL;
  evalresp_new_filter (NULL, &filter);
  evalresp_set_year (NULL, filter, "2001");
  evalresp_set_julian_day (NULL, filter, "235");
  evalresp_set_time (NULL, filter, "08:16:02");
  time_t epoch = to_epoch (filter->datetime);
  // https://www.epochconverter.com/
  fail_if (epoch != 998554562, "Bad epoch: %lld", epoch);
  evalresp_free_filter (&filter);
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_slurp_line);
  tcase_add_test (tc, test_slurp_line_buffer);
  tcase_add_test (tc, test_find_line);
  tcase_add_test (tc, test_find_field);
  tcase_add_test (tc, test_file_to_char);
  tcase_add_test (tc, test_filename_to_channels);
  tcase_add_test (tc, test_splits);
  tcase_add_test (tc, test_filter);
  tcase_add_test (tc, test_julian_day);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-log.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
