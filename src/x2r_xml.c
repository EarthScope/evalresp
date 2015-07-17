
#include <string.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "x2r.h"
#include "x2r_xml.h"

#define BUFFER_SIZE 5000
// this is associated with "stn" which should be used in all queries
#define STATION_URI "http://www.fdsn.org/xml/station/1"


// This file builds an in-memory model of the station.xml document.
// Only those parts needed to print the response document (in x2r_ws.c)
// are generated.

// http://stackoverflow.com/questions/16647819/timegm-cross-platform
#ifdef _WIN32
#define timegm _mkgmtime
#endif

/** Support routine for stream2doc */
static int ioread(FILE *f, char * buf, int len) {
    return(fread(buf, 1, len, f));
}

/** Support routine for stream2doc */
static void ioclose(FILE UNUSED *f) {
    // we don't open or close in this lib; we just process streams
}

/** Read an XML file from the given stream and create an in-memory model. */
static int stream2doc(x2r_log *log, FILE *in, xmlDocPtr *doc) {

    int status = X2R_OK;
    xmlParserCtxtPtr ctxt = NULL;

    if (!(ctxt = xmlCreateIOParserCtxt(NULL, NULL,
            (xmlInputReadCallback)ioread, (xmlInputCloseCallback)ioclose,
            in, XML_CHAR_ENCODING_UTF8))) {
        status = x2r_error(log, X2R_ERR_XML2, "Could not create parser context");
        goto exit;
    }

    if (!(*doc = xmlCtxtReadFd(ctxt, fileno(in), NULL, "utf8", 0))) {
        status = x2r_error(log, X2R_ERR_XML2, "Could not parse input");
        goto exit;
    }

exit:
    if (ctxt) xmlFreeParserCtxt(ctxt);
    return status;
}


/** Underlying xpath search logic for routines below. */
static int vfind_xpaths(x2r_log *log, xmlDocPtr doc, xmlXPathObjectPtr *xpath, int *count,
        xmlNodePtr from, const char *template, va_list argp) {

    int status = X2R_OK, len, n;
    xmlXPathContextPtr xpath_ctx = NULL;
    char buffer[BUFFER_SIZE];

    len = vsnprintf(buffer, BUFFER_SIZE, template, argp);
    if (len < 0 || len >= BUFFER_SIZE) {
        status = x2r_error(log, X2R_ERR_BUFFER, "Buffer size exceeded");
        goto exit;
    }

    if (!(xpath_ctx = xmlXPathNewContext(doc))) {
        status = x2r_error(log, X2R_ERR_XML2, "Cannot create xpath context");
        goto exit;
    }
    if (-1 == xmlXPathRegisterNs(xpath_ctx, BAD_CAST "stn", BAD_CAST STATION_URI)) {
        status = x2r_error(log, X2R_ERR_XML2, "Cannot register namespace with xpath");
        goto exit;
    }
    if (from) xpath_ctx->node = from;
    if (!(*xpath = xmlXPathEvalExpression(BAD_CAST buffer, xpath_ctx))) {
        status = x2r_error(log, X2R_ERR_XML2, "Could not evaluate %s", buffer);
        goto exit;
    }
    n = (*xpath)->nodesetval ? (*xpath)->nodesetval->nodeNr : 0;
    x2r_debug(log, "Found %d nodes for %s", n, buffer);
    if (count) *count = n;

exit:
    if (xpath_ctx) xmlXPathFreeContext(xpath_ctx);
    return status;
}


/** Find 0-many nodes matching a given xpath expression. */
static int find_xpaths(x2r_log *log, xmlDocPtr doc, xmlXPathObjectPtr *xpath, int *count,
        xmlNodePtr from, const char *template, ...) {

    int status = X2R_OK;
    va_list argp;

    va_start(argp, template);
    if ((status = vfind_xpaths(log, doc, xpath, count, from, template, argp))) goto exit;

exit:
    va_end(argp);
    return status;
}


/**
 * Find 0-1 nodes matching an xpath expression.
 *
 * If found is NULL then the xpath is assumed required, and it is an error if none
 * was found.
 *
 * If multiple values are found, it is an error.
 */
