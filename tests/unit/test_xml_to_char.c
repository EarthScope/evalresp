
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/input.h"
#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"

static FILE *
open_path (const char *path)
{
  FILE *f = NULL;
  ER_CHECK_MSG (NULL != (f = fopen (path, "r")), "%s", path);
  return f;
}

static void
run_xml_to_char_test (const char *xml_path, const char *resp_path)
{
  FILE *in_fd = NULL, *check_fd = NULL;
  char *test_char = NULL, *test_xml = NULL, *check_char = NULL;
  evalresp_logger *log = NULL;

  in_fd = open_path (xml_path);
  ER_CHECK (EVALRESP_OK == file_to_char (log, in_fd, &test_xml));
  fclose (in_fd);
  ER_CHECK (EVALRESP_OK == evalresp_xml_to_char (log, 1, test_xml, &test_char));
  free (test_xml);
  check_fd = open_path (resp_path);
  ER_CHECK (EVALRESP_OK == file_to_char (log, check_fd, &check_char));
  fclose (check_fd);
  ER_CHECK (0 == strcmp (test_char, check_char));
  free (check_char);
  free (test_char);
}

static void
test_xml_to_char_1_flag_1 (void)
{
  run_xml_to_char_test (er_data_path ("station-1.xml"), er_data_path ("response-1"));
}

static void
test_xml_to_char_1_flag_2 (void)
{
  run_xml_to_char_test (er_data_path ("station-2.xml"), er_data_path ("response-2"));
}

static void
test_xml_to_char_1_flag_3 (void)
{
  run_xml_to_char_test (er_data_path ("station-3.xml"), er_data_path ("response-3"));
}

static void
test_xml_to_char_0_flag (void)
{
  char *test_xml = NULL, *test_char = NULL;
  evalresp_logger *log = NULL;
  ER_CHECK (EVALRESP_OK == evalresp_xml_to_char (log, 0, test_xml, &test_char));
}

const er_test unit_xml_to_char_tests[] = {
    {"0_flag", test_xml_to_char_0_flag, NULL},
    {"1_flag_1", test_xml_to_char_1_flag_1, NULL},
    {"1_flag_2", test_xml_to_char_1_flag_2, NULL},
    {"1_flag_3", test_xml_to_char_1_flag_3, NULL},
    {NULL, NULL, NULL},
};
