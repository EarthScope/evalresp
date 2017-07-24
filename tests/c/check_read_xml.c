
#include <check.h>
#include <fcntl.h>
#include <stdio.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/xml_to_dom.h"
#include "evalresp_log/log.h"

START_TEST (test_read_xml)
{
  FILE *in;
  fail_if (!(in = fopen ("./data/station-1.xml", "r")));
  /*XXX x2r_log *log; */
  evalresp_logger *log = NULL;
  /*XXX fail_if(x2r_alloc_log(X2R_DEBUG, stderr, &log)); */
  /*XXX fail_if(x2r_alloc_log(X2R_DEBUG, stderr, log)); */
  x2r_fdsn_station_xml *root = NULL;
  fail_if (x2r_station_service_load (log, in, &root));
  fail_if (root->n_networks != 1, "unexpected number of networks: %d", root->n_networks);
  fail_if (root->network[0].n_stations != 1, "unexpected number of stations: %d", root->network[0].n_stations);
  fail_if (root->network[0].station[0].n_channels != 47, "unexpected number of channels: %d", root->network[0].station[0].n_channels);
  fail_if (x2r_free_fdsn_station_xml (root, X2R_OK));
  /*XXX fail_if(x2r_free_log(log, X2R_OK)); */
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_read_xml);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-read_xml.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
