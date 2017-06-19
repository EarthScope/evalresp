#include "evalresp/public_api.h"

int evalresp_response_to_char (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_format format, char **output)
{
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
    
    switch (format)
    {
        case evalresp_fap_format:
            break;
        case evalresp_amplitude_format:
            break;
        case evalresp_phase_format:
            break;
        case evalresp_complex_format:
            break;
        default:
            return EVALRESP_IO; /* TODO should this be EVALRESP_INVALID_OPT?) */
    }
    return EVALRESP_OK;
}
