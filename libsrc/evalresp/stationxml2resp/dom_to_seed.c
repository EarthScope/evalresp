
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <evalresp/stationxml2resp.h>
#include <evalresp/stationxml2resp/xml_to_dom.h>
#include <evalresp/stationxml2resp/dom_to_seed.h>
#include <evalresp_log/log.h>


// This file generates response formatted data from the in-memory model of
// the station.xml document (created in x2r_xml.c).


/* printf-style output with linefeed. */
static int line(evalresp_logger *log, FILE *out, const char *template, ...) {

    int status = X2R_OK;
    va_list argp;
    char *with_lf = NULL;

    va_start(argp, template);

    if (!(with_lf = calloc(strlen(template) + 2, sizeof(*with_lf)))) {
        evalresp_log(log, EV_ERROR, 0, "Cannot allocate buffer");
        status = X2R_ERR_MEMORY;
    } else {
        sprintf(with_lf, "%s\n", template);
        if (vfprintf(out, with_lf, argp) < 0) {
            evalresp_log(log, EV_ERROR, 0, "Error printing %s", template);
            status = X2R_ERR_IO;
        }
    }

    free(with_lf);
    va_end(argp);

    return status;
}


/* Print multiple lines, NULL terminated. */
static int lines(evalresp_logger *log, FILE *out, ...) {

    int status = X2R_OK;
    va_list argp;
    char *text;

    va_start(argp, out);
    for (text = va_arg(argp, char*); text; text = va_arg(argp, char*)) {
        if (text) line(log, out, "%s", text);
    }

    va_end(argp);
    return status;
}


// https://github.com/earthscope/evalresp/issues/69
#define OPEN "No Ending Time"

/* Format epoch as given. */
static int format_date(evalresp_logger *log, const time_t epoch, int n, char *template, char **date) {

    int status = X2R_OK;
    struct tm *tm;

    if (epoch != unset_time_t) {
		if (!(tm = gmtime(&epoch))) {
			evalresp_log(log, EV_ERROR, 0, "Cannot convert epoch to time");
			status = X2R_ERR_DATE;
		} else {
			if (!(*date = calloc(n, sizeof(**date)))) {
				evalresp_log(log, EV_ERROR, 0, "Cannot alloc date");
				status = X2R_ERR_MEMORY;
			} else {
				if (!(strftime(*date, n, template, tm))) {
					evalresp_log(log, EV_ERROR, 0, "Cannot format date in %d char", n);
					status = X2R_ERR_BUFFER;
				}
			}
		}
    } else {
    	if (!(*date = calloc(1+strlen(OPEN), sizeof(**date)))) {
    		evalresp_log(log, EV_ERROR, 0, "Cannot alloc open date");
    		status = X2R_ERR_MEMORY;
    	} else {
    		if (!(*date = strdup(OPEN))) {
    			evalresp_log(log, EV_ERROR, 0, "Cannot copy open date");
				status = X2R_ERR_MEMORY;
    		}
    	}
    }

    return status;
}


/* Format epoch julian days. */
static int format_date_yjhms(evalresp_logger *log, const time_t epoch, char **date) {
    return format_date(log, epoch, strlen("YYYY,jjj,HH:MM:SS") + 1, "%Y,%j,%H:%M:%S", date);
}


/* Format epoch in American date style. */
static int format_date_mdy(evalresp_logger *log, time_t epoch, char **date) {
    return format_date(log, epoch, strlen("mm/dd/YYYY") + 1, "%m/%d/%Y", date);
}


/*
 * Copy a char* to another char*, to a max of len chars, without copying the terminating \0.
 *
 * (strncpy copies the terminating \0).
 */
static void chrncpy(char *destn, const char *source, int len) {
    int i;
    for (i = 0; i < len && *(source + i); ++i) {
        *(destn + i) = *(source + i);
    }
}


/* Pad the given text to the centre of the given width. */
static int centre(evalresp_logger *log, const char *text, int width, char **result) {

    int status = X2R_OK, margin, len;

    if (!(*result = calloc(width + 1, sizeof(**result)))) {
        evalresp_log(log, EV_ERROR, 0, "Cannot alloc centre");
        status = X2R_ERR_MEMORY;
    } else {

        len = strlen(text);
        len = len > width ? width : len;
        margin = (width - len) / 2;

        memset(*result, ' ', width);
        chrncpy(*result+margin, text, len);
    }

    return status;
}


