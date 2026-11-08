
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <evalresp/constants.h>
#include <evalresp/input.h>
#include <evalresp/private.h>
#include <evalresp/public_api.h>
#include <evalresp/stationxml2resp/dom_to_channels.h>
#include <evalresp_log/log.h>


// This file builds evalresp channels from the in-memory model of the
// station.xml document (created in xml_to_dom.c).  The channels are those that
// reading the response document written by dom_to_seed.c would give (see
// read_channel_header and read_channel_data in input.c), without the
// intermediate formatting of values.


#define OPEN_END "No Ending Time"

/* The stage being filled. */
typedef struct {
    evalresp_channel *channel;
    evalresp_stage *stage;
    evalresp_blkt *last_blkt;
    int last_seq_no;
    int n_blkt;
    int no_units;
} builder;

/* Units of a filter, until they are assigned to a stage. */
typedef struct {
    int in;
    int out;
    char *in_str;
    char *out_str;
} stage_units;


static void release_units(stage_units *units) {
    free(units->in_str);
    free(units->out_str);
    memset(units, 0, sizeof(*units));
}


/* Describe and classify the units as the response document does. */
static int make_units(evalresp_logger *log, evalresp_options const *options, evalresp_channel *channel,
        const x2r_units *in, const x2r_units *out, stage_units *units) {

    char line[MAXLINELEN];

    memset(units, 0, sizeof(*units));

    snprintf(line, sizeof(line), "%s - %s", in->name, in->description);
    if (!(units->in_str = strdup(line))) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate input units");
        return EVALRESP_MEM;
    }
    (void)parse_units(log, options, line, channel, &units->in);

    snprintf(line, sizeof(line), "%s - %s", out->name, out->description);
    if (!(units->out_str = strdup(line))) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate output units");
        release_units(units);
        return EVALRESP_MEM;
    }
    (void)parse_units(log, options, line, channel, &units->out);

    return EVALRESP_OK;
}


/*
 * Append a blockette to the stage with the given sequence number, starting a
 * new stage when the number differs from the previous blockette.  Takes
 * ownership of the blockette (which must have no arrays allocated yet, so
 * that it can be freed on failure) and of the units, which are given to the
 * stage if it has none.  Decimation and gain have no units.
 */
static int append_blkt(evalresp_logger *log, builder *b, evalresp_blkt *blkt, int seq_no,
        stage_units *units) {

    evalresp_stage *next;

    if (!b->n_blkt) {
        b->stage->first_blkt = blkt;
        b->stage->sequence_no = seq_no;
        b->no_units = 1;
    } else if (b->last_seq_no != seq_no) {
        if (!(next = alloc_stage(log))) {
            free(blkt);
            if (units) release_units(units);
            return EVALRESP_MEM;
        }
        next->sequence_no = seq_no;
        next->first_blkt = blkt;
        b->stage->next_stage = next;
        b->stage = next;
        b->channel->nstages++;
        b->no_units = 1;
    } else {
        b->last_blkt->next_blkt = blkt;
    }
    b->n_blkt++;

    if (units) {
        if (b->no_units) {
            b->stage->input_units = units->in;
            b->stage->output_units = units->out;
            b->stage->input_units_str = units->in_str;
            b->stage->output_units_str = units->out_str;
            b->no_units = 0;
        } else {
            free(units->in_str);
            free(units->out_str);
        }
        memset(units, 0, sizeof(*units));
    }

    b->last_blkt = blkt;
    b->last_seq_no = seq_no;
    return EVALRESP_OK;
}


/* Allocate, classify and append a filter blockette. */
static int append_filter(evalresp_logger *log, evalresp_options const *options, builder *b,
        evalresp_blkt *(*alloc)(evalresp_logger *), int type, int seq_no,
        const x2r_units *in, const x2r_units *out, evalresp_blkt **blkt) {

    int status;
    stage_units units;

    if (!(*blkt = alloc(log))) return EVALRESP_MEM;
    (*blkt)->type = type;
    if ((status = make_units(log, options, b->channel, in, out, &units))) {
        free(*blkt);
        *blkt = NULL;
        return status;
    }
    return append_blkt(log, b, *blkt, seq_no, &units);
}


/* Transfer function type code as in the response document. */
static char transfer_function_code(const char *type) {
    if (!strcmp(type, "LAPLACE (RADIANS/SECOND)")) return 'A';
    if (!strcmp(type, "LAPLACE (HERTZ)")) return 'B';
    if (!strcmp(type, "DIGITAL (Z-TRANSFORM)") || !strcmp(type, "DIGITAL")) return 'D';
    return 0;
}


static int alloc_failed(evalresp_logger *log, const char *name, int n, const void *array) {
    if (n && !array) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate %s", name);
        return 1;
    }
    return 0;
}


