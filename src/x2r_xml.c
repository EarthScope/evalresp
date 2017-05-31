
#include <string.h>
#include <mxml.h>

#include "x2r.h"
#include "x2r_xml.h"

#define BUFFER_SIZE 5000


// This file builds an in-memory model of the station.xml document.
// Only those parts needed to print the response document (in x2r_ws.c)
// are generated.

// http://stackoverflow.com/questions/16647819/timegm-cross-platform
#ifdef _WIN32
#define timegm _mkgmtime
#endif

/* Read an XML file from the given stream and create an in-memory model. */
static int stream2doc(x2r_log *log, FILE *in, mxml_node_t **doc) {

    int status = X2R_OK;

    if (!(*doc = mxmlLoadFd(NULL, fileno(in), MXML_OPAQUE_CALLBACK))) {
        status = x2r_error(log, X2R_ERR_XML, "Could not parse input");
    }

    return status;
}


/* A collection of nodes. */
typedef struct {
	int n;
	mxml_node_t **node;
} nodelist;


/* Free a collection of nodes. */
void free_nodelist(nodelist **nodes) {
	if (*nodes) {
		free((*nodes)->node);
		free(*nodes);
		*nodes = NULL;
	}
}


/* Find all children with the given name. */
static int find_children(x2r_log *log, nodelist **result, mxml_node_t *from, const char *name) {

	int status = X2R_OK;
	mxml_node_t *child;

	if (!(*result = calloc(1, sizeof(**result)))) {
		status = x2r_error(log, X2R_ERR_MEMORY, "Could not allocate nodelist");
	} else {
		child = mxmlGetFirstChild(from);
		while (!status && child) {
			while (!status && child &&
					(mxmlGetType(child) != MXML_ELEMENT || strcmp(mxmlGetElement(child), name))) {
				child = mxmlGetNextSibling(child);
			}
			if (!status && child) {
				(*result)->n++;
				if (!((*result)->node = realloc((*result)->node, (*result)->n * sizeof(*(*result)->node)))) {
					status = x2r_error(log, X2R_ERR_MEMORY, "Could not reallocate nodelist");
				} else {
					(*result)->node[(*result)->n-1] = child;
				}
				child = mxmlGetNextSibling(child);
			}
		}
	}

	if (status) free_nodelist(result);
	return status;
}


/*
 * Find a single child of the given name.  If name is "." then return this node.
 *
 * If found is provided, it is set to 1 if the child is found (otherwise 0).
 * If found is not provided it is an error if the child is not found.
 *
 * Other children (possible duplicates) are ignored.
 */
static int find_child(x2r_log *log, mxml_node_t **result, int* found, mxml_node_t *from,
		const char *name) {

    int status = X2R_OK, count = 0;
    mxml_node_t *child;

    if (!name || !strcmp(name, ".")) {
    	*result = from;
    	count = 1;
    } else {
    	child = mxmlGetFirstChild(from);
    	while (child &&
    			(mxmlGetType(child) != MXML_ELEMENT || strcmp(mxmlGetElement(child), name))) {
    		child = mxmlGetNextSibling(child);
    	}
    	if (child) {
    		*result = child;
    		count = 1;
    	}
    }

    if (found) {
        *found = count;
    } else {
        if (!count) {
            status = x2r_error(log, X2R_ERR_XML, "No child for %s", name);
        }
    }

    return status;
}


/* Read the text contents of an element as a char*. */
static int char_element(x2r_log *log, mxml_node_t *node, const char *name,
        const char *deflt, char **value) {

    int status = X2R_OK, found;
    mxml_node_t *element, *child;
    const char *text;

    if (!(status = find_child(log, &element, &found, node, name))) {
		if (found) {
			child = mxmlGetFirstChild(element);
			while (child && mxmlGetType(child) != MXML_OPAQUE) child = mxmlGetNextSibling(child);
			if (!child) {
				status = x2r_error(log, X2R_ERR_XML, "No text for %s", name);
			} else {
				if (!(text = mxmlGetOpaque(child))) {
					status = x2r_error(log, X2R_ERR_MEMORY, "Cannot access text for %s", name);
				} else {
					*value = strdup(text);
				}
			}
		} else {
			if (deflt) {
				*value = strdup(deflt);
			} else {
				status = x2r_error(log, X2R_ERR_XML, "Missing %s element", name);
			}
		}
    }

    return status;
}