static int find_xpath(x2r_log *log, xmlDocPtr doc, xmlNodePtr *result, int* found,
        xmlNodePtr from, const char *template, ...) {

    int status = X2R_OK, count;
    va_list argp;
    xmlXPathObjectPtr xpath = NULL;

    va_start(argp, template);
    if ((status = vfind_xpaths(log, doc, &xpath, &count, from, template, argp))) goto exit;

    if (count > 1) {
        status = x2r_error(log, X2R_ERR_XML, "Multiple results for %s", template);
        goto exit;
    }
    if (found) {
        *found = count;
    } else {
        if (!count) {
            status = x2r_error(log, X2R_ERR_XML, "No result for %s", template);
            goto exit;
        }
    }

    if (count) {
        *result = xpath->nodesetval->nodeTab[0];
    } else {
        *result = NULL;
    }

exit:
    if (xpath) xmlXPathFreeObject(xpath);
    va_end(argp);
    return status;
}


/** Read the text contents of an element as a char*. */
static int char_element(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, const char *name,
        const char *deflt, char **value) {

    int status = X2R_OK, found;
    xmlNodePtr element, child;

    if ((status = find_xpath(log, doc, &element, &found, node, name))) goto exit;
    if (found) {
        child = element->children;
        while (child && child->type != XML_TEXT_NODE) child = child->next;
        if (!child) {
            status = x2r_error(log, X2R_ERR_XML, "No text for %s", name);
            goto exit;
        }
        if (!(*value = strdup((char*) child->content))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot copy text for %s", name);
            goto exit;
        }
    } else {
        if (deflt) {
            if (!(*value = strdup(deflt))) {
                status = x2r_error(log, X2R_ERR_MEMORY, "Cannot copy default for %s", name);
                goto exit;
            }
        } else {
            status = x2r_error(log, X2R_ERR_XML, "Missing %s element", name);
            goto exit;
        }
    }

exit:
    return status;
}


/** Read the text contents of an element as an int. */
static int int_element(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, const char *name,
        const char *deflt, int *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if ((status = char_element(log, doc, node, name, deflt, &text))) goto exit;
    *value = strtol(text, &end, 10);
    if (*end) {  // should point to end of string
        status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
        goto exit;
    }

exit:
    free(text);
    return status;
}


/** Read the text contents of an element as a double. */
static int double_element(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, const char *name,
        const char *deflt, double *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if ((status = char_element(log, doc, node, name, deflt, &text))) goto exit;
    *value = strtod(text, &end);
    if (*end) {  // should point to end of string
        status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
        goto exit;
    }

exit:
    free(text);
    return status;
}


/** Read an attribute value as a char*. */
static int char_attribute(x2r_log *log, xmlNodePtr node, const char *name,
        const char *deflt, char **value) {

    xmlChar *attr;

    if (!(attr = xmlGetProp(node, BAD_CAST name))) {
        if (deflt) {
            *value = strdup(deflt);
            return X2R_OK;
        } else {
            return x2r_error(log, X2R_ERR_XML, "Could not find %s in %s", name, node->name);
        }
    } else {
        *value = (char*) attr;
        return X2R_OK;
    }
}

/**
 * Parse an ISO yyyy-mm-ddThh:mm:ss format datetime.
 *
 * Separated out for testing (see check_parse_datetime and also implicit tests against
 * the IRIS-WS code elsewhere).  Originally used strptime, but that's not available on
 * Windows.
 */
int x2r_parse_iso_datetime(x2r_log *log, const char *datetime, time_t *epoch) {

    int status = X2R_OK, year, month;
    struct tm tm;

    if (!(6 == sscanf(datetime, "%d-%d-%dT%d:%d:%d",
    		&year, &month, &tm.tm_mday,
    		&tm.tm_hour, &tm.tm_min, &tm.tm_sec))) {
    	status = x2r_error(log, X2R_ERR_XML, "Could not parse %s", datetime);
        goto exit;
    }
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_isdst = 0;
    // these two are bsd extensions which we're not using
//    tm.tm_gmtoff = 0;
//    tm.tm_zone = "UTC";
    // these two are wrong, but ignored (actually, set) by mktime
    tm.tm_wday = 0;
    tm.tm_yday = 0;

    *epoch = timegm(&tm);

exit:
    return status;
}


/** Read an attribute value as an ISO formatted epoch. */
static int datetime_attribute(x2r_log *log, xmlNodePtr node, const char *name, time_t *epoch) {

    int status = X2R_OK;
    char *value = NULL;

    if ((status = char_attribute(log, node, name, NULL, &value))) goto exit;
    if ((status = x2r_parse_iso_datetime(log, value, epoch))) goto exit;

exit:
    free(value);
    return status;
}


