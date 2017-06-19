#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "evalresp/public_api.h"

int evalresp_response_to_char (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_format format, char **output)
{
    int num_of_points = 0;
    int i, length, offset;
    double *amp_arr = NULL;
    double *pha_arr = NULL;
    double *freq_arr = NULL;

    if (*output)
    {
        /* don't want to get bad memory location to write too
         * or we don't want to zombify stuff */
        evalresp_log(log, EV_ERROR, EV_ERROR, "cannot out put to an already allocated output");
        return EVALRESP_MEM;
    }

    if (!response)
    {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot Process Empty Response");
        return EVALRESP_IO;
    }

    num_of_points = response->nfreqs;
    /*XXX make sure we are dealing with interpolated freq as well before deleting this comment */
    freq_arr = response->freqs;
    
    switch (format)
    {
        case evalresp_fap_format:
            amp_arr = (double *) calloc(num_of_points, sizeof(double));
            pha_arr = (double *) calloc(num_of_points, sizeof(double));
            break;
        case evalresp_amplitude_format:
            amp_arr = (double *) calloc(num_of_points, sizeof(double));
            break;
        case evalresp_phase_format:
            pha_arr = (double *) calloc(num_of_points, sizeof(double));
            break;
        case evalresp_complex_format:
            /* this is in here to act as a catch for bad cases */
            break;
        default:
            return EVALRESP_IO; /* TODO should this be EVALRESP_INVALID_OPT?) */
    }
    for (i = 0; i < num_of_points; i++)
    {
        if (amp_arr)
        {
            amp_arr[i] = sqrt (response->rvec[i].real * response->rvec[i].real + response->rvec[i].imag * response->rvec[i].imag);
        }
        if (pha_arr)
        {
            pha_arr[i] = atan2 (response->rvec[i].imag, response->rvec[i].real + 1.e-200) * 180.0 / Pi;
        }
     }
    /* Get Size of output string */
    for (i = 0, length = 0; i < num_of_points; i++)
    {
        switch (format)
        {
            case evalresp_fap_format:
                length += snprintf(NULL, 0, "%.6E  %.6E  %.6E\n", freq_arr[i], amp_arr[i], pha_arr[i]);
                break;
            case evalresp_amplitude_format:
                length += snprintf(NULL, 0, "%.6E  %.6E\n", freq_arr[i], amp_arr[i]);
                break;
            case evalresp_phase_format:
                length += snprintf(NULL, 0, "%.6E  %.6E\n", freq_arr[i], pha_arr[i]);
                break;
            case evalresp_complex_format:
                length += snprintf(NULL, 0, "%.6E  %.6E  %.6E\n", freq_arr[i], response->rvec[i].real, response->rvec[i].imag);
                break;
        }
    }
    length++; /* Adding one for NULL termination */

    *output = (char *) calloc (length, sizeof(char));

    for (i = 0, offset = 0; i < num_of_points; i++)
    {
        switch (format)
        {
            case evalresp_fap_format:
                offset += snprintf(*output + offset, length - offset, "%.6E  %.6E  %.6E\n", freq_arr[i], amp_arr[i], pha_arr[i]);
                break;
            case evalresp_amplitude_format:
                offset += snprintf(*output + length, length - offset, "%.6E  %.6E\n", freq_arr[i], amp_arr[i]);
                break;
            case evalresp_phase_format:
                offset += snprintf(*output + offset, length - offset, "%.6E  %.6E\n", freq_arr[i], pha_arr[i]);
                break;
            case evalresp_complex_format:
                offset += snprintf(*output + offset, length - offset, "%.6E  %.6E  %.6E\n", freq_arr[i], response->rvec[i].real, response->rvec[i].imag);
                break;
        }
    }

    if (amp_arr)
    {
        free(amp_arr);
    }
    amp_arr = NULL;
    if (pha_arr)
    {
        free(pha_arr);
    }
    pha_arr = NULL;

    return EVALRESP_OK;
}