/* Format a SNCL (without dots), centred as above. */
static int centre_sncl(evalresp_logger *log, const char *net, const char *stn, const x2r_channel *channel,
        int width, char **result) {

    int status = X2R_OK, len;
    char *sncl = NULL;

    len = 4 + 7 + 4 + strlen(channel->code);

    if (!(sncl = calloc(len + 1, sizeof(*sncl)))) {
        evalresp_log(log, EV_ERROR, 0, "Cannot alloc sncl");
        status = X2R_ERR_MEMORY;
    } else {

        memset(sncl, ' ', len);
        chrncpy(sncl, net, 4);
        chrncpy(sncl+4, stn, 7);
        chrncpy(sncl+11, channel->location_code, 4);
        strcpy(sncl+15, channel->code);

        status = centre(log, sncl, width, result);
    }

    free(sncl);
    return status;
}


/* Convert transfer function type from station.xml for response. */
static char *convert_tft(const char *transfer_function_type) {
    if (!strcmp(transfer_function_type, "LAPLACE (RADIANS/SECOND)")) {
        return "A";
    } else if (!strcmp(transfer_function_type, "LAPLACE (HERTZ)")) {
        return "B";
    // this one for poles and zeros
    } else if (!strcmp(transfer_function_type, "DIGITAL (Z-TRANSFORM)")) {
        return "D";
    // this one was for coefficients
    } else if (!strcmp(transfer_function_type, "DIGITAL")) {
        return "D";
    } else {
        return "Undefined";
    }
}


/* Convert symmetry type from station.xml to response. */
static char *convert_symmetry(const char *symmetry) {
    if (!strcmp(symmetry, "EVEN")) {
        return "C";
    } else if (!strcmp(symmetry, "ODD")) {
        return "B";
    } else {
        return "A";
    }
}


/* Display a pretty comment box. */
static int box(evalresp_logger *log, FILE *out, const char *title, const char UNUSED *net, const char UNUSED *stn,
        const x2r_channel UNUSED *channel) {

    int status = X2R_OK;
    char *ctitle = NULL, *csncl = NULL, *start = NULL, *end = NULL;

    if (!(status = centre(log, title, 35, &ctitle))) {
        if (!(status = centre_sncl(log, net, stn, channel, 35, &csncl))) {
            if (!(status = format_date_mdy(log, channel->start_date, &start))) {
                status = format_date_mdy(log, channel->end_date, &end);
            }
        }
    }

    if (!status) {
        if (!(status = lines(log, out,
                "#",
                "#                  +-----------------------------------+",
                NULL))) {
            if (!(status = line(log, out, "#                  |%s|", ctitle))) {
                if (!(status = line(log, out, "#                  |%s|", csncl))) {
                    if (!(status = line(log, out, "#                  |     %s to %s      |", start, end))) {
                        status = lines(log, out,
                                "#                  +-----------------------------------+",
                                "#",
                                NULL);
                    }
                }
            }
        }
    }

    free(csncl);
    free(ctitle);
    free(start);
    free(end);
    return status;
}


/* Print x2r_pole_zero. */
static int print_pole_zero(evalresp_logger *log, FILE *out, const char *tag, const char *name,
        int n, x2r_pole_zero *pole_zero) {

    int status = X2R_OK, i;

    if (n) {
        if (!(status = line(log, out, "#              Complex %s:", name))) {
            status = line(log, out, "#              i  real          imag          real_error    imag_error");
        }
        for (i = 0; !status && i < n; ++i) {
            x2r_pole_zero pz = pole_zero[i];
            status = line(log, out, "%s%6d  %+9.5E  %+9.5E  %+9.5E  %+9.5E",
                    tag, pz.number, pz.real.value, pz.imaginary.value,
                    pz.real.minus_error, pz.imaginary.minus_error);
        }
    }

    return status;
}


/* Print x2r_poles_zeros. */
static int print_poles_zeros(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK;

    if (!(status = box(log, out, "Response (Poles and Zeros)", net, stn, channel))) {
        if (!(status = line(log, out, "B053F03     Transfer function type:                %s",
                convert_tft(poles_zeros->pz_transfer_function_type)))) {
            if (!(status = line(log, out, "B053F04     Stage sequence number:                 %d",
                    stage))) {
                status = line(log, out, "B053F05     Response in units lookup:              %s - %s",
                        poles_zeros->input_units.name, poles_zeros->input_units.description);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B053F06     Response out units lookup:             %s - %s",
                poles_zeros->output_units.name, poles_zeros->output_units.description))) {
            if (!(status = line(log, out, "B053F07     A0 normalization factor:               %+9.5E",
                    poles_zeros->normalization_factor))) {
                status = line(log, out, "B053F08     Normalization frequency:               %+9.5E",
                        poles_zeros->normalization_frequency);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B053F09     Number of zeroes:                      %d",
                poles_zeros->n_zeros))) {
            if (!(status = line(log, out, "B053F14     Number of poles:                       %d",
                    poles_zeros->n_poles))) {
                status = print_pole_zero(log, out, "B053F10-13", "zeroes",
                        poles_zeros->n_zeros, poles_zeros->zero);
            }
        }
    }
    if (!status) {
        status = print_pole_zero(log, out, "B053F15-18", "poles",
            poles_zeros->n_poles, poles_zeros->pole);
    }

    return status;
}