static int append_poles_zeros(evalresp_logger *log, evalresp_options const *options, builder *b,
        int seq_no, const x2r_poles_zeros *pz) {

    int status, i, type;
    evalresp_blkt *blkt;
    evalresp_pole_zero *out;

    switch (transfer_function_code(pz->pz_transfer_function_type)) {
    case 'A': type = LAPLACE_PZ; break;
    case 'B': type = ANALOG_PZ; break;
    case 'D': type = IIR_PZ; break;
    default:
        evalresp_log(log, EV_ERROR, EV_ERROR, "Unexpected pole/zero transfer function type '%s'",
                pz->pz_transfer_function_type);
        return EVALRESP_PAR;
    }

    if ((status = append_filter(log, options, b, alloc_pz, type, seq_no,
            &pz->input_units, &pz->output_units, &blkt))) return status;

    out = &blkt->blkt_info.pole_zero;
    out->a0 = pz->normalization_factor;
    out->a0_freq = pz->normalization_frequency;
    out->nzeros = pz->n_zeros;
    out->npoles = pz->n_poles;
    out->zeros = alloc_complex(pz->n_zeros, log);
    out->poles = alloc_complex(pz->n_poles, log);
    if (alloc_failed(log, "zeros", pz->n_zeros, out->zeros) ||
            alloc_failed(log, "poles", pz->n_poles, out->poles)) return EVALRESP_MEM;

    for (i = 0; i < pz->n_zeros; ++i) {
        out->zeros[i].real = pz->zero[i].real.value;
        out->zeros[i].imag = pz->zero[i].imaginary.value;
    }
    for (i = 0; i < pz->n_poles; ++i) {
        out->poles[i].real = pz->pole[i].real.value;
        out->poles[i].imag = pz->pole[i].imaginary.value;
    }

    return EVALRESP_OK;
}


static int append_coefficients(evalresp_logger *log, evalresp_options const *options, builder *b,
        int seq_no, const x2r_coefficients *cf) {

    int status, i;
    evalresp_blkt *blkt;

    if (transfer_function_code(cf->cf_transfer_function_type) != 'D') {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Unsupported coefficients transfer function type '%s'",
                cf->cf_transfer_function_type);
        return EVALRESP_PAR;
    }

    if (cf->n_denominators && !cf->n_numerators) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Coefficients of stage %d have denominators but no numerators", seq_no);
        return EVALRESP_PAR;
    }

    if (cf->n_denominators) {
        evalresp_coeff *out;
        if ((status = append_filter(log, options, b, alloc_coeff, IIR_COEFFS, seq_no,
                &cf->input_units, &cf->output_units, &blkt))) return status;
        out = &blkt->blkt_info.coeff;
        out->nnumer = cf->n_numerators;
        out->ndenom = cf->n_denominators;
        out->numer = alloc_double(cf->n_numerators, log);
        out->denom = alloc_double(cf->n_denominators, log);
        if (alloc_failed(log, "numerators", cf->n_numerators, out->numer) ||
                alloc_failed(log, "denominators", cf->n_denominators, out->denom)) return EVALRESP_MEM;
        for (i = 0; i < cf->n_numerators; ++i) out->numer[i] = cf->numerator[i].value;
        for (i = 0; i < cf->n_denominators; ++i) out->denom[i] = cf->denominator[i].value;
    } else {
        evalresp_fir *out;
        if ((status = append_filter(log, options, b, alloc_fir, FIR_ASYM, seq_no,
                &cf->input_units, &cf->output_units, &blkt))) return status;
        out = &blkt->blkt_info.fir;
        out->ncoeffs = cf->n_numerators;
        out->coeffs = alloc_double(cf->n_numerators, log);
        if (alloc_failed(log, "coefficients", cf->n_numerators, out->coeffs)) return EVALRESP_MEM;
        for (i = 0; i < cf->n_numerators; ++i) out->coeffs[i] = cf->numerator[i].value;
    }

    return EVALRESP_OK;
}


static int append_response_list(evalresp_logger *log, evalresp_options const *options, builder *b,
        int seq_no, const x2r_response_list *list) {

    int status, i;
    evalresp_blkt *blkt;
    evalresp_list *out;

    if (list->n_response_list_elements < 1) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "ResponseList of stage %d has no elements", seq_no);
        return EVALRESP_PAR;
    }

    if ((status = append_filter(log, options, b, alloc_list, LIST, seq_no,
            &list->input_units, &list->output_units, &blkt))) return status;

    out = &blkt->blkt_info.list;
    out->nresp = list->n_response_list_elements;
    out->freq = alloc_double(out->nresp, log);
    out->amp = alloc_double(out->nresp, log);
    out->phase = alloc_double(out->nresp, log);
    if (alloc_failed(log, "response list frequencies", out->nresp, out->freq) ||
            alloc_failed(log, "response list amplitudes", out->nresp, out->amp) ||
            alloc_failed(log, "response list phases", out->nresp, out->phase)) return EVALRESP_MEM;

    for (i = 0; i < out->nresp; ++i) {
        out->freq[i] = list->response_list_element[i].frequency;
        out->amp[i] = list->response_list_element[i].amplitude.value;
        out->phase[i] = list->response_list_element[i].phase.value;
    }

    return EVALRESP_OK;
}


