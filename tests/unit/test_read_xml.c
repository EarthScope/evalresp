
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

static void
test_read_xml_empty_description (void)
{
  static const char xml[] =
      "<FDSNStationXML xmlns=\"http://www.fdsn.org/xml/station/1\" schemaVersion=\"1.1\">"
      "<Source>test</Source><Created>2026-11-06T14:28:02</Created>"
      "<Network code=\"XX\" startDate=\"1963-01-01T00:00:00\">"
      "<Station code=\"STA\" startDate=\"2024-08-20T00:00:00\">"
      "<Latitude>1</Latitude><Longitude>1</Longitude><Elevation>1</Elevation>"
      "<Site><Name>S</Name></Site>"
      "<Channel code=\"BHZ\" locationCode=\"00\" startDate=\"2024-08-20T00:00:00\">"
      "<Latitude>1</Latitude><Longitude>1</Longitude><Elevation>1</Elevation>"
      "<Depth>0</Depth><SampleRate>1</SampleRate>"
      "<Response>"
      "<Stage number=\"1\"><PolesZeros>"
      "<InputUnits><Name>m</Name><Description></Description></InputUnits>"
      "<OutputUnits><Name>V</Name><Description/></OutputUnits>"
      "<PzTransferFunctionType>LAPLACE (RADIANS/SECOND)</PzTransferFunctionType>"
      "<NormalizationFactor>1.0</NormalizationFactor>"
      "<NormalizationFrequency>1.0</NormalizationFrequency>"
      "</PolesZeros>"
      "<StageGain><Value>1.0</Value><Frequency>1.0</Frequency></StageGain></Stage>"
      "</Response></Channel></Station></Network></FDSNStationXML>";
  evalresp_logger *log = NULL;
  x2r_fdsn_station_xml *root = NULL;
  x2r_poles_zeros *pz;

  ER_CHECK (!x2r_station_service_load_string (log, xml, &root));
  ER_CHECK (!(root->network[0].station[0].channel[0].response.n_stages != 1));
  pz = root->network[0].station[0].channel[0].response.stage[0].u.poles_zeros;
  ER_CHECK (!strcmp (pz->input_units.name, "m"));
  ER_CHECK (!strcmp (pz->input_units.description, ""));
  ER_CHECK (!strcmp (pz->output_units.description, ""));
  ER_CHECK (!x2r_free_fdsn_station_xml (root, X2R_OK));
}

const er_test unit_read_xml_tests[] = {
    {"read_xml", test_read_xml, NULL},
    {"response_list", test_read_xml_response_list, NULL},
    {"empty_description", test_read_xml_empty_description, NULL},
    {NULL, NULL, NULL},
};
