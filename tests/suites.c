#include "harness/harness.h"

extern const er_test unit_auto_tests[];
extern const er_test unit_compare_tests[];
extern const er_test unit_convert_tests[];
extern const er_test unit_count_tests[];
extern const er_test unit_evaluation_tests[];
extern const er_test unit_globmatch_tests[];
extern const er_test unit_input_tests[];
extern const er_test unit_log_tests[];
extern const er_test unit_log_context_tests[];
extern const er_test unit_match_tests[];
extern const er_test unit_parse_datetime_tests[];
extern const er_test unit_read_xml_tests[];
extern const er_test unit_response_tests[];
extern const er_test unit_response_char_tests[];
extern const er_test unit_spline_tests[];
extern const er_test unit_xml_channels_tests[];
extern const er_test unit_xml_to_char_tests[];
extern const er_test *cli_tests (void);

static er_suite suites[] = {
    {"auto", unit_auto_tests},
    {"compare", unit_compare_tests},
    {"convert", unit_convert_tests},
    {"count", unit_count_tests},
    {"evaluation", unit_evaluation_tests},
    {"globmatch", unit_globmatch_tests},
    {"input", unit_input_tests},
    {"log", unit_log_tests},
    {"log_context", unit_log_context_tests},
    {"match", unit_match_tests},
    {"parse_datetime", unit_parse_datetime_tests},
    {"read_xml", unit_read_xml_tests},
    {"response", unit_response_tests},
    {"response_char", unit_response_char_tests},
    {"spline", unit_spline_tests},
    {"xml_channels", unit_xml_channels_tests},
    {"xml_to_char", unit_xml_to_char_tests},
    {"cli", NULL},
    {NULL, NULL},
};

const er_suite *
er_get_suites (void)
{
  int i;
  for (i = 0; suites[i].name; ++i)
    if (!suites[i].tests)
      suites[i].tests = cli_tests ();
  return suites;
}