/* Print x2r_float as coefficients. */
static int print_coefficient(evalresp_logger *log, FILE *out, const char *tag, const char *name,
        int n, x2r_float *coefficient) {

    int status = X2R_OK, i;

    if (n) {
        if (!(status = line(log, out, "#              %s coefficients:", name))) {
            if (!(status = line(log, out, "#              i  coefficient   error"))) {
                for (i = 0; !status && i < n; ++i) {
                    x2r_float cf = coefficient[i];
                    status = line(log, out, "%s%6d  %+9.5E  %+9.5E",
                            tag, i, cf.value, cf.minus_error);
                }
            }
        }
    }

    return status;
}


/* Print x2r_coefficients. */
static int print_coefficients(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_coefficients *coefficients) {

    int status = X2R_OK;

    if (!(status = box(log, out, "Response (Coefficients)", net, stn, channel))) {
        if (!(status = line(log, out, "B054F03     Transfer function type:                %s",
                convert_tft(coefficients->cf_transfer_function_type)))) {
            if (!(status = line(log, out, "B054F04     Stage sequence number:                 %d",
                    stage))) {
                status = line(log, out, "B054F05     Response in units lookup:              %s - %s",
                        coefficients->input_units.name, coefficients->input_units.description);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B054F06     Response out units lookup:             %s - %s",
                coefficients->output_units.name, coefficients->output_units.description))) {
            if (!(status = line(log, out, "B054F07     Number of numerators:                  %d",
                    coefficients->n_numerators))) {
                status = line(log, out, "B054F10     Number of denominators:                %d",
                        coefficients->n_denominators);
            }
        }
    }
    if (!status) {
        if (!(status = print_coefficient(log, out, "B054F08-09", "Numerator",
                coefficients->n_numerators, coefficients->numerator))) {
            status = print_coefficient(log, out, "B054F11-12", "Denominator",
                    coefficients->n_denominators, coefficients->denominator);
        }
    }

    return status;
}


/* Print x2r_response_list. */
static int print_response_list(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_response_list *response_list) {

    int status = X2R_OK, i;

    if (!(status = box(log, out, "Response List", net, stn, channel))) {
        if (!(status = line(log, out, "B055F03     Stage sequence number:                 %d",
                stage))) {
            if (!(status = line(log, out, "B055F04     Response in units lookup:              %s - %s",
                    response_list->input_units.name, response_list->input_units.description))) {
                status = line(log, out, "B055F05     Response out units lookup:             %s - %s",
                        response_list->output_units.name, response_list->output_units.description);
            }
        }
    }
    if (!status) {
        status = line(log, out, "B055F06     Number of responses listed:            %d",
                response_list->n_response_list_elements);
        if (!status && response_list->n_response_list_elements) {
            status = line(log, out, "#              i  frequency     amplitude     amplitude err phase angle   phase err");
            for (i = 0; !status && i < response_list->n_response_list_elements; ++i) {
                x2r_response_list_element elt = response_list->response_list_element[i];
                status = line(log, out, "B055F07-11     %-4d%+9.5E  %+9.5E  %+9.5E  %+9.5E  %+9.5E",
                        i+1, elt.frequency, elt.amplitude.value, elt.amplitude.minus_error,
                        elt.phase.value, elt.phase.minus_error);
            }
        }
    }

    return status;
}


/* Print x2r_fir. */
static int print_fir(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_fir *fir) {

    int status = X2R_OK, i;

    if (!(status = box(log, out, "FIR Response", net, stn, channel))) {
        if (!(status = line(log, out, "B061F03     Stage sequence number:                 %d",
                stage))) {
            if (!(status = line(log, out, "B061F04     Response Name:                         %s",
                    fir->name))) {
                status = line(log, out, "B061F05     Symmetry Code:                         %s",
                        convert_symmetry(fir->symmetry));
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B061F06     Response in units lookup:              %s - %s",
                fir->input_units.name, fir->input_units.description))) {
            if (!(status = line(log, out, "B061F07     Response out units lookup:             %s - %s",
                    fir->output_units.name, fir->output_units.description))) {
                status = line(log, out, "B061F08     Number of Coefficients:                %d",
                        fir->n_numerator_coefficients);
            }
        }
    }
    if (!status && fir->n_numerator_coefficients) {
        status = line(log, out, "#              i  FIR Coefficient");
        for (i = 0; !status && i < fir->n_numerator_coefficients; ++i) {
            x2r_numerator_coefficient nc = fir->numerator_coefficient[i];
            status = line(log, out, "B061F09%6d  %+9.5E", i, nc.value);
        }
    }

    return status;
}