/* Read the text contents of an element as an int. */
static int int_element(x2r_log *log, mxml_node_t *node, const char *name,
		const char *deflt, int *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if (!(status = char_element(log, node, name, deflt, &text))) {
    	*value = strtol(text, &end, 10);
    	while (isspace(*end)) end++;
    	if (*end) {  // should point to end of string
    		status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
    	}
    }

    free(text);
    return status;
}


/* Read the text contents of an element as a double. */
static int double_element(x2r_log *log, mxml_node_t *node, const char *name,
        const char *deflt, double *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if (!(status = char_element(log, node, name, deflt, &text))) {
		*value = strtod(text, &end);
    	while (isspace(*end)) end++;
		if (*end) {  // should point to end of string
			status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
		}
    }

    free(text);
    return status;
}


/* Read an attribute value as a char*. */
static int char_attribute(x2r_log *log, mxml_node_t *node, const char *name,
        const char *deflt, char **value) {

    int status = X2R_OK;
    const char *attr;

    if (!(attr = mxmlElementGetAttr(node, name))) {
        if (deflt) {
            *value = strdup(deflt);
        } else {
            status = x2r_error(log, X2R_ERR_XML, "Could not find %s in %s", name,
            		mxmlGetElement(node));
        }
    } else {
        *value = strdup(attr);
    }

    return status;
}


/*
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
    } else {
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
    }

    return status;
}


/* Read an attribute value as an ISO formatted epoch. */
static int datetime_attribute(x2r_log *log, mxml_node_t *node, const char *name, time_t *epoch) {

    int status = X2R_OK;
    char *value = NULL;

    if (!(status = char_attribute(log, node, name, NULL, &value))) {
    	status = x2r_parse_iso_datetime(log, value, epoch);
    }

    free(value);
    return status;
}


/* Read an attribute value as an int. */
static int int_attribute(x2r_log *log, mxml_node_t *node, const char *name,
        const char *deflt, int *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if (!(status = char_attribute(log, node, name, deflt, &text))) {
    	*value = strtol(text, &end, 10);
    	if (*end) {  // should point to end of string
    		status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
    	}
    }

    free(text);
    return status;
}


/* Read an attribute value as double. */
static int double_attribute(x2r_log *log, mxml_node_t *node, const char *name, double *value) {

    int status = X2R_OK;
    char *text = NULL, *end;

    if (!(status = char_attribute(log, node, name, NULL, &text))) {
    	*value = strtod(text, &end);
    	if (*end) {  // should point to end of string
    		status = x2r_error(log, X2R_ERR_XML, "Did not parse all of %s", text);
    	}
    }

    free(text);
    return status;
}


/* Read an x2r_float value from the given path. */
static int parse_float(x2r_log *log, mxml_node_t *parent, const char *path, x2r_float *flt) {

    int status = X2R_OK;
    mxml_node_t *node;

    x2r_debug(log, "Parsing float");

    if (!(status = find_child(log, &node, NULL, parent, path))) {
    	if (!(status = double_element(log, node, ".", NULL, &flt->value))) {
    		if (!(status = double_attribute(log, node, "plusError", &flt->plus_error))) {
    			status = double_attribute(log, node, "minusError", &flt->minus_error);
    		}
    	}
    }

    return status;
}


/* Read an x2r_pole_zero value from the current node. */
static int parse_pole_zero(x2r_log *log, mxml_node_t *node, x2r_pole_zero *pole_zero) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing pole_zero");

    if (!(status = int_attribute(log, node, "number", NULL, &pole_zero->number))) {
    	if (!(status = parse_float(log, node, "Real", &pole_zero->real))) {
    		status = parse_float(log, node, "Imaginary", &pole_zero->imaginary);
    	}
    }

    return status;
}


