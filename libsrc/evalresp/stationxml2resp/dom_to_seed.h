/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private X2R formatted data interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_private_x2r_ws evalresp Private X2R Formatted Data Interface
 * @ingroup evalresp_private_x2r
 * @brief Private X2R formatted data interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        X2R formatted data.
 */

#ifndef X2R_WS_H
#define X2R_WS_H

#include <stdio.h>

#include <evalresp/stationxml2resp/xml_to_dom.h>
#include <evalresp_log/log.h>
#include <mxml/mxml.h>

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief Print the entire response document, given the in-memory model.
 */
int x2r_resp_util_write(evalresp_log_t *log, FILE *out, const x2r_fdsn_station_xml *root);

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief parse a mxml root node to datastructure
 */
int x2r_parse_fdsn_station_xml(evalresp_log_t *log, mxml_node_t *doc, x2r_fdsn_station_xml **root);

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief detect if file is xml
 */
int x2r_detect_xml(FILE *in, int *xml_flag);
#endif
