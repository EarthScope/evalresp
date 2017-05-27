/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private X2R in-memory model interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_x2r_xml evalresp Private X2RL In-Memory Model Interface
 * @ingroup evalresp_x2r
 * @brief Private X2R in-memory model interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        X2R in-memory model.
 */

#ifndef X2R_XML_H
#define X2R_XML_H

#include <time.h>
#include <stdio.h>

#include "x2r_log.h"


// This is the in-memory model of the station.xml file.

// It might have been possible to use the same structures as evalresp, and
// so perhaps have avoided writing an intermediate response file.  But that
// would have made the system much harder to debug.

// Name below correspond, in general, to edu.iris.dmc.fdsn.station.model classes
// in the IRIS-WS source.


/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    double value;
    double plus_error;
    double minus_error;
} x2r_float;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    int number;
    x2r_float real;
    x2r_float imaginary;
} x2r_pole_zero;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    double frequency;
    x2r_float amplitude;
    x2r_float phase;
} x2r_response_list_element;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    int i;
    double value;
} x2r_numerator_coefficient;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    int number;
    x2r_float value;
} x2r_coefficient;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *name;
    char *description;
} x2r_units;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *pz_transfer_function_type;
    x2r_units input_units;
    x2r_units output_units;
    double normalization_factor;
    double normalization_frequency;
    x2r_pole_zero *zero;
    int n_zeros;
    x2r_pole_zero *pole;
    int n_poles;
} x2r_poles_zeros;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *cf_transfer_function_type;
    x2r_units input_units;
    x2r_units output_units;
    x2r_float *numerator;
    int n_numerators;
    x2r_float *denominator;
    int n_denominators;
} x2r_coefficients;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    x2r_units input_units;
    x2r_units output_units;
    x2r_response_list_element *response_list_element;
    int n_response_list_elements;
} x2r_response_list;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *symmetry;
    char *name;
    x2r_units input_units;
    x2r_units output_units;
    x2r_numerator_coefficient *numerator_coefficient;
    int n_numerator_coefficients;
} x2r_fir;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *approximation_type;
    x2r_units input_units;
    x2r_units output_units;
    double frequency_lower_bound;
    double frequency_upper_bound;
    double approximation_lower_bound;
    double approximation_upper_bound;
    double maximum_error;
    x2r_coefficient *coefficient;
    int n_coefficients;
} x2r_polynomial;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    double input_sample_rate;
    int factor;
    int offset;
    double delay;
    double correction;
} x2r_decimation;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    double value;
    double frequency;
} x2r_gain;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef enum {
    X2R_STAGE_UNKNOWN = 0,
    X2R_STAGE_POLES_ZEROS,
    X2R_STAGE_COEFFICIENTS,
    X2R_STAGE_RESPONSE_LIST,
    X2R_STAGE_FIR,
    X2R_STAGE_POLYNOMIAL
} x2r_stage_type;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    int number;
    x2r_stage_type type;
    union {
        x2r_poles_zeros *poles_zeros;
        x2r_coefficients *coefficients;
        x2r_response_list *response_list;
        x2r_fir *fir;
        x2r_polynomial *polynomial;
    } u;
    x2r_decimation *decimation;
    x2r_gain *stage_gain;
} x2r_stage;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    x2r_stage *stage;
    int n_stages;
    x2r_gain *instrument_sensitivity;
    x2r_polynomial *instrument_polynomial;
} x2r_response;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *code;
    char *location_code;
    time_t start_date;
    time_t end_date;
    x2r_response response;
} x2r_channel;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *code;
    x2r_channel *channel;
    int n_channels;
} x2r_station;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    char *code;
    x2r_station *station;
    int n_stations;
} x2r_network;

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
typedef struct {
    x2r_network *network;
    int n_networks;
} x2r_fdsn_station_xml;


/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
int x2r_parse_iso_datetime(x2r_log *log, const char *datetime, time_t *epoch);

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
int x2r_station_service_load(x2r_log *log, FILE *in, x2r_fdsn_station_xml **root);

/**
 * @private
 * @ingroup evalresp_x2r_xml
 */
int x2r_free_fdsn_station_xml(x2r_fdsn_station_xml *root, int status);

#endif