/** Read an attribute value as an int. */
static int int_attribute(x2r_log *log, xmlNodePtr node, const char *name,
        const char *deflt, int *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if ((status = char_attribute(log, node, name, deflt, &text))) goto exit;
    *value = strtol(text, &end, 10);
    if (*end) {  // should point to end of string
        status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
        goto exit;
    }

exit:
    free(text);
    return status;
}


/** Read an attribute value as double. */
static int double_attribute(x2r_log *log, xmlNodePtr node, const char *name, double *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if ((status = char_attribute(log, node, name, NULL, &text))) goto exit;
    *value = strtod(text, &end);
    if (*end) {  // should point to end of string
        status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
        goto exit;
    }

exit:
    free(text);
    return status;
}


/** Read an x2r_float value from the given xpath. */
static int parse_float(x2r_log *log, xmlDocPtr doc, xmlNodePtr parent, const char *xpath,
        x2r_float *flt) {

    int status = X2R_OK;
    xmlNodePtr node;

    x2r_debug(log, "Parsing float");

    if ((status = find_xpath(log, doc, &node, NULL, parent, xpath))) goto exit;
    if ((status = double_element(log, doc, node, ".", NULL, &flt->value))) goto exit;
    if ((status = double_attribute(log, node, "plusError", &flt->plus_error))) goto exit;
    if ((status = double_attribute(log, node, "minusError", &flt->minus_error))) goto exit;

exit:
    return status;
}


/** Read an x2r_pole_zero value from the current node. */
static int parse_pole_zero(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_pole_zero *pole_zero) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing pole_zero");

    if ((status = int_attribute(log, node, "number", NULL, &pole_zero->number))) goto exit;
    if ((status = parse_float(log, doc, node, "./stn:Real", &pole_zero->real))) goto exit;
    if ((status = parse_float(log, doc, node, "./stn:Imaginary", &pole_zero->imaginary))) goto exit;

exit:
    return status;
}


/** Read an x2r_coefficient value from the current node. */
static int parse_coefficient(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_coefficient *coefficient) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing coefficient");

    if ((status = int_attribute(log, node, "number", NULL, &coefficient->number))) goto exit;
    if ((status = parse_float(log, doc, node, ".", &coefficient->value))) goto exit;

exit:
    return status;
}


/** Read an x2r_response_list_element value from the current node. */
static int parse_response_list_element(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_response_list_element *element) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing response_list_element");

    if ((status = double_element(log, doc, node, "./stn:Frequency", NULL,
            &element->frequency))) goto exit;
    if ((status = parse_float(log, doc, node, "./stn:Amplitude", &element->amplitude))) goto exit;
    if ((status = parse_float(log, doc, node, "./stn:Phase", &element->phase))) goto exit;

exit:
    return status;
}


/** Read an x2r_numerator_coefficient value from the current node. */
static int parse_numerator_coefficient(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_numerator_coefficient *numerator_coefficient) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing numerator_coefficient");

    // this (i) is not used in IRIS-WS or here - on output we provide a new index.
    // furthermore, the statoin.xml examples seem to have a 'number' attribute
    if ((status = int_attribute(log, node, "i", "0",
            &numerator_coefficient->i))) goto exit;
    if ((status = double_element(log, doc, node, ".", NULL,
            &numerator_coefficient->value))) goto exit;

exit:
    return status;
}


/** Read an x2r_units value from the given xpath. */
static int parse_units(x2r_log *log, xmlDocPtr doc, xmlNodePtr parent, const char *xpath,
        x2r_units *units) {

    int status = X2R_OK;
    xmlNodePtr node;

    x2r_debug(log, "Parsing units");

    if ((status = find_xpath(log, doc, &node, NULL, parent, xpath))) goto exit;
    if ((status = char_element(log, doc, node, "./stn:Name", NULL, &units->name))) goto exit;
    if ((status = char_element(log, doc, node, "./stn:Description", "", &units->description))) goto exit;

exit:
    return status;
}


/** Free an x2r_units value. */
static int free_units(x2r_units *units, int status) {
    if (units) {
        free(units->name);
        free(units->description);
    }
    return status;
}