static int append_fir(evalresp_logger *log, evalresp_options const *options, builder *b,
        int seq_no, const x2r_fir *fir) {

    int status, i, type;
    evalresp_blkt *blkt;
    evalresp_fir *out;

    if (!strcmp(fir->symmetry, "EVEN")) {
        type = FIR_SYM_2;
    } else if (!strcmp(fir->symmetry, "ODD")) {
        type = FIR_SYM_1;
    } else {
        type = FIR_ASYM;
    }

    if ((status = append_filter(log, options, b, alloc_fir, type, seq_no,
            &fir->input_units, &fir->output_units, &blkt))) return status;

    out = &blkt->blkt_info.fir;
    out->ncoeffs = fir->n_numerator_coefficients;
    out->coeffs = alloc_double(out->ncoeffs, log);
    if (alloc_failed(log, "FIR coefficients", out->ncoeffs, out->coeffs)) return EVALRESP_MEM;
    for (i = 0; i < out->ncoeffs; ++i) out->coeffs[i] = fir->numerator_coefficient[i].value;

    return EVALRESP_OK;
}


static int append_polynomial(evalresp_logger *log, evalresp_options const *options, builder *b,
        int seq_no, const x2r_polynomial *polynomial) {

    int status, i;
    evalresp_blkt *blkt;
    evalresp_polynomial *out;

    if ((status = append_filter(log, options, b, alloc_polynomial, POLYNOMIAL, seq_no,
            &polynomial->input_units, &polynomial->output_units, &blkt))) return status;

    out = &blkt->blkt_info.polynomial;
    out->approximation_type = 'M';
    out->frequency_units = 'B';
    out->lower_freq_bound = polynomial->frequency_lower_bound;
    out->upper_freq_bound = polynomial->frequency_upper_bound;
    out->lower_approx_bound = polynomial->approximation_lower_bound;
    out->upper_approx_bound = polynomial->approximation_upper_bound;
    out->max_abs_error = polynomial->maximum_error;
    out->ncoeffs = polynomial->n_coefficients;
    out->coeffs = calloc(out->ncoeffs, sizeof(*out->coeffs));
    out->coeffs_err = calloc(out->ncoeffs, sizeof(*out->coeffs_err));
    if (alloc_failed(log, "polynomial coefficients", out->ncoeffs, out->coeffs) ||
            alloc_failed(log, "polynomial coefficient errors", out->ncoeffs, out->coeffs_err)) return EVALRESP_MEM;

    for (i = 0; i < out->ncoeffs; ++i) {
        out->coeffs[i] = polynomial->coefficient[i].value.value;
        out->coeffs_err[i] = polynomial->coefficient[i].value.minus_error;
    }

    return EVALRESP_OK;
}


static int append_decimation(evalresp_logger *log, builder *b, int seq_no,
        const x2r_decimation *decimation) {

    int status;
    evalresp_blkt *blkt;
    evalresp_decimation *out;

    if (!(blkt = alloc_deci(log))) return EVALRESP_MEM;
    if ((status = append_blkt(log, b, blkt, seq_no, NULL))) return status;

    out = &blkt->blkt_info.decimation;
    if (decimation->input_sample_rate) out->sample_int = 1.0 / decimation->input_sample_rate;
    out->deci_fact = decimation->factor;
    out->deci_offset = decimation->offset;
    out->estim_delay = decimation->delay;
    out->applied_corr = decimation->correction;

    return EVALRESP_OK;
}


static int append_gain(evalresp_logger *log, builder *b, int seq_no, const x2r_gain *gain) {

    int status;
    evalresp_blkt *blkt;

    if (!(blkt = alloc_gain(log))) return EVALRESP_MEM;
    if ((status = append_blkt(log, b, blkt, seq_no, NULL))) return status;

    blkt->blkt_info.gain.gain = gain->value;
    blkt->blkt_info.gain.gain_freq = gain->frequency;

    return EVALRESP_OK;
}


