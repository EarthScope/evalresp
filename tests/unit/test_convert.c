
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"

static void
run_test (const char *station, const char *response)
{
  FILE *in;
  ER_CHECK (in = fopen (station, "r"));
  FILE *out;
  ER_CHECK (out = fopen ("convert.out", "w+"));
  evalresp_logger *log = NULL;
  /*XXX x2r_log *log;
    ER_CHECK (!x2r_alloc_log(X2R_DEBUG, stderr, &log)); */
  x2r_fdsn_station_xml *root = NULL;
  ER_CHECK (!x2r_station_service_load (log, in, &root));
  ER_CHECK (!x2r_resp_util_write (log, out, root));
  rewind (out);
  FILE *expect;
  ER_CHECK (expect = fopen (response, "r"));
  char a[1000], b[1000];
  int line = 0, aok = 1, bok = 1;
  while (aok && bok)
  {
    aok = (fgets (a, 1000, expect) != NULL);
    bok = (fgets (b, 1000, out) != NULL);
    line++;
    ER_CHECK_MSG (!(aok != bok), "Different file lengths after line %d", line);
    if (aok)
      ER_CHECK_MSG (!strcmp (a, b), "%s/%s: line %d differs (expected, generated):\n%s%s", response, station, line, a, b);
  }
  fclose (in);
  fclose (out);
  fclose (expect);
  ER_CHECK (!x2r_free_fdsn_station_xml (root, X2R_OK));
  /*XXX ER_CHECK (!x2r_free_log(log, X2R_OK)); */
}

static void
test_convert_1 (void)
{
  run_test (er_data_path ("station-1.xml"), er_data_path ("response-1"));
}

static void
test_convert_2 (void)
{
  run_test (er_data_path ("station-2.xml"), er_data_path ("response-2"));
}

static void
test_convert_3 (void)
{
  // this has "no data" in a stage, but still has a channel gain
  run_test (er_data_path ("station-3.xml"), er_data_path ("response-3"));
}

const er_test unit_convert_tests[] = {
    {"1", test_convert_1, NULL},
    {"2", test_convert_2, NULL},
    {"3", test_convert_3, NULL},
    {NULL, NULL, NULL},
};
