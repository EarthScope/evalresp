
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


typedef struct {
    double value;
    double plus_error;
    double minus_error;
} x2r_float;

typedef struct {
    int number;
    x2r_float real;
    x2r_float imaginary;
} x2r_pole_zero;

typedef struct {
    double frequency;
    x2r_float amplitude;
    x2r_float phase;
} x2r_response_list_element;

typedef struct {
    int i;
    double value;
} x2r_numerator_coefficient;

typedef struct {
    int number;
    x2r_float value;
} x2r_coefficient;

typedef struct {
    char *name;
    char *description;
} x2r_units;

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

typedef struct {
    char *cf_transfer_function_type;
    x2r_units input_units;
    x2r_units output_units;
    x2r_float *numerator;
    int n_numerators;
    x2r_float *denominator;
    int n_denominators;
} x2r_coefficients;

typedef struct {
    x2r_units input_units;
    x2r_units output_units;
    x2r_response_list_element *response_list_element;
    int n_response_list_elements;
} x2r_response_list;

typedef struct {
    char *symmetry;
    char *name;
    x2r_units input_units;
    x2r_units output_units;
    x2r_numerator_coefficient *numerator_coefficient;
    int n_numerator_coefficients;
} x2r_fir;

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

typedef struct {
    double input_sample_rate;
    int factor;
    int offset;
    double delay;
    double correction;
} x2r_decimation;

typedef struct {
    double value;
    double frequency;
} x2r_gain;

typedef enum {
    X2R_STAGE_UNKNOWN = 0,
    X2R_STAGE_POLES_ZEROS,
    X2R_STAGE_COEFFICIENTS,
    X2R_STAGE_RESPONSE_LIST,
    X2R_STAGE_FIR,
    X2R_STAGE_POLYNOMIAL
} x2r_stage_type;

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

typedef struct {
    x2r_stage *stage;
    int n_stages;
    x2r_gain *instrument_sensitivity;
    x2r_polynomial *instrument_polynomial;
} x2r_response;

typedef struct {
    char *code;
    char *location_code;
    time_t start_date;
    time_t end_date;
    x2r_response response;
} x2r_channel;

typedef struct {
    char *code;
    x2r_channel *channel;
    int n_channels;
} x2r_station;

typedef struct {
    char *code;
    x2r_station *station;
    int n_stations;
} x2r_network;

typedef struct {
    x2r_network *network;
    int n_networks;
} x2r_fdsn_station_xml;


int x2r_parse_iso_datetime(x2r_log *log, const char *datetime, time_t *epoch);
int x2r_station_service_load(x2r_log *log, FILE *in, x2r_fdsn_station_xml **root);
int x2r_free_fdsn_station_xml(x2r_fdsn_station_xml *root, int status);

#endif
