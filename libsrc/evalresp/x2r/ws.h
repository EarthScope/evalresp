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

#include <evalresp/x2r/log.h>
#include <evalresp/x2r/xml.h>
#include <evalresp_log/log.h>

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief Print the entire response document, given the in-memory model.
 */
int x2r_resp_util_write(evalresp_log_t *log, FILE *out, const x2r_fdsn_station_xml *root);

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief If xml_flag is set, convert the file and replace *in. Otherwise, do
 *        nothing.
 */
int x2r_xml2resp_on_flag(FILE **in, int xml_flag, int log_level, evalresp_log_t *log);

/**
 * @private
 * @ingroup evalresp_private_x2r_ws
 * @brief Check the given file, to see if the first character as <, and if so,
 *        convert and replace *in.
 */
int x2r_xml2resp_auto(FILE **in, int log_level, evalresp_log_t *log);

#endif
