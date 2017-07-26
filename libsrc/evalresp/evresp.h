#ifndef __EVRESP_H__
#define __EVRESP_H__

#include <evalresp/public_compat.h>
/*NOTE THIS HEADER IS ONLY A LEGACY INTERFACE*/
/* DO NOT USE UNLESS YOU NEED DEPRECATED METHODS and TYPES */

// once this is defined (v4.0.6 onwards), some functions take an additional
// xml flag argument
#define EVRESP_XML

/* old struct defines */
#define channel evalresp_channel_s
#define stage evalresp_stage_s
#define blkt evalresp_blkt_s
#define pole_zeroType evalresp_pole_zero_s
#define genericType evalresp_generic_s
#define firType evalresp_fir_s
#define listType evalresp_list_s
#define decimationType evalresp_decimation_s
#define gainType evalresp_gain_s

#endif /* __EVRESP_H__ */