/* Print x2r_polynomial. */
static int print_polynomial(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_polynomial *polynomial) {

    int status = X2R_OK, i;

    if (!(status = box(log, out, "Response (Polynomial)", net, stn, channel))) {
        if (!(status = line(log, out, "B062F03     Transfer function type:                P"))) {
            if (!(status = line(log, out, "B062F04     Stage sequence number:                 %d",
                    stage))) {
                status = line(log, out, "B062F05     Response in units lookup:              %s - %s",
                        polynomial->input_units.name, polynomial->input_units.description);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B062F06     Response out units lookup:             %s - %s",
                polynomial->output_units.name, polynomial->output_units.description))) {
            if (!(status = line(log, out, "B062F07     Polynomial Approximation Type:         M"))) {
                status = line(log, out, "B062F08     Valid Frequency Units:                 B");
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B062F09     Lower Valid Frequency Bound:           %+9.5E",
                polynomial->frequency_lower_bound))) {
            if (!(status = line(log, out, "B062F10     Upper Valid Frequency Bound:           %+9.5E",
                    polynomial->frequency_upper_bound))) {
                status = line(log, out, "B062F11     Lower Bound of Approximation:          %+9.5E",
                        polynomial->approximation_lower_bound);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B062F12     Upper Bound of Approximation:          %+9.5E",
                polynomial->approximation_upper_bound))) {
            if (!(status = line(log, out, "B062F13     Maximum Absolute Error:                %+9.5E",
                    polynomial->maximum_error))) {
                status = line(log, out, "B062F14     Number of Coefficients:                %d",
                        polynomial->n_coefficients);
            }
        }
    }

    // there are subtle differences with print_coefficient, as in IRIS_WS
    if (!status && polynomial->n_coefficients) {
        if (!(status = line(log, out, "#              Polynomial coefficients:"))) {
            status = line(log, out, "#              i, coefficient,  error");
            for (i = 0; !status && i < polynomial->n_coefficients; ++i) {
                x2r_coefficient cf = polynomial->coefficient[i];
                status = line(log, out, "B062F15-16%6d   %+9.5E  %+9.5E",
                        cf.number, cf.value.value, cf.value.minus_error);
            }
        }
    }

    return status;
}


/* Print x2r_decimation. */
static int print_decimation(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_decimation *decimation) {

    int status = X2R_OK;

    if (!(status = box(log, out, "Decimation", net, stn, channel))) {
        // extra spacing below is in IRIS-WS
        if (!(status = line(log, out, "B057F03     Stage sequence number:                  %d",
                stage))) {
            // unusual format below is in IRIS-WS
            status = line(log, out, "B057F04     Input sample rate (HZ):                 %8.4E",
                    decimation->input_sample_rate);
        }
    }
    if (!status) {
        // again, in IRIS-WS
        if (decimation->factor) {
            status = line(log, out, "B057F05     Decimation factor:                      %05d",
                    decimation->factor);
        } else {
            status = line(log, out, "B057F05     Decimation factor:                      %d",
                decimation->factor);
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B057F06     Decimation offset:                      %05d",
                decimation->offset))) {
            if (!(status = line(log, out, "B057F07     Estimated delay (seconds):             %+8.4E",
                    decimation->delay))) {
                status = line(log, out, "B057F08     Correction applied (seconds):          %+8.4E",
                        decimation->correction);
            }
        }
    }

    return status;
}


/* Print x2r_gain. */
static int print_stage_gain(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, int stage, const x2r_gain *gain) {

    int status = X2R_OK;

    // leading space below is in IRIS-WS
    if (!(status = box(log, out, " Channel Sensitivity/Gain", net, stn, channel))) {
        if (!(status = line(log, out, "B058F03     Stage sequence number:                 %d",
                stage))) {
            status = line(log, out, "B058F04     Sensitivity:                           %+9.5E",
                    gain->value);
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B058F05     Frequency of sensitivity:              %+9.5E",
                gain->frequency))) {
            status = line(log, out, "B058F06     Number of calibrations:                0");
        }
    }

    return status;
}


