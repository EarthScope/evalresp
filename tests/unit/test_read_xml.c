
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/xml_to_dom.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"

static void
test_read_xml (void)
{
  FILE *in;
  ER_CHECK (in = fopen (er_data_path ("station-1.xml"), "r"));
  /*XXX x2r_log *log; */
  evalresp_logger *log = NULL;
  /*XXX ER_CHECK (!x2r_alloc_log(X2R_DEBUG, stderr, &log)); */
  /*XXX ER_CHECK (!x2r_alloc_log(X2R_DEBUG, stderr, log)); */
  x2r_fdsn_station_xml *root = NULL;
  ER_CHECK (!x2r_station_service_load (log, in, &root));
  ER_CHECK_MSG (!(root->n_networks != 1), "unexpected number of networks: %d", root->n_networks);
  ER_CHECK_MSG (!(root->network[0].n_stations != 1), "unexpected number of stations: %d", root->network[0].n_stations);
  ER_CHECK_MSG (!(root->network[0].station[0].n_channels != 47), "unexpected number of channels: %d", root->network[0].station[0].n_channels);
  ER_CHECK (!x2r_free_fdsn_station_xml (root, X2R_OK));
  /*XXX ER_CHECK (!x2r_free_log(log, X2R_OK)); */
}

static void
test_read_xml_response_list (void)
{
  FILE *in;
  evalresp_logger *log = NULL;
  x2r_fdsn_station_xml *root = NULL;
  x2r_stage *stage;
  x2r_response_list *list;

  ER_CHECK (in = fopen (er_data_path ("station-list.xml"), "r"));
  ER_CHECK (!x2r_station_service_load (log, in, &root));
  fclose (in);
  ER_CHECK (!(root->network[0].station[0].n_channels != 1));
  ER_CHECK (!(root->network[0].station[0].channel[0].response.n_stages != 1));
  stage = &root->network[0].station[0].channel[0].response.stage[0];
  ER_CHECK (!(stage->type != X2R_STAGE_RESPONSE_LIST));
  list = stage->u.response_list;
  ER_CHECK_MSG (!(list->n_response_list_elements != 3), "unexpected number of elements: %d", list->n_response_list_elements);
  ER_CHECK (!(list->response_list_element[0].frequency != 0.1));
  ER_CHECK (!(list->response_list_element[1].amplitude.value != 2.5));
  ER_CHECK (!(list->response_list_element[2].phase.value != 30.0));
  ER_CHECK (!x2r_free_fdsn_station_xml (root, X2R_OK));
}

const er_test unit_read_xml_tests[] = {
    {"read_xml", test_read_xml, NULL},
    {"response_list", test_read_xml_response_list, NULL},
    {NULL, NULL, NULL},
};
