
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"

static void
run_test (const char *input, const char *response)
{
  FILE *in = NULL, *out = NULL;
  evalresp_logger *log = NULL;
  ER_CHECK (NULL != (in = fopen (input, "r")));
  // ER_CHECK (!x2r_xml2resp_auto (&in, log));
  ER_CHECK (EVALRESP_OK == evalresp_xml_stream_to_resp_stream_auto (log, in, NULL, &out));
  ER_CHECK_MSG (out != NULL, "Should not get here, auto detection returned okay but no output fd");
  if (in != out)
  {
    fclose (in);
    in = NULL;
  }
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
      ER_CHECK_MSG (!strcmp (a, b), "%s/%s: line %d differs (expected, generated):\n%s%s", response, input, line, a, b);
  }
}

static void
test_auto_1 (void)
{
  run_test (er_data_path ("station-1.xml"), er_data_path ("response-1"));
}

static void
test_auto_2 (void)
{
  run_test (er_data_path ("response-1"), er_data_path ("response-1"));
}

const er_test unit_auto_tests[] = {
    {"1", test_auto_1, NULL},
    {"2", test_auto_2, NULL},
    {NULL, NULL, NULL},
};
