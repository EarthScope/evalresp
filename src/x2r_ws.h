
#ifndef X2R_WS_H
#define X2R_WS_H

#include <stdio.h>

#include "x2r_log.h"
#include "x2r_xml.h"

int x2r_resp_util_write(x2r_log *log, FILE *out, const x2r_fdsn_station_xml *root);

#endif
