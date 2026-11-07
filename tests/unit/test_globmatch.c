#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/constants.h"
#include "evalresp/globmatch.h"
#include "evalresp/input.h"
#include "evalresp/private.h"
#include "evalresp/public_api.h"
#include "harness/harness.h"

static void
test_glob (void)
{
  ER_CHECK (evr_globmatch ("BHZ", "BHZ"));
  ER_CHECK (evr_globmatch ("BHZ", "BH?"));
  ER_CHECK (evr_globmatch ("BHZ", "*"));
  ER_CHECK (evr_globmatch ("BHZ", "B*Z"));
  ER_CHECK (evr_globmatch ("BHZ", "*Z"));
  ER_CHECK (evr_globmatch ("BHZ", "B??"));
  ER_CHECK (evr_globmatch ("", "*"));
  ER_CHECK (evr_globmatch ("", ""));
  ER_CHECK (!evr_globmatch ("00", ""));
  ER_CHECK (!evr_globmatch ("", "00"));
  ER_CHECK (!evr_globmatch ("", "?"));

  /* patterns match the whole string */
  ER_CHECK (!evr_globmatch ("ANMO2", "ANMO"));
  ER_CHECK (!evr_globmatch ("XANMO", "ANMO"));
  ER_CHECK (!evr_globmatch ("BHZ", "BH"));
  ER_CHECK (!evr_globmatch ("BH", "BHZ"));
  ER_CHECK (!evr_globmatch ("BHZ", "BH?Z"));

  /* '.' is an ordinary character */
  ER_CHECK (!evr_globmatch ("BHZ", "B.Z"));
  ER_CHECK (evr_globmatch ("B.Z", "B.Z"));

  /* character sets, ranges and negation */
  ER_CHECK (evr_globmatch ("BHZ", "BH[ZNE]"));
  ER_CHECK (!evr_globmatch ("BH1", "BH[ZNE]"));
  ER_CHECK (evr_globmatch ("BH1", "BH[0-9]"));
  ER_CHECK (evr_globmatch ("BH1", "BH[!ZNE]"));
  ER_CHECK (evr_globmatch ("BH1", "BH[^ZNE]"));
  ER_CHECK (!evr_globmatch ("BHZ", "BH[!ZNE]"));
  ER_CHECK (!evr_globmatch ("BH", "BH[ZNE]"));
  ER_CHECK (!evr_globmatch ("BHZ", "BH[ZNE"));

  /* escapes */
  ER_CHECK (evr_globmatch ("a*b", "a\\*b"));
  ER_CHECK (!evr_globmatch ("axb", "a\\*b"));
  ER_CHECK (evr_globmatch ("a?b", "a\\?b"));

  /* backtracking */
  ER_CHECK (evr_globmatch ("aXbXc", "*X*c"));
  ER_CHECK (evr_globmatch ("abcabc", "*abc"));
  ER_CHECK (!evr_globmatch ("abcab", "*abc"));
  ER_CHECK (evr_globmatch ("M/S**2", "M/S\\*\\*2*"));

  ER_CHECK (!evr_globmatch (NULL, "*"));
  ER_CHECK (!evr_globmatch ("a", NULL));
}

static void
test_is_int (void)
{
  const char *yes[] = {"0", "7", "+12", "-3", "0042", NULL};
  const char *no[] = {"", "+", "-", "1.0", " 1", "1 ", "1x", "x1", "--1", "1e5", NULL};
  int i;
  for (i = 0; yes[i]; ++i)
    ER_CHECK_MSG (is_int (yes[i], NULL), "'%s'", yes[i]);
  for (i = 0; no[i]; ++i)
    ER_CHECK_MSG (!is_int (no[i], NULL), "'%s'", no[i]);
}

static void
test_is_real (void)
{
  const char *yes[] = {"1", "1.", ".5", "1.5", "-1.5e-3", "+2E+4", "1.e5", ".5E+2", "10e3",
                       "0.0", "-0", "1.5E3", NULL};
  const char *no[] = {"", ".", "+", "-.", "e5", "1e", "1e+", "1.2.3", "inf", "nan", "0x1",
                      " 1", "1 ", "1x", "1..2", "1e5.5", ".e5", NULL};
  int i;
  for (i = 0; yes[i]; ++i)
    ER_CHECK_MSG (is_real (yes[i], NULL), "'%s'", yes[i]);
  for (i = 0; no[i]; ++i)
    ER_CHECK_MSG (!is_real (no[i], NULL), "'%s'", no[i]);
}

/* Number of channels selected by the given SNCL from RESP.IU.ANMO..BHZ,
 * which holds channels BHZ and BHX. */
static int
count_selected (const char *net, const char *sta, const char *loc, const char *chan)
{
  int n;
  evalresp_channels *channels = NULL;
  evalresp_filter *filter = NULL;
  ER_CHECK (!evalresp_new_filter (NULL, &filter));
  ER_CHECK (!evalresp_add_sncl_text (NULL, filter, net, sta, loc, chan));
  ER_CHECK (!evalresp_filename_to_channels (NULL, er_data_path ("RESP.IU.ANMO..BHZ"), NULL, filter,
                                            &channels));
  n = channels->nchannels;
  evalresp_free_channels (&channels);
  evalresp_free_filter (&filter);
  return n;
}

static void
test_filter_patterns (void)
{
  ER_CHECK (count_selected ("IU", "ANMO", NULL, "BHZ") == 1);
  ER_CHECK (count_selected ("IU", "AN*", NULL, "BH?") == 2);
  ER_CHECK (count_selected ("*", "*", NULL, "*") == 2);
  ER_CHECK (count_selected ("*", "*", NULL, "BH[ZNE]") == 1);
  ER_CHECK (count_selected ("*", "*", NULL, "BH[!Z]") == 1);
  ER_CHECK (count_selected ("IU", "ANMO", NULL, "BH[NE]") == 0);
  ER_CHECK (count_selected ("IU", "ANM", NULL, "BHZ") == 0);
  ER_CHECK (count_selected ("IU", "NMO", NULL, "BHZ") == 0);
  ER_CHECK (count_selected ("I", "ANMO", NULL, "BHZ") == 0);
  ER_CHECK (count_selected ("IU", "ANMO", NULL, "BH") == 0);
}

const er_test unit_globmatch_tests[] = {
    {"glob", test_glob, NULL},
    {"is_int", test_is_int, NULL},
    {"is_real", test_is_real, NULL},
    {"filter_patterns", test_filter_patterns, NULL},
    {NULL, NULL, NULL},
};