/** Read an x2r_poles_zeros value from the current node. */
static int parse_poles_zeros(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr zeros = NULL;
    xmlXPathObjectPtr poles = NULL;

    x2r_debug(log, "Parsing poles_zeros");

    if ((status = char_element(log, doc, node, "./stn:PzTransferFunctionType", NULL,
            &poles_zeros->pz_transfer_function_type))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:InputUnits",
            &poles_zeros->input_units))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:OutputUnits",
            &poles_zeros->output_units))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:NormalizationFactor", "1.0",
            &poles_zeros->normalization_factor))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:NormalizationFrequency", NULL,
            &poles_zeros->normalization_frequency))) goto exit;

    if ((status = find_xpaths(log, doc, &zeros, &poles_zeros->n_zeros, node,
            "./stn:Zero"))) goto exit;
    if (!(poles_zeros->zero = calloc(poles_zeros->n_zeros, sizeof(*poles_zeros->zero)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc zeros");
        goto exit;
    }
    for (i = 0; i < poles_zeros->n_zeros; ++i) {
        if ((status = parse_pole_zero(log, doc, zeros->nodesetval->nodeTab[i],
                &poles_zeros->zero[i]))) goto exit;
    }

    if ((status = find_xpaths(log, doc, &poles, &poles_zeros->n_poles, node,
            "./stn:Pole"))) goto exit;
    if (!(poles_zeros->pole = calloc(poles_zeros->n_poles, sizeof(*poles_zeros->pole)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc poles");
        goto exit;
    }
    for (i = 0; i < poles_zeros->n_poles; ++i) {
        if ((status = parse_pole_zero(log, doc, poles->nodesetval->nodeTab[i],
                &poles_zeros->pole[i]))) goto exit;
    }

exit:
    if (zeros) xmlXPathFreeObject(zeros);
    if (poles) xmlXPathFreeObject(poles);
    return status;
}


/** Free an x2r_poles_zeros value. */
static int free_poles_zeros(x2r_poles_zeros *poles_zeros, int status) {
    if (poles_zeros) {
        free(poles_zeros->zero);
        free(poles_zeros->pole);
        free(poles_zeros->pz_transfer_function_type);
        status = free_units(&poles_zeros->input_units, status);
        status = free_units(&poles_zeros->output_units, status);
    }
    return status;
}


/** Read an x2r_coefficients value from the current node. */
static int parse_coefficients(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_coefficients *coefficients) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr numerators = NULL;
    xmlXPathObjectPtr denominators = NULL;

    x2r_debug(log, "Parsing coefficients");

    if ((status = char_element(log, doc, node, "./stn:CfTransferFunctionType", NULL,
            &coefficients->cf_transfer_function_type))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:InputUnits",
            &coefficients->input_units))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:OutputUnits",
            &coefficients->output_units))) goto exit;

    if ((status = find_xpaths(log, doc, &numerators, &coefficients->n_numerators, node,
            "./stn:Numerator"))) goto exit;
    if (!(coefficients->numerator = calloc(coefficients->n_numerators, sizeof(*coefficients->numerator)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc numerators");
        goto exit;
    }
    for (i = 0; i < coefficients->n_numerators; ++i) {
        if ((status = parse_float(log, doc, numerators->nodesetval->nodeTab[i], ".",
                &coefficients->numerator[i]))) goto exit;
    }

    if ((status = find_xpaths(log, doc, &denominators, &coefficients->n_denominators, node,
            "./stn:Denominator"))) goto exit;
    if (!(coefficients->denominator = calloc(coefficients->n_denominators, sizeof(*coefficients->denominator)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc denominators");
        goto exit;
    }
    for (i = 0; i < coefficients->n_denominators; ++i) {
        if ((status = parse_float(log, doc, denominators->nodesetval->nodeTab[i], ".",
                &coefficients->denominator[i]))) goto exit;
    }

exit:
    if (numerators) xmlXPathFreeObject(numerators);
    if (denominators) xmlXPathFreeObject(denominators);
    return status;
}


/** Free an x2r_coefficients value. */
static int free_coefficients(x2r_coefficients *coefficients, int status) {
    if (coefficients) {
        free(coefficients->numerator);
        free(coefficients->denominator);
        free(coefficients->cf_transfer_function_type);
        status = free_units(&coefficients->input_units, status);
        status = free_units(&coefficients->output_units, status);
    }
    return status;
}


/** Read an x2r_response_list value from the current node. */
static int parse_response_list(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_response_list *response_list) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr elements = NULL;

    x2r_debug(log, "Parsing response_list");

    if ((status = parse_units(log, doc, node, "./stn:InputUnits",
            &response_list->input_units))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:OutputUnits",
            &response_list->output_units))) goto exit;

    if ((status = find_xpaths(log, doc, &elements, &response_list->n_response_list_elements, node,
            "./stn:ResponseListElement"))) goto exit;
    if (!(response_list->response_list_element =
            calloc(response_list->n_response_list_elements, sizeof(*response_list->response_list_element)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc response_list_elements");
        goto exit;
    }
    for (i = 0; i < response_list->n_response_list_elements; ++i) {
        if ((status = parse_response_list_element(log, doc, elements->nodesetval->nodeTab[i],
                &response_list->response_list_element[i]))) goto exit;
    }

exit:
    if (elements) xmlXPathFreeObject(elements);
    return status;
}


/** Free an x2r_response_list value. */
static int free_response_list(x2r_response_list *response_list, int status) {
    if (response_list) {
        free(response_list->response_list_element);
    }
    return status;
}


/** Read an x2r_fir value from the current node. */
static int parse_fir(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_fir *fir) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr numerator_coefficients = NULL;

    x2r_debug(log, "Parsing fir");

    if ((status = char_element(log, doc, node, "./stn:Symmetry", NULL, &fir->symmetry))) goto exit;
    // 'null'[sic] - see station-2.xml and response-2 in tests
    if ((status = char_attribute(log, node, "name", "null", &fir->name))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:InputUnits", &fir->input_units))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:OutputUnits", &fir->output_units))) goto exit;

    if ((status = find_xpaths(log, doc, &numerator_coefficients, &fir->n_numerator_coefficients,
            node, "./stn:NumeratorCoefficient"))) goto exit;
    if (!(fir->numerator_coefficient =
            calloc(fir->n_numerator_coefficients, sizeof(*fir->numerator_coefficient)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc numerator_coefficient");
        goto exit;
    }
    for (i = 0; i < fir->n_numerator_coefficients; ++i) {
        if ((status = parse_numerator_coefficient(log, doc,
                numerator_coefficients->nodesetval->nodeTab[i],
                &fir->numerator_coefficient[i]))) goto exit;
    }

exit:
    if (numerator_coefficients) xmlXPathFreeObject(numerator_coefficients);
    return status;
}


/** Free an x2r_fir value. */
static int free_fir(x2r_fir *fir, int status) {
    if (fir) {
        free(fir->symmetry);
        free(fir->name);
        free(fir->numerator_coefficient);
    }
    return status;
}


/** Read an x2r_polynomial value from the current node. */
static int parse_polynomial(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_polynomial *polynomial) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr coefficents = NULL;

    x2r_debug(log, "Parsing polynomial");

    if ((status = char_element(log, doc, node, "./stn:ApproximationType", NULL,
            &polynomial->approximation_type))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:InputUnits",
            &polynomial->input_units))) goto exit;
    if ((status = parse_units(log, doc, node, "./stn:OutputUnits",
            &polynomial->output_units))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:FrequencyLowerBound", "0.0",
            &polynomial->frequency_lower_bound))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:FrequencyUpperBound", "0.0",
            &polynomial->frequency_upper_bound))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:ApproximationLowerBound", "0.0",
            &polynomial->approximation_lower_bound))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:ApproximationUpperBound", "0.0",
            &polynomial->approximation_upper_bound))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:MaximumError", "0.0",
            &polynomial->maximum_error))) goto exit;

    if ((status = find_xpaths(log, doc, &coefficents, &polynomial->n_coefficients, node,
            "./stn:Coefficient"))) goto exit;
    if (!(polynomial->coefficient =
            calloc(polynomial->n_coefficients, sizeof(*polynomial->coefficient)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc coefficients");
        goto exit;
    }
    for (i = 0; i < polynomial->n_coefficients; ++i) {
        if ((status = parse_coefficient(log, doc, coefficents->nodesetval->nodeTab[i],
                &polynomial->coefficient[i]))) goto exit;
    }

exit:
    if (coefficents) xmlXPathFreeObject(coefficents);
    return status;
}


/** Free an x2r_polynomial value. */
static int free_polynomial(x2r_polynomial *polynomial, int status) {
    if (polynomial) {
        free(polynomial->coefficient);
        free(polynomial->approximation_type);
        status = free_units(&polynomial->input_units, status);
        status = free_units(&polynomial->output_units, status);
    }
    return status;
}


/** Read an x2r_decimation value from the current node. */
static int parse_decimation(x2r_log *log, xmlDocPtr doc, xmlNodePtr node,
        x2r_decimation *decimation) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing decimation");

    if ((status = double_element(log, doc, node, "./stn:InputSampleRate", NULL,
            &decimation->input_sample_rate))) goto exit;
    if ((status = int_element(log, doc, node, "./stn:Factor", NULL,
            &decimation->factor))) goto exit;
    if ((status = int_element(log, doc, node, "./stn:Offset", NULL,
            &decimation->offset))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:Delay", NULL,
            &decimation->delay))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:Correction", NULL,
            &decimation->correction))) goto exit;

