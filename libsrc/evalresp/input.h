
#ifndef EVALRESP_INPUT_H
#define EVALRESP_INPUT_H

#include "evalresp_log/log.h"

// private functions exposed only for testing

void
slurp_line (char **seed, char *line, int maxlen);

int
read_line (evalresp_log_t *log, char **seed, char *return_line, int blkt_no, int fld_no, char *sep);

int
read_field (evalresp_log_t *log, char **seed, char *return_field,
    int blkt_no, int fld_no, char *sep, int fld_wanted);

#endif