/* Read an x2r_coefficient value from the current node. */
static int parse_coefficient(x2r_log *log, mxml_node_t *node, x2r_coefficient *coefficient) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing coefficient");

    if (!(status = int_attribute(log, node, "number", NULL, &coefficient->number))) {
    	status = parse_float(log, node, ".", &coefficient->value);
    }

    return status;
}


/* Read an x2r_response_list_element value from the current node. */
static int parse_response_list_element(x2r_log *log, mxml_node_t *node,
		x2r_response_list_element *element) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing response_list_element");

    if (!(status = double_element(log, node, "Frequency", NULL, &element->frequency))) {
    	if (!(status = parse_float(log, node, "Amplitude", &element->amplitude))) {
    		status = parse_float(log, node, "Phase", &element->phase);
    	}
    }

    return status;
}


/* Read an x2r_numerator_coefficient value from the current node. */
static int parse_numerator_coefficient(x2r_log *log, mxml_node_t *node,
        x2r_numerator_coefficient *numerator_coefficient) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing numerator_coefficient");

    // this (i) is not used in IRIS-WS or here - on output we provide a new index.
    // furthermore, the station.xml examples seem to have a 'number' attribute
    if (!(status = int_attribute(log, node, "i", "0", &numerator_coefficient->i))) {
    	status = double_element(log, node, ".", NULL, &numerator_coefficient->value);
    }

    return status;
}


/* Read an x2r_units value from the given path. */
static int parse_units(x2r_log *log, mxml_node_t *parent, const char *path,
        x2r_units *units) {

    int status = X2R_OK;
    mxml_node_t *node;

    x2r_debug(log, "Parsing units");

    if (!(status = find_child(log, &node, NULL, parent, path))) {
    	if (!(status = char_element(log, node, "Name", NULL, &units->name))) {
    		status = char_element(log, node, "Description", "", &units->description);
    	}
    }

    return status;
}


/* Free an x2r_units value. */
static int free_units(x2r_units *units, int status) {
    if (units) {
        free(units->name);
        free(units->description);
    }
    return status;
}


/* Parse the (non-pole/zero) data associated with poles and zeros. */
static int parse_poles_zeros_data(x2r_log *log, mxml_node_t *node,
		x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK;

    if (!(status = char_element(log, node, "PzTransferFunctionType", NULL,
            &poles_zeros->pz_transfer_function_type))) {
    	if (!(status = parse_units(log, node, "InputUnits",
    			&poles_zeros->input_units))) {
    		if (!(status = parse_units(log, node, "OutputUnits",
    				&poles_zeros->output_units))) {
    			if (!(status = double_element(log, node, "NormalizationFactor", "1.0",
    					&poles_zeros->normalization_factor))) {
    				status = double_element(log, node, "NormalizationFrequency", NULL,
    						&poles_zeros->normalization_frequency);
    			}
    		}
    	}
    }

    return status;
}