exit:
    return status;
}


/** Read an x2r_gain value from the current node. */
static int parse_gain(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_gain *gain) {

    int status = X2R_OK;

    if ((status = double_element(log, doc, node, "./stn:Value", "0.0", &gain->value))) goto exit;
    if ((status = double_element(log, doc, node, "./stn:Frequency", "0.0", &gain->frequency))) goto exit;

exit:
    return status;
}


static int set_stage(x2r_log *log, x2r_stage *stage, x2r_stage_type type) {
    if (stage->type) {
        return x2r_error(log, X2R_ERR_XML, "Multiple content in a single stage");
    } else {
        stage->type = type;
        return X2R_OK;
    }
}


/** Read an x2r_stage value from the current node. */
static int parse_stage(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_stage *stage) {

    int status = X2R_OK, found;
    xmlNodePtr poles_zeros;
    xmlNodePtr coefficients;
    xmlNodePtr response_list;
    xmlNodePtr fir;
    xmlNodePtr polynomial;
    xmlNodePtr decimation;
    xmlNodePtr stage_gain;

    if ((status = int_attribute(log, node, "number", NULL, &stage->number))) goto exit;

    x2r_debug(log, "Parsing stage %d", stage->number);

    if ((status = find_xpath(log, doc, &poles_zeros, &found, node, "./stn:PolesZeros"))) goto exit;
    if (found) {
        if ((status = set_stage(log, stage, X2R_STAGE_POLES_ZEROS))) goto exit;
        if (!(stage->u.poles_zeros = calloc(1, sizeof(*stage->u.poles_zeros)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc poles_zeros");
            goto exit;
        }
        if ((status = parse_poles_zeros(log, doc, poles_zeros, stage->u.poles_zeros))) goto exit;
    }

    if ((status = find_xpath(log, doc, &coefficients, &found, node, "./stn:Coefficients"))) goto exit;
    if (found) {
        if ((status = set_stage(log, stage, X2R_STAGE_COEFFICIENTS))) goto exit;
        if (!(stage->u.coefficients = calloc(1, sizeof(*stage->u.coefficients)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc coefficients");
            goto exit;
        }
        if ((status = parse_coefficients(log, doc, coefficients, stage->u.coefficients))) goto exit;
    }

    if ((status = find_xpath(log, doc, &response_list, &found, node, "./stn:ResponseList"))) goto exit;
    if (found) {
        if ((status = set_stage(log, stage, X2R_STAGE_RESPONSE_LIST))) goto exit;
        if (!(stage->u.response_list = calloc(1, sizeof(*stage->u.response_list)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc response_list");
            goto exit;
        }
        if ((status = parse_response_list(log, doc, response_list, stage->u.response_list))) goto exit;
    }

    if ((status = find_xpath(log, doc, &fir, &found, node, "./stn:FIR"))) goto exit;
    if (found) {
        if ((status = set_stage(log, stage, X2R_STAGE_FIR))) goto exit;
        if (!(stage->u.fir = calloc(1, sizeof(*stage->u.fir)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc fir");
            goto exit;
        }
        if ((status = parse_fir(log, doc, fir, stage->u.fir))) goto exit;
    }

    if ((status = find_xpath(log, doc, &polynomial, &found, node, "./stn:Polynomial"))) goto exit;
    if (found) {
        if ((status = set_stage(log, stage, X2R_STAGE_POLYNOMIAL))) goto exit;
        if (!(stage->u.polynomial = calloc(1, sizeof(*stage->u.polynomial)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc polynomial");
            goto exit;
        }
        if ((status = parse_polynomial(log, doc, polynomial, stage->u.polynomial))) goto exit;
    }

    if (!stage->type) {
        x2r_warn(log, "No content in stage (during parse)");
    }

    if ((status = find_xpath(log, doc, &decimation, &found, node, "./stn:Decimation"))) goto exit;
    if (found) {
        if (!(stage->decimation = calloc(1, sizeof(*stage->decimation)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc decimation");
            goto exit;
        }
        if ((status = parse_decimation(log, doc, decimation, stage->decimation))) goto exit;
    }

    // the schema says this is required, i think, but IRIS-WS happily works with data
    // without it.
    if ((status = find_xpath(log, doc, &stage_gain, &found, node, "./stn:StageGain"))) goto exit;
    if (found) {
        if (!(stage->stage_gain = calloc(1, sizeof(*stage->stage_gain)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stage_gain");
            goto exit;
        }
        if ((status = parse_gain(log, doc, stage_gain, stage->stage_gain))) goto exit;
    }

exit:
    return status;
}


/** Free an x2r_stage value. */
static int free_stage(x2r_stage *stage, int status) {
    if (stage->u.poles_zeros) {
        switch (stage->type) {
        case X2R_STAGE_POLES_ZEROS:
            status = free_poles_zeros(stage->u.poles_zeros, status);
            break;
        case X2R_STAGE_COEFFICIENTS:
            status = free_coefficients(stage->u.coefficients, status);
            break;
        case X2R_STAGE_RESPONSE_LIST:
            status = free_response_list(stage->u.response_list, status);
            break;
        case X2R_STAGE_FIR:
            status = free_fir(stage->u.fir, status);
            break;
        case X2R_STAGE_POLYNOMIAL:
            status = free_polynomial(stage->u.polynomial, status);
            break;
        default:
            break;
        }
        free(stage->u.poles_zeros);  // any
        free(stage->decimation);
        free(stage->stage_gain);
    }
    return status;
}


/** Read an x2r_response value from the current node. */
static int parse_response(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_response *response) {

    int status = X2R_OK, i, found;
    xmlXPathObjectPtr stages = NULL;
    xmlNodePtr instrument_sensitivity;
    xmlNodePtr instrument_polynomial;

    x2r_debug(log, "Parsing response");

    if ((status = find_xpaths(log, doc, &stages, &response->n_stages, node,
            "./stn:Stage"))) goto exit;

    if (!(response->stage = calloc(response->n_stages, sizeof(*response->stage)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stages");
        goto exit;
    }

    for (i = 0; i < response->n_stages; ++i) {
        if ((status = parse_stage(log, doc, stages->nodesetval->nodeTab[i],
                &response->stage[i]))) goto exit;
    }

    if ((status = find_xpath(log, doc, &instrument_sensitivity, &found, node,
            "./stn:InstrumentSensitivity"))) goto exit;
    if (found) {
        if (!(response->instrument_sensitivity =
                calloc(1, sizeof(*response->instrument_sensitivity)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc instrument_sensitivity");
            goto exit;
        }
        if ((status = parse_gain(log, doc, instrument_sensitivity,
                response->instrument_sensitivity))) goto exit;
    }

    if ((status = find_xpath(log, doc, &instrument_polynomial, &found, node,
            "./stn:InstrumentPolynomial"))) goto exit;
    if (found) {
        if (!(response->instrument_polynomial =
                calloc(1, sizeof(*response->instrument_polynomial)))) {
            status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc instrument_polynomial");
            goto exit;
        }
        if ((status = parse_polynomial(log, doc, instrument_polynomial,
                response->instrument_polynomial))) goto exit;
    }

exit:
    if (stages) xmlXPathFreeObject(stages);
    return status;
}


/** Free an x2r_response value. */
static int free_response(x2r_response *response, int status) {
    int i;
    if (response) {
        for (i = 0; i < response->n_stages; ++i) {
            status = free_stage(&response->stage[i], status);
        }
        free(response->stage);
        free(response->instrument_sensitivity);
        if (response->instrument_polynomial) {
            status = free_polynomial(response->instrument_polynomial, status);
            free(response->instrument_polynomial);
        }
    }
    return status;
}


/** Read an x2r_channel value from the current node. */
static int parse_channel(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_channel *channel) {

    int status = X2R_OK;
    xmlNodePtr response;

    x2r_debug(log, "Parsing channel");

    if ((status = char_attribute(log, node, "code", NULL, &channel->code))) goto exit;
    if ((status = char_attribute(log, node, "locationCode", NULL, &channel->location_code))) goto exit;
    if ((status = datetime_attribute(log, node, "startDate", &channel->start_date))) goto exit;
    if ((status = datetime_attribute(log, node, "endDate", &channel->end_date))) goto exit;
    if ((status = find_xpath(log, doc, &response, NULL, node, "./stn:Response"))) goto exit;

    if ((status = parse_response(log, doc, response, &channel->response))) goto exit;

exit:
    return status;
}


/** Free an x2r_channel value. */
static int free_channel(x2r_channel *channel, int status) {
    if (channel) {
        status = free_response(&channel->response, status);
        free(channel->location_code);
        free(channel->code);
    }
    return status;
}


/** Read an x2r_station value from the current node. */
static int parse_station(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_station *station) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr stations = NULL;

    x2r_debug(log, "Parsing station");

    if ((status = find_xpaths(log, doc, &stations, &station->n_channels, node,
            "./stn:Channel"))) goto exit;

    if (!(station->channel = calloc(station->n_channels, sizeof(*station->channel)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc channels");
        goto exit;
    }

    if ((status = char_attribute(log, node, "code", NULL, &station->code))) goto exit;

    for (i = 0; i < station->n_channels; ++i) {
        if ((status = parse_channel(log, doc, stations->nodesetval->nodeTab[i],
                &station->channel[i]))) goto exit;
    }

exit:
    if (stations) xmlXPathFreeObject(stations);
    return status;
}


/** Free an x2r_station value. */
static int free_station(x2r_station *station, int status) {
    int i;
    if (station) {
        for (i = 0; i < station->n_channels; ++i) {
            status = free_channel(&station->channel[i], status);
        }
        free(station->code);
        free(station->channel);
    }
    return status;
}


/** Read an x2r_network value from the current node. */
static int parse_network(x2r_log *log, xmlDocPtr doc, xmlNodePtr node, x2r_network *network) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr stations = NULL;

    x2r_debug(log, "Parsing network");

    if ((status = find_xpaths(log, doc, &stations, &network->n_stations, node,
            "./stn:Station"))) goto exit;

    if (!(network->station = calloc(network->n_stations, sizeof(*network->station)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stations");
        goto exit;
    }

    if ((status = char_attribute(log, node, "code", NULL, &network->code))) goto exit;

    for (i = 0; i < network->n_stations; ++i) {
        if ((status = parse_station(log, doc, stations->nodesetval->nodeTab[i],
                &network->station[i]))) goto exit;
    }

exit:
    if (stations) xmlXPathFreeObject(stations);
    return status;
}


/** Free an x2r_network value. */
static int free_network(x2r_network *network, int status) {
    int i;
    if (network) {
        for (i = 0; i < network->n_stations; ++i) {
            status = free_station(&network->station[i], status);
        }
        free(network->code);
        free(network->station);
    }
    return status;
}


/** Read an x2r_fdsn_station_xml value from the current node. */
static int parse_fdsn_station_xml(x2r_log *log, xmlDocPtr doc, x2r_fdsn_station_xml **root) {

    int status = X2R_OK, i;
    xmlXPathObjectPtr networks = NULL;

    x2r_debug(log, "Parsing root");

    if (!(*root = calloc(1, sizeof(**root)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc fdsn_station_xml");
        goto exit;
    }

    if ((status = find_xpaths(log, doc, &networks, &(*root)->n_networks, NULL,
            "//stn:FDSNStationXML/stn:Network"))) goto exit;

    if (!((*root)->network = calloc((*root)->n_networks, sizeof(*(*root)->network)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc networks");
        goto exit;
    }

    for (i = 0; i < (*root)->n_networks; ++i) {
        if ((status = parse_network(log, doc, networks->nodesetval->nodeTab[i],
                &(*root)->network[i]))) goto exit;
    }

exit:
    if (networks) xmlXPathFreeObject(networks);
    return status;
}


/** Free an x2r_fdsn_station_xml value. */
int x2r_free_fdsn_station_xml(x2r_fdsn_station_xml *root, int status) {
    int i;
    if (root) {
        for (i = 0; i < root->n_networks; ++i) {
            status = free_network(&root->network[i], status);
        }
        free(root->network);
        free(root);
    }
    return status;
}


/**
 * The equivalent of StationService.load() in IRIS-WS, constructing an in-memory
 * representation of the station.xml file read from the given stream.
 */
int x2r_station_service_load(x2r_log *log, FILE *in, x2r_fdsn_station_xml **root) {

    int status = X2R_OK;
    xmlDocPtr doc = NULL;

    if ((status = stream2doc(log, in, &doc))) goto exit;
    if ((status = parse_fdsn_station_xml(log, doc, root))) goto exit;

exit:
    if (doc) xmlFreeDoc(doc);
    return status;
}


