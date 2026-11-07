
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/xml_to_dom.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"

static void
test_parse_datetime (void)
{
  evalresp_logger *log = NULL;
  time_t epoch, target = 946684800;
  ER_CHECK (!x2r_parse_iso_datetime (log, "2000-01-01T00:00:00", &epoch));
  ER_CHECK_MSG (!(epoch != target), "Bad epoch: %ld (error %d)", epoch, epoch - target);
}

static void
test_32bit_bug (void)
{
  evalresp_logger *log = NULL;
  time_t epoch, target = 4102444800;
  ER_CHECK (!x2r_parse_iso_datetime (log, "2100-01-01T00:00:00", &epoch));
  ER_CHECK_MSG (!(epoch != target), "Bad epoch: %ld (error %d)", epoch, epoch - target);
}

const er_test unit_parse_datetime_tests[] = {
    {"parse_datetime", test_parse_datetime, NULL},
    {"32bit_bug", test_32bit_bug, NULL},
    {NULL, NULL, NULL},
};