/* Parse the zeros. */
static int parse_poles_zeros_zeros(x2r_log *log, mxml_node_t *node,
		x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK, i;
    nodelist *zeros = NULL;

    if (!(status = find_children(log, &zeros, node, "Zero"))) {
    	poles_zeros->n_zeros = zeros->n;
    	if (!(poles_zeros->zero = calloc(poles_zeros->n_zeros, sizeof(*poles_zeros->zero)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc zeros");
    	}
    	for (i = 0; !status && i < poles_zeros->n_zeros; ++i) {
    		status = parse_pole_zero(log, zeros->node[i], &poles_zeros->zero[i]);
    	}
    }

    free_nodelist(&zeros);
    return status;
}


/* Parse the poles. */
static int parse_poles_zeros_poles(x2r_log *log, mxml_node_t *node,
		x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK, i;
    nodelist *poles = NULL;

    if (!(status = find_children(log, &poles, node, "Pole"))) {
    	poles_zeros->n_poles = poles->n;
    	if (!(poles_zeros->pole = calloc(poles_zeros->n_poles, sizeof(*poles_zeros->pole)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc poles");
    	}
    	for (i = 0; !status && i < poles_zeros->n_poles; ++i) {
    		status = parse_pole_zero(log, poles->node[i], &poles_zeros->pole[i]);
    	}
    }

    return status;
}


/* Read an x2r_poles_zeros value from the current node. */
static int parse_poles_zeros(x2r_log *log, mxml_node_t *node,
        x2r_poles_zeros *poles_zeros) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing poles_zeros");

    if (!(status = parse_poles_zeros_data(log, node, poles_zeros))) {
    	if (!(status = parse_poles_zeros_poles(log, node, poles_zeros))) {
    		status = parse_poles_zeros_zeros(log, node, poles_zeros);
    	}
    }

    return status;
}


/* Free an x2r_poles_zeros value. */
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


/* Parse the (non-numerator/denominator) data associated with coefficients. */
static int parse_coefficients_data(x2r_log *log, mxml_node_t *node,
        x2r_coefficients *coefficients) {

    int status = X2R_OK;

    if (!(status = char_element(log, node, "CfTransferFunctionType", NULL,
            &coefficients->cf_transfer_function_type))) {
    	if (!(status = parse_units(log, node, "InputUnits",
    			&coefficients->input_units))) {
    		status = parse_units(log, node, "OutputUnits",
    				&coefficients->output_units);
    	}
    }

    return status;
}


/* Parse the numerators. */
static int parse_coefficients_numerators(x2r_log *log, mxml_node_t *node,
        x2r_coefficients *coefficients) {

    int status = X2R_OK, i;
    nodelist *numerators = NULL;

    if (!(status = find_children(log, &numerators, node, "Numerator"))) {
    	coefficients->n_numerators = numerators->n;
    	if (!(coefficients->numerator = calloc(coefficients->n_numerators,
    			sizeof(*coefficients->numerator)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc numerators");
    	} else {
    		for (i = 0; !status && i < coefficients->n_numerators; ++i) {
    			status = parse_float(log, numerators->node[i], ".",
    					&coefficients->numerator[i]);
    		}
    	}
    }

    free_nodelist(&numerators);
    return status;
}


/* Parse the denominators. */
static int parse_coefficients_denominators(x2r_log *log, mxml_node_t *node,
        x2r_coefficients *coefficients) {

    int status = X2R_OK, i;
    nodelist *denominators = NULL;

    if (!(status = find_children(log, &denominators, node, "Denominator"))) {
    	coefficients->n_denominators = denominators->n;
    	if (!(coefficients->denominator = calloc(coefficients->n_denominators,
    			sizeof(*coefficients->denominator)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc denominators");
    	} else {
    		for (i = 0; !status && i < coefficients->n_denominators; ++i) {
    			status = parse_float(log, denominators->node[i], ".",
    					&coefficients->denominator[i]);
    		}
    	}
    }

    free_nodelist(&denominators);
    return status;
}


/* Read an x2r_coefficients value from the current node. */
static int parse_coefficients(x2r_log *log, mxml_node_t *node,
        x2r_coefficients *coefficients) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing coefficients");

    if (!(status = parse_coefficients_data(log, node, coefficients))) {
    	if (!(status = parse_coefficients_numerators(log, node, coefficients))) {
    		status = parse_coefficients_denominators(log, node, coefficients);
    	}
    }

    return status;
}


/* Free an x2r_coefficients value. */
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


/* Read an x2r_response_list value from the current node. */
static int parse_response_list(x2r_log *log, mxml_node_t *node,
        x2r_response_list *response_list) {

    int status = X2R_OK, i;
    nodelist *elements = NULL;

    x2r_debug(log, "Parsing response_list");

    if (!(status = parse_units(log, node, "InputUnits",
            &response_list->input_units))) {
    	if (!(status = parse_units(log, node, "OutputUnits",
            &response_list->output_units))) {

    		if ((status = find_children(log, &elements, node, "ResponseListElement"))) {
    			response_list->n_response_list_elements = elements->n;
    			if (!(response_list->response_list_element =
    					calloc(response_list->n_response_list_elements,
    								sizeof(*response_list->response_list_element)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc response_list_elements");
    			} else {
    				for (i = 0; !status && i < response_list->n_response_list_elements; ++i) {
    					status = parse_response_list_element(log, elements->node[i],
    							&response_list->response_list_element[i]);
    				}
    			}
    		}
    	}
    }

    free_nodelist(&elements);
    return status;
}


/* Free an x2r_response_list value. */
static int free_response_list(x2r_response_list *response_list, int status) {
    if (response_list) {
        free(response_list->response_list_element);
    }
    return status;
}


/* Parse (non-numerator/denominator) FIR data. */
static int parse_fir_data(x2r_log *log, mxml_node_t *node, x2r_fir *fir) {

    int status = X2R_OK;

    if (!(status = char_element(log, node, "Symmetry", NULL, &fir->symmetry))) {
    	// 'null'[sic] - see station-2.xml and response-2 in tests
    	if (!(status = char_attribute(log, node, "name", "null", &fir->name))) {
    		if (!(status = parse_units(log, node, "InputUnits", &fir->input_units))) {
    			status = parse_units(log, node, "OutputUnits", &fir->output_units);
    		}
    	}
    }

    return status;
}


/* Read an x2r_fir value from the current node. */
static int parse_fir(x2r_log *log, mxml_node_t *node, x2r_fir *fir) {

    int status = X2R_OK, i;
    nodelist *numerator_coefficients = NULL;

    x2r_debug(log, "Parsing fir");

    if (!(status = parse_fir_data(log, node, fir))) {

    	if (!(status = find_children(log, &numerator_coefficients,
    			node, "NumeratorCoefficient"))) {
    		fir->n_numerator_coefficients = numerator_coefficients->n;
    		if (!(fir->numerator_coefficient =
    				calloc(fir->n_numerator_coefficients, sizeof(*fir->numerator_coefficient)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc numerator_coefficient");
    		} else {
    			for (i = 0; !status && i < fir->n_numerator_coefficients; ++i) {
    				status = parse_numerator_coefficient(log, numerator_coefficients->node[i],
    						&fir->numerator_coefficient[i]);
    			}
    		}
    	}
    }

    free_nodelist(&numerator_coefficients);
    return status;
}


/* Free an x2r_fir value. */
static int free_fir(x2r_fir *fir, int status) {
    if (fir) {
        free(fir->symmetry);
        free(fir->name);
        free(fir->numerator_coefficient);
    }
    return status;
}


/* Parse (non-coefficient) polynomial data. */
static int parse_polynomial_data(x2r_log *log, mxml_node_t *node,
		x2r_polynomial *polynomial) {

    int status = X2R_OK;

    if (!(status = char_element(log, node, "ApproximationType", NULL,
            &polynomial->approximation_type))) {
    	if (!(status = parse_units(log, node, "InputUnits",
    			&polynomial->input_units))) {
    		if (!(status = parse_units(log, node, "OutputUnits",
    				&polynomial->output_units))) {
    			status = double_element(log, node, "FrequencyLowerBound", "0.0",
    					&polynomial->frequency_lower_bound);
    		}
    	}
    }
    if (!status) {
    	if (!(status = double_element(log, node, "FrequencyUpperBound", "0.0",
            &polynomial->frequency_upper_bound))) {
    		if (!(status = double_element(log, node, "ApproximationLowerBound", "0.0",
    				&polynomial->approximation_lower_bound))) {
    			if (!(status = double_element(log,  node, "ApproximationUpperBound", "0.0",
    					&polynomial->approximation_upper_bound))) {
    				status = double_element(log, node, "MaximumError", "0.0",
    						&polynomial->maximum_error);
    			}
    		}
    	}
    }

    return status;
}


/* Read an x2r_polynomial value from the current node. */
static int parse_polynomial(x2r_log *log, mxml_node_t *node, x2r_polynomial *polynomial) {

    int status = X2R_OK, i;
    nodelist *coefficents = NULL;

    x2r_debug(log, "Parsing polynomial");

    if (!(status = parse_polynomial_data(log, node, polynomial))) {

    	if (!(status = find_children(log, &coefficents, node, "Coefficient"))) {
    		polynomial->n_coefficients = coefficents->n;
    		if (!(polynomial->coefficient =
    				calloc(polynomial->n_coefficients, sizeof(*polynomial->coefficient)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc coefficients");
    		} else {
    			for (i = 0; !status && i < polynomial->n_coefficients; ++i) {
    				status = parse_coefficient(log, coefficents->node[i],
    						&polynomial->coefficient[i]);
    			}
    		}
    	}
    }

    free_nodelist(&coefficents);
    return status;
}


/* Free an x2r_polynomial value. */
static int free_polynomial(x2r_polynomial *polynomial, int status) {
    if (polynomial) {
        free(polynomial->coefficient);
        free(polynomial->approximation_type);
        status = free_units(&polynomial->input_units, status);
        status = free_units(&polynomial->output_units, status);
    }
    return status;
}


/* Read an x2r_decimation value from the current node. */
static int parse_decimation(x2r_log *log, mxml_node_t *node, x2r_decimation *decimation) {

    int status = X2R_OK;

    x2r_debug(log, "Parsing decimation");

    if (!(status = double_element(log, node, "InputSampleRate", NULL,
            &decimation->input_sample_rate))) {
    	if (!(status = int_element(log, node, "Factor", NULL,
    			&decimation->factor))) {
    		if (!(status = int_element(log, node, "Offset", NULL,
    				&decimation->offset))) {
    			if (!(status = double_element(log, node, "Delay", NULL,
    					&decimation->delay))) {
    				status = double_element(log, node, "Correction", NULL,
    						&decimation->correction);
    			}
    		}
    	}
    }

    return status;
}


/* Read an x2r_gain value from the current node. */
static int parse_gain(x2r_log *log, mxml_node_t *node, x2r_gain *gain) {

    int status = X2R_OK;

    if (!(status = double_element(log, node, "Value", "0.0", &gain->value))) {
    	status = double_element(log, node, "Frequency", "0.0", &gain->frequency);
    }

    return status;
}


/* Set the stage type (checking it is not already set). */
static int set_stage(x2r_log *log, x2r_stage *stage, x2r_stage_type type) {
    if (stage->type) {
        return x2r_error(log, X2R_ERR_XML, "Multiple content in a single stage");
    } else {
        stage->type = type;
        return X2R_OK;
    }
}


/* Read an x2r_stage value from the current node. */
static int parse_stage(x2r_log *log, mxml_node_t *node, x2r_stage *stage) {

    int status = X2R_OK, found;
    mxml_node_t *poles_zeros, *coefficients, *response_list, *fir, *polynomial;
    mxml_node_t *decimation, *stage_gain;

    if (!(status = int_attribute(log, node, "number", NULL, &stage->number))) {
    	x2r_debug(log, "Parsing stage %d", stage->number);
    }

    if (!status) {
    	status = find_child(log, &poles_zeros, &found, node, "PolesZeros");
    	if (!status && found) {
    		if (!(status = set_stage(log, stage, X2R_STAGE_POLES_ZEROS))) {
    			if (!(stage->u.poles_zeros = calloc(1, sizeof(*stage->u.poles_zeros)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc poles_zeros");
    			} else {
    				status = parse_poles_zeros(log, poles_zeros, stage->u.poles_zeros);
    			}
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &coefficients, &found, node, "Coefficients");
    	if (!status && found) {
    		if (!(status = set_stage(log, stage, X2R_STAGE_COEFFICIENTS))) {
    			if (!(stage->u.coefficients = calloc(1, sizeof(*stage->u.coefficients)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc coefficients");
    			} else {
    				status = parse_coefficients(log, coefficients, stage->u.coefficients);
    			}
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &response_list, &found, node, "ResponseList");
    	if (!status && found) {
    		if (!(status = set_stage(log, stage, X2R_STAGE_RESPONSE_LIST))) {
    			if (!(stage->u.response_list = calloc(1, sizeof(*stage->u.response_list)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc response_list");
    			} else {
    				status = parse_response_list(log, response_list, stage->u.response_list);
    			}
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &fir, &found, node, "FIR");
    	if (!status && found) {
    		if (!(status = set_stage(log, stage, X2R_STAGE_FIR))) {
    			if (!(stage->u.fir = calloc(1, sizeof(*stage->u.fir)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc fir");
    			} else {
    				status = parse_fir(log, fir, stage->u.fir);
    			}
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &polynomial, &found, node, "Polynomial");
    	if (!status && found) {
    		if (!(status = set_stage(log, stage, X2R_STAGE_POLYNOMIAL))) {
    			if (!(stage->u.polynomial = calloc(1, sizeof(*stage->u.polynomial)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc polynomial");
    			} else {
    				status = parse_polynomial(log, polynomial, stage->u.polynomial);
    			}
    		}
    	}
    }

    if (!status && !stage->type) {
        x2r_warn(log, "No content in stage (during parse)");
    }

    if (!status) {
    	status = find_child(log, &decimation, &found, node, "Decimation");
    	if (!status && found) {
    		if (!(stage->decimation = calloc(1, sizeof(*stage->decimation)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc decimation");
    		} else {
    			status = parse_decimation(log, decimation, stage->decimation);
    		}
    	}
    }

    if (!status) {
    	// the schema says this is required, i think, but IRIS-WS happily works with data
    	// without it.
    	status = find_child(log, &stage_gain, &found, node, "StageGain");
    	if (!status && found) {
    		if (!(stage->stage_gain = calloc(1, sizeof(*stage->stage_gain)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stage_gain");
    		} else {
    			status = parse_gain(log, stage_gain, stage->stage_gain);
    		}
    	}
    }

    return status;
}


/* Free an x2r_stage value. */
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


/* Read an x2r_response value from the current node. */
static int parse_response(x2r_log *log, mxml_node_t *node, x2r_response *response) {

    int status = X2R_OK, i, found;
    nodelist *stages = NULL;
    mxml_node_t *instrument_sensitivity;
    mxml_node_t *instrument_polynomial;

    x2r_debug(log, "Parsing response");

    if (!(status = find_children(log, &stages, node, "Stage"))) {
    	response->n_stages = stages->n;
    	if (!(response->stage = calloc(response->n_stages, sizeof(*response->stage)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stages");
    	} else {
    		for (i = 0; !status && i < response->n_stages; ++i) {
    			status = parse_stage(log, stages->node[i], &response->stage[i]);
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &instrument_sensitivity, &found, node, "InstrumentSensitivity");
    	if (!status && found) {
    		if (!(response->instrument_sensitivity =
    				calloc(1, sizeof(*response->instrument_sensitivity)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc instrument_sensitivity");
    		} else {
    			status = parse_gain(log, instrument_sensitivity, response->instrument_sensitivity);
    		}
    	}
    }

    if (!status) {
    	status = find_child(log, &instrument_polynomial, &found, node, "InstrumentPolynomial");
    	if (!status && found) {
    		if (!(response->instrument_polynomial =
    				calloc(1, sizeof(*response->instrument_polynomial)))) {
    			status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc instrument_polynomial");
    		} else {
    			status = parse_polynomial(log, instrument_polynomial, response->instrument_polynomial);
    		}
    	}
    }

    free_nodelist(&stages);
    return status;
}


/* Free an x2r_response value. */
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


/* Read an x2r_channel value from the current node. */
static int parse_channel(x2r_log *log, mxml_node_t *node, x2r_channel *channel) {

    int status = X2R_OK;
    mxml_node_t *response;

    x2r_debug(log, "Parsing channel");

    if (!(status = char_attribute(log, node, "code", NULL, &channel->code))) {
    	if (!(status = char_attribute(log, node, "locationCode", NULL, &channel->location_code))) {
    		if (!(status = datetime_attribute(log, node, "startDate", &channel->start_date))) {
    			status = datetime_attribute(log, node, "endDate", &channel->end_date);
    		}
    	}
    }

    if (!status) {
		if (!(status = find_child(log, &response, NULL, node, "Response"))) {
			status = parse_response(log, response, &channel->response);
		}
    }

    return status;
}


/* Free an x2r_channel value. */
static int free_channel(x2r_channel *channel, int status) {
    if (channel) {
        status = free_response(&channel->response, status);
        free(channel->location_code);
        free(channel->code);
    }
    return status;
}


/* Read an x2r_station value from the current node. */
static int parse_station(x2r_log *log, mxml_node_t *node, x2r_station *station) {

    int status = X2R_OK, i;
    nodelist *stations = NULL;

    x2r_debug(log, "Parsing station");

    if (!(status = find_children(log, &stations, node, "Channel"))) {
    	station->n_channels = stations->n;
    	if (!(station->channel = calloc(station->n_channels, sizeof(*station->channel)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc channels");
    	} else {
    		if (!(status = char_attribute(log, node, "code", NULL, &station->code))) {
    			for (i = 0; !status && i < station->n_channels; ++i) {
    				status = parse_channel(log, stations->node[i], &station->channel[i]);
    			}
    		}
    	}
    }

    free_nodelist(&stations);
    return status;
}


/* Free an x2r_station value. */
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


/* Read an x2r_network value from the current node. */
static int parse_network(x2r_log *log, mxml_node_t *node, x2r_network *network) {

    int status = X2R_OK, i;
    nodelist *stations = NULL;

    x2r_debug(log, "Parsing network");

    if (!(status = find_children(log, &stations, node, "Station"))) {
    	network->n_stations = stations->n;
    	if (!(network->station = calloc(network->n_stations, sizeof(*network->station)))) {
    		status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc stations");
    	} else {
    		if (!(status = char_attribute(log, node, "code", NULL, &network->code))) {
    			for (i = 0; !status && i < network->n_stations; ++i) {
    				status = parse_station(log, stations->node[i], &network->station[i]);
    			}
    		}
    	}
    }

    free_nodelist(&stations);
    return status;
}


/* Free an x2r_network value. */
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


/* Read an x2r_fdsn_station_xml value from the current node. */
static int parse_fdsn_station_xml(x2r_log *log, mxml_node_t *doc, x2r_fdsn_station_xml **root) {

    int status = X2R_OK, i;
    mxml_node_t *fdsn = NULL;
    nodelist *networks = NULL;

    x2r_debug(log, "Parsing root");

    if (!(*root = calloc(1, sizeof(**root)))) {
        status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc fdsn_station_xml");
    } else {
    	if (!(status = find_child(log, &fdsn, NULL, doc, "FDSNStationXML"))) {
    		if (!(status = find_children(log, &networks, fdsn, "Network"))) {
    			(*root)->n_networks = networks->n;
    			if (!((*root)->network = calloc((*root)->n_networks, sizeof(*(*root)->network)))) {
    				status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc networks");
    			} else {
    				for (i = 0; !status && i < (*root)->n_networks; ++i) {
    					status = parse_network(log, networks->node[i], &(*root)->network[i]);
    				}
    			}
    		}
    	}
    }

    free_nodelist(&networks);
    return status;
}


/* Free an x2r_fdsn_station_xml value. */
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


/*
 * The equivalent of StationService.load() in IRIS-WS, constructing an in-memory
 * representation of the station.xml file read from the given stream.
 */
int x2r_station_service_load(x2r_log *log, FILE *in, x2r_fdsn_station_xml **root) {

    int status = X2R_OK;
    mxml_node_t *doc = NULL;

    if (!(status = stream2doc(log, in, &doc))) {
    	status = parse_fdsn_station_xml(log, doc, root);
    }

    mxmlDelete(doc);
    return status;
}