/* Print x2r_stage. */
static int print_stage(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, const x2r_stage *stage) {

    int status = X2R_OK;

    switch (stage->type) {
    case X2R_STAGE_POLES_ZEROS:
        status = print_poles_zeros(log, out, net, stn, channel, stage->number,
                stage->u.poles_zeros);
        break;
    case X2R_STAGE_COEFFICIENTS:
        status = print_coefficients(log, out, net, stn, channel, stage->number,
                stage->u.coefficients);
        break;
    case X2R_STAGE_RESPONSE_LIST:
        status = print_response_list(log, out, net, stn, channel, stage->number,
                stage->u.response_list);
        break;
    case X2R_STAGE_FIR:
        status = print_fir(log, out, net, stn, channel, stage->number, stage->u.fir);
        break;
    case X2R_STAGE_POLYNOMIAL:
        status = print_polynomial(log, out, net, stn, channel, stage->number,
                stage->u.polynomial);
        break;
    default:
        evalresp_log(log, EV_WARN, 0, "No content in stage (during print)");
        break;
    }

    if (!status && stage->decimation) {
        status = print_decimation(log, out, net, stn, channel, stage->number,
                stage->decimation);
    }

    if (!status && stage->stage_gain) {
        status = print_stage_gain(log, out, net, stn, channel, stage->number,
                stage->stage_gain);
    }

    return status;
}


/* Print x2r_response. */
static int print_response(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel, const x2r_response *response) {

    int status = X2R_OK, i;

    for (i = 0; !status && i < response->n_stages; ++i) {
        status = print_stage(log, out, net, stn, channel, &response->stage[i]);
    }

    // the check on value is not in the IRIS-WS code, but the logic appears to be there
    // perhaps there's a failure to unserialize when the value is missing?
    if (!status && response->instrument_sensitivity && response->instrument_sensitivity->value != 0) {
        status = print_stage_gain(log, out, net, stn, channel, 0,
                response->instrument_sensitivity);
    }

    if (!status && response->instrument_polynomial) {
        status = print_polynomial(log, out, net, stn, channel, 0,
                response->instrument_polynomial);
    }

    return status;
}


/* Print x2r_channel. */
static int print_channel(evalresp_logger *log, FILE *out, const char *net, const char *stn,
        const x2r_channel *channel) {

    int status = X2R_OK;
    char *start = NULL, *end = NULL;

    if (!(status = lines(log, out,
            "#",
            "###################################################################################",
            "#",
            NULL
            ))) {
        if (!(status = line(log, out, "B050F03     Station:     %s", stn))) {
            if (!(status = line(log, out, "B050F16     Network:     %s", net))) {
                status = line(log, out, "B052F03     Location:    %s",
                        (!(strcmp(channel->location_code, "  ") && strcmp(channel->location_code, ""))) ?
                                "??" : channel->location_code);
            }
        }
    }
    if (!status) {
        if (!(status = line(log, out, "B052F04     Channel:     %s", channel->code))) {
            if (!(status = format_date_yjhms(log, channel->start_date, &start))) {
                status = line(log, out, "B052F22     Start date:  %s", start);
            }
        }
    }
    if (!status) {
        if (!(status = format_date_yjhms(log, channel->end_date, &end))) {
            if (!(status = line(log, out, "B052F23     End date:    %s", end))) {
                status = print_response(log, out, net, stn, channel, &channel->response);
            }
        }
    }

    free(start);
    free(end);
    return status;
}


/*
 * Print the entire response document, given the in-memory model.
 */
int x2r_resp_util_write(evalresp_logger *log, FILE *out, const x2r_fdsn_station_xml *root) {

    int status = X2R_OK, i, j, k;
    x2r_network network;
    x2r_station station;

    for (i = 0; !status && i < root->n_networks; ++i) {
        network = root->network[i];
        for (j = 0; !status && j < network.n_stations; ++j) {
            station = network.station[j];
            for (k = 0; !status && k < station.n_channels; ++k) {
                status = print_channel(log, out, network.code, station.code,
                        &station.channel[k]);
            }
        }
    }

    return status;
}

int x2r_detect_xml(FILE *in, int *xml_flag) {

    int status = X2R_OK;
    int character = 0;

    while (character > -1) {
        character = fgetc(in);
        if (character > -1) {
            switch ((char)character) {
            // on space or newline, keep reading
            case ' ':
            case '\n':
            case '\r':
                break;
            // a < means xml
            case '<':
                *xml_flag = 1;
                character = -1;
                break;
            // anything else means SEED
            default:
                *xml_flag = 0;
                character = -1;
                break;
            }
        }
    }

// doesn't work well on OSX?
//  rewind(*in);
    fseek(in, 0, SEEK_SET);
    return status;
}
