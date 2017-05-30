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
 * @defgroup evalresp_private_x2r_xml evalresp Private X2R In-Memory Model Interface
 * @ingroup evalresp_private_x2r
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
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    double value;  /**< FIXME. */
    double plus_error;  /**< FIXME. */
    double minus_error;  /**< FIXME. */
} x2r_float;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    int number;  /**< FIXME. */
    x2r_float real;  /**< FIXME. */
    x2r_float imaginary;  /**< FIXME. */
} x2r_pole_zero;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    double frequency;  /**< FIXME. */
    x2r_float amplitude;  /**< FIXME. */
    x2r_float phase;  /**< FIXME. */
} x2r_response_list_element;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    int i;  /**< FIXME. */
    double value;  /**< FIXME. */
} x2r_numerator_coefficient;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    int number;  /**< FIXME. */
    x2r_float value;  /**< FIXME. */
} x2r_coefficient;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *name;  /**< FIXME. */
    char *description;  /**< FIXME. */
} x2r_units;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *pz_transfer_function_type;  /**< FIXME. */
    x2r_units input_units;  /**< FIXME. */
    x2r_units output_units;  /**< FIXME. */
    double normalization_factor;  /**< FIXME. */
    double normalization_frequency;  /**< FIXME. */
    x2r_pole_zero *zero;  /**< FIXME. */
    int n_zeros;  /**< FIXME. */
    x2r_pole_zero *pole;  /**< FIXME. */
    int n_poles;  /**< FIXME. */
} x2r_poles_zeros;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *cf_transfer_function_type;  /**< FIXME. */
    x2r_units input_units;  /**< FIXME. */
    x2r_units output_units;  /**< FIXME. */
    x2r_float *numerator;  /**< FIXME. */
    int n_numerators;  /**< FIXME. */
    x2r_float *denominator;  /**< FIXME. */
    int n_denominators;  /**< FIXME. */
} x2r_coefficients;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    x2r_units input_units;  /**< FIXME. */
    x2r_units output_units;  /**< FIXME. */
    x2r_response_list_element *response_list_element;  /**< FIXME. */
    int n_response_list_elements;  /**< FIXME. */
} x2r_response_list;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *symmetry;  /**< FIXME. */
    char *name;  /**< FIXME. */
    x2r_units input_units;  /**< FIXME. */
    x2r_units output_units;  /**< FIXME. */
    x2r_numerator_coefficient *numerator_coefficient;  /**< FIXME. */
    int n_numerator_coefficients;  /**< FIXME. */
} x2r_fir;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *approximation_type;  /**< FIXME. */
    x2r_units input_units;  /**< FIXME. */
    x2r_units output_units;  /**< FIXME. */
    double frequency_lower_bound;  /**< FIXME. */
    double frequency_upper_bound;  /**< FIXME. */
    double approximation_lower_bound;  /**< FIXME. */
    double approximation_upper_bound;  /**< FIXME. */
    double maximum_error;  /**< FIXME. */
    x2r_coefficient *coefficient;  /**< FIXME. */
    int n_coefficients;  /**< FIXME. */
} x2r_polynomial;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    double input_sample_rate;  /**< FIXME. */
    int factor;  /**< FIXME. */
    int offset;  /**< FIXME. */
    double delay;  /**< FIXME. */
    double correction;  /**< FIXME. */
} x2r_decimation;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    double value;  /**< FIXME. */
    double frequency;  /**< FIXME. */
} x2r_gain;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef enum {
    X2R_STAGE_UNKNOWN = 0,  /**< FIXME. */
    X2R_STAGE_POLES_ZEROS,  /**< FIXME. */
    X2R_STAGE_COEFFICIENTS,  /**< FIXME. */
    X2R_STAGE_RESPONSE_LIST,  /**< FIXME. */
    X2R_STAGE_FIR,  /**< FIXME. */
    X2R_STAGE_POLYNOMIAL  /**< FIXME. */
} x2r_stage_type;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    int number;  /**< FIXME. */
    x2r_stage_type type;  /**< FIXME. */
    union {
        x2r_poles_zeros *poles_zeros;  /**< FIXME. */
        x2r_coefficients *coefficients;  /**< FIXME. */
        x2r_response_list *response_list;  /**< FIXME. */
        x2r_fir *fir;  /**< FIXME. */
        x2r_polynomial *polynomial;  /**< FIXME. */
    } u;  /**< FIXME. */
    x2r_decimation *decimation;  /**< FIXME. */
    x2r_gain *stage_gain;  /**< FIXME. */
} x2r_stage;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    x2r_stage *stage;  /**< FIXME. */
    int n_stages;  /**< FIXME. */
    x2r_gain *instrument_sensitivity;  /**< FIXME. */
    x2r_polynomial *instrument_polynomial;  /**< FIXME. */
} x2r_response;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *code;  /**< FIXME. */
    char *location_code;  /**< FIXME. */
    time_t start_date;  /**< FIXME. */
    time_t end_date;  /**< FIXME. */
    x2r_response response;  /**< FIXME. */
} x2r_channel;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *code;  /**< FIXME. */
    x2r_channel *channel;  /**< FIXME. */
    int n_channels;  /**< FIXME. */
} x2r_station;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    char *code;  /**< FIXME. */
    x2r_station *station;  /**< FIXME. */
    int n_stations;  /**< FIXME. */
} x2r_network;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief FIXME.
 */
typedef struct {
    x2r_network *network;  /**< FIXME. */
    int n_networks;  /**< FIXME. */
} x2r_fdsn_station_xml;


/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Parse an ISO yyyy-mm-ddThh:mm:ss format datetime.
 * @remarks Separated out for testing (see check_parse_datetime and also
 *          implicit tests against the IRIS-WS code elsewhere). Originally
 *          used strptime, but that's not available on Windows.
 */
int x2r_parse_iso_datetime(x2r_log *log, const char *datetime, time_t *epoch);

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief The equivalent of StationService.load() in IRIS-WS, constructing an
 *        in-memory representation of the station.xml file read from the given
 *        stream.
 */
int x2r_station_service_load(x2r_log *log, FILE *in, x2r_fdsn_station_xml **root);

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Free an x2r_fdsn_station_xml value.
 */
int x2r_free_fdsn_station_xml(x2r_fdsn_station_xml *root, int status);

#endif
