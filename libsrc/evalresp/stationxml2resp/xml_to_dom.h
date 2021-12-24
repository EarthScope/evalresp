/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private XML-to-RSEED in-memory model interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_private_x2r_xml evalresp Private XML-to-RSEED In-Memory Model Interface
 * @ingroup evalresp_private_x2r
 * @brief Private XML-to-RSEED in-memory model interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        XML-to-RSEED in-memory model.
 */

#ifndef X2R_XML_H
#define X2R_XML_H

#include <time.h>
#include <stdio.h>

#include <evalresp_log/log.h>


// This is the in-memory model of the station.xml file.

// It might have been possible to use the same structures as evalresp, and
// so perhaps have avoided writing an intermediate response file.  But that
// would have made the system much harder to debug.

// Names below correspond, in general, to edu.iris.dmc.fdsn.station.model classes
// in the IRIS-WS source.

/* A value to represent an unset, open, date-time.  Equates to 1906-08-16T20:26:40 */
#define unset_time_t -2000000000

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A float value with associated errors.
 */
typedef struct {
    double value;  /**< The value itself. */
    double plus_error;  /**< The positive error (optional; -1 if missing). */
    double minus_error;  /**< The negative error (optional; -1 if missing). */
} x2r_float;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A pole or zero (an indexed complex value with errors).
 */
typedef struct {
    int number;  /**< The index. */
    x2r_float real;  /**< The real component, with errors. */
    x2r_float imaginary;  /**< The imaginary component, with errors. */
} x2r_pole_zero;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A FAP value within a frequency response.
 */
typedef struct {
    double frequency;  /**< The frequency. */
    x2r_float amplitude;  /**< The amplitude, with errors. */
    x2r_float phase;  /**< The phase, with errors. */
} x2r_response_list_element;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief An indexed FIR numerator coefficient.
 */
typedef struct {
    int i;  /**< The index. */
    double value;  /**< The coefficient value. */
} x2r_numerator_coefficient;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief An indexed polynomial coefficient.
 */
typedef struct {
    int number;  /**< The index. */
    x2r_float value;  /**< The coefficient value, with errors. */
} x2r_coefficient;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Units data.
 */
typedef struct {
    char *name;  /**< Unit name. */
    char *description;  /**< Unit description. */
} x2r_units;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Poles and zeros defining a response.
 */
typedef struct {
    char *pz_transfer_function_type;  /**< The transfer function type. */
    x2r_units input_units;  /**< Input units. */
    x2r_units output_units;  /**< Output units. */
    double normalization_factor;  /**< Normalization factor. */
    double normalization_frequency;  /**< Normalization frequency. */
    x2r_pole_zero *zero;  /**< Array of zeros values. */
    int n_zeros;  /**< Number of zeros values. */
    x2r_pole_zero *pole;  /**< Array of poles values. */
    int n_poles;  /**< Number of poles values. */
} x2r_poles_zeros;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Stage coefficients (for an IIR filter, I assume).
 */
typedef struct {
	char *cf_transfer_function_type;  /**< The transfer function type. */
    x2r_units input_units;  /**< Input units. */
    x2r_units output_units;  /**< Output units. */
    x2r_float *numerator;  /**< Numerator values. */
    int n_numerators;  /**< Number of numerator values. */
    x2r_float *denominator;  /**< Denominator values. */
    int n_denominators;  /**< Number of denominator values. */
} x2r_coefficients;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief An FAP response.
 */
typedef struct {
    x2r_units input_units;  /**< Input units. */
    x2r_units output_units;  /**< Output units. */
    x2r_response_list_element *response_list_element;  /**< FAP triplets. */
    int n_response_list_elements;  /**< The number of FAP triplets. */
} x2r_response_list;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Coefficients for an FIR filter.
 */
typedef struct {
    char *symmetry;  /**< The filter symmetry. */
    char *name;  /**< The filter name. */
    x2r_units input_units;  /**< Input units. */
    x2r_units output_units;  /**< Output units. */
    x2r_numerator_coefficient *numerator_coefficient;  /**< Numerator coefficients. */
    int n_numerator_coefficients;  /**< The nunber of numerator coefficients. */
} x2r_fir;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A polynomial response.
 */