static int append_stage(evalresp_logger *log, evalresp_options const *options, builder *b,
        const x2r_stage *stage) {

    int status = EVALRESP_OK;

    switch (stage->type) {
    case X2R_STAGE_POLES_ZEROS:
        status = append_poles_zeros(log, options, b, stage->number, stage->u.poles_zeros);
        break;
    case X2R_STAGE_COEFFICIENTS:
        status = append_coefficients(log, options, b, stage->number, stage->u.coefficients);
        break;
    case X2R_STAGE_RESPONSE_LIST:
        status = append_response_list(log, options, b, stage->number, stage->u.response_list);
        break;
    case X2R_STAGE_FIR:
        status = append_fir(log, options, b, stage->number, stage->u.fir);
        break;
    case X2R_STAGE_POLYNOMIAL:
        status = append_polynomial(log, options, b, stage->number, stage->u.polynomial);
        break;
    default:
        break;
    }

    if (!status && stage->decimation) {
        status = append_decimation(log, b, stage->number, stage->decimation);
    }
    if (!status && stage->stage_gain) {
        status = append_gain(log, b, stage->number, stage->stage_gain);
    }

    return status;
}


static int append_response(evalresp_logger *log, evalresp_options const *options, builder *b,
        const x2r_response *response) {

    int status = EVALRESP_OK, i;

    for (i = 0; !status && i < response->n_stages; ++i) {
        status = append_stage(log, options, b, &response->stage[i]);
    }
    if (!status && response->instrument_sensitivity && response->instrument_sensitivity->value != 0) {
        status = append_gain(log, b, 0, response->instrument_sensitivity);
    }
    if (!status && response->instrument_polynomial) {
        status = append_polynomial(log, options, b, 0, response->instrument_polynomial);
    }

    return status;
}


/* Copy text, always terminating the destination. */
static void copy_text(char *destn, size_t size, const char *source) {
    snprintf(destn, size, "%s", source);
}


/* Format an epoch as year,julian day,time (or open when unset). */
static int format_date(evalresp_logger *log, time_t epoch, char *destn) {

    struct tm *tm;
    char text[DATIMLEN];

    if (epoch == unset_time_t) {
        copy_text(destn, DATIMLEN, OPEN_END);
    } else if (!(tm = gmtime(&epoch)) || !strftime(text, sizeof(text), "%Y,%j,%H:%M:%S", tm)) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot format date");
        return EVALRESP_PAR;
    } else {
        copy_text(destn, DATIMLEN, text);
    }

    return EVALRESP_OK;
}


static int blank(const char *text) {
    return !text[strspn(text, " ")];
}


static int build_channel(evalresp_logger *log, evalresp_options const *options,
        const char *network, const char *station, const x2r_channel *xc, evalresp_channel **channel) {

    int status;
    builder b;
    evalresp_log_context ctx;

    if (!(*channel = calloc(1, sizeof(**channel)))) {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate memory for channel");
        return EVALRESP_MEM;
    }
    if (!(b.stage = alloc_stage(log))) return EVALRESP_MEM;

    copy_text((*channel)->staname, STALEN, station);
    copy_text((*channel)->network, NETLEN, network);
    copy_text((*channel)->locid, LOCIDLEN, blank(xc->location_code) ? "" : xc->location_code);
    copy_text((*channel)->chaname, CHALEN, xc->code);

    (*channel)->unit_scale_fact = 1.0;
    (*channel)->first_stage = b.stage;
    (*channel)->nstages = 1;
    b.channel = *channel;
    b.last_blkt = NULL;
    b.last_seq_no = 0;
    b.n_blkt = 0;
    b.no_units = 0;

    evalresp_log_context_init(&ctx, log, "%s.%s.%s.%s", (*channel)->network,
            (*channel)->staname, (*channel)->locid, (*channel)->chaname);
    log = &ctx.logger;

    if (!(status = format_date(log, xc->start_date, (*channel)->beg_t))) {
        if (!(status = format_date(log, xc->end_date, (*channel)->end_t))) {
            status = append_response(log, options, &b, &xc->response);
        }
    }

    return status;
}


/*
 * Build the channels of the whole document.
 */
int x2r_to_channels(evalresp_logger *log, const x2r_fdsn_station_xml *root,
        evalresp_options const *options, evalresp_channels **channels) {

    int status, i, j, k;
    const x2r_network *network;
    const x2r_station *station;
    evalresp_channel *channel;

    *channels = NULL;
    if (!(status = evalresp_alloc_channels(log, channels))) {
        for (i = 0; !status && i < root->n_networks; ++i) {
            network = &root->network[i];
            for (j = 0; !status && j < network->n_stations; ++j) {
                station = &network->station[j];
                for (k = 0; !status && k < station->n_channels; ++k) {
                    channel = NULL;
                    if (!(status = build_channel(log, options, network->code, station->code,
                            &station->channel[k], &channel))) {
                        if (!(status = add_channel(log, channel, *channels))) {
                            channel = NULL;
                        }
                    }
                    evalresp_free_channel(&channel);
                }
            }
        }
    }

    if (status) {
        evalresp_free_channels(channels);
    }

    return status;
}