typedef struct {
    char *approximation_type;  /**< The approximation type. */
    x2r_units input_units;  /**< Input units. */
    x2r_units output_units;  /**< Output units. */
    double frequency_lower_bound;  /**< Lower frequency limit. */
    double frequency_upper_bound;  /**< Higher frequency limit. */
    double approximation_lower_bound;  /**< Lower approximation limit. */
    double approximation_upper_bound;  /**< Higher approximation limit. */
    double maximum_error;  /**< The maximum error. */
    x2r_coefficient *coefficient;  /**< The polynomial coefficients. */
    int n_coefficients;  /**< The number of polynomial coeffcicients. */
} x2r_polynomial;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A decimation stage.
 */
typedef struct {
    double input_sample_rate;  /**< Input sample rate. */
    int factor;  /**< Decimation factor. */
    int offset;  /**< Offset. */
    double delay;  /**< Delay. */
    double correction;  /**< Correction. */
} x2r_decimation;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A gain value at a given frequency.
 */
typedef struct {
    double value;  /**< The gain. */
    double frequency;  /**< The frequency. */
} x2r_gain;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Stage types.
 */
typedef enum {
    X2R_STAGE_UNKNOWN = 0,  /**< Unknown stage type. */
    X2R_STAGE_POLES_ZEROS,  /**< Poles and zeros stage. */
    X2R_STAGE_COEFFICIENTS,  /**< IIR stage. */
    X2R_STAGE_RESPONSE_LIST,  /**< FAP stage. */
    X2R_STAGE_FIR,  /**< FIR stage. */
    X2R_STAGE_POLYNOMIAL  /**< Polynomial stage. */
} x2r_stage_type;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A single stage.
 */
typedef struct {
    int number;  /**< The index. */
    x2r_stage_type type;  /**< The stage type (selects the correct union entry). */
    union {
        x2r_poles_zeros *poles_zeros;  /**< Poles and zeros data. */
        x2r_coefficients *coefficients;  /**< IIR data. */
        x2r_response_list *response_list;  /**< FAP data. */
        x2r_fir *fir;  /**< FIR data. */
        x2r_polynomial *polynomial;  /**< Polynomial data. */
    } u;  /**< The union. */
    x2r_decimation *decimation;  /**< Associated decimation. */
    x2r_gain *stage_gain;  /**< Associated gain. */
} x2r_stage;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A response consisting of stages and instrument.
 */
typedef struct {
    x2r_stage *stage;  /**< The stages. */
    int n_stages;  /**< The number of stages. */
    x2r_gain *instrument_sensitivity;  /**< The instrument sensitivity. */
    x2r_polynomial *instrument_polynomial;  /**< The instrument polynomial. */
} x2r_response;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A channel and response.
 */
typedef struct {
    char *code;  /**< Channel code. */
    char *location_code;  /**< Location code. */
    time_t start_date;  /**< Start date. */
    time_t end_date;  /**< End date. */
    x2r_response response;  /**< The associated response. */
} x2r_channel;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A station with channels.
 */
typedef struct {
    char *code;  /**< Station code. */
    x2r_channel *channel;  /**< The channels. */
    int n_channels;  /**< The number of channels. */
} x2r_station;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief A network with stations.
 */
typedef struct {
    char *code;  /**< Network code. */
    x2r_station *station;  /**< The stations. */
    int n_stations;  /**< The number of stations. */
} x2r_network;

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief An FDSN station with networks.
 */
typedef struct {
    x2r_network *network;  /**< The networks. */
    int n_networks;  /**< The number of networks. */
} x2r_fdsn_station_xml;


/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Parse an ISO yyyy-mm-ddThh:mm:ss format datetime.
 * @remarks Separated out for testing (see check_parse_datetime and also
 *          implicit tests against the IRIS-WS code elsewhere). Originally
 *          used strptime, but that's not available on Windows.
 */
int x2r_parse_iso_datetime(evalresp_logger *log, const char *datetime, time_t *epoch);

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief The equivalent of StationService.load() in IRIS-WS, constructing an
 *        in-memory representation of the station.xml file read from the given
 *        stream.
 */
int x2r_station_service_load(evalresp_logger *log, FILE *in, x2r_fdsn_station_xml **root);

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Free an x2r_fdsn_station_xml value.
 */
int x2r_free_fdsn_station_xml(x2r_fdsn_station_xml *root, int status);

#endif
