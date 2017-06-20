
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code as a clean wrapper for calc_resp etc.

int evalresp_new_options (evalresp_log_t *log, evalresp_options **options)
{
  int status = EVALRESP_OK;
  if (!(*options = calloc(1, sizeof(**options)))) {
    evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate options");
    status = EVALRESP_MEM;
  } else {
    (*options)->start_stage = EVALRESP_ALL_STAGES;
    (*options)->stop_stage = EVALRESP_ALL_STAGES;
  }
  return status;
}

void evalresp_free_options (evalresp_options **options) {
  if (*options) {
    free(*options);
    *options = NULL;
  }
}

int evalresp_set_frequency (evalresp_log_t *log, evalresp_options *options,
    const char *min_freq, const char *max_freq, const char *nfreq)
{
  int status = EVALRESP_OK;
  if (!(status = parse_double(log, "minimum frequency", min_freq, &options->min_freq))) {
    if (!(status = parse_double(log, "maximum frequency", max_freq, &options->max_freq))) {
      if (!(status = parse_int(log, "number of frequency bins", nfreq, &options->nfreq))) {
        if (options->max_freq < options->min_freq) {
          double tmp = options->max_freq;
          options->max_freq = options->min_freq;
          options->min_freq = tmp;
        }
      }
    }
  }
  return status;
}

typedef struct {
  int index;
  const char *str;
} option_pair;

static option_pair formats[] = {
    {evalresp_ap_user_format, "ap"},
    {evalresp_fap_user_format, "cs"},
    {evalresp_complex_user_format, "fap"}
};

static int
parse_option (evalresp_log_t *log, const char *name, int noptions, option_pair *options,
    const char *str, int *value)
{
  int status = EVALRESP_OK, i, found = 0;;
  char *copy;
  copy = strdup(str);
  for (i = 0; i < strlen(copy); ++i) {
    copy[i] = tolower(copy[i]);
  }
  for (i = 0; i < noptions; ++i) {
    if (!strcmp(options[i].str, str)) {
      *value = options[i].index;
      found = 1;
    }
  }
  if (!found) {
    evalresp_log(log, EV_ERROR, EV_ERROR, "Could not match '%s' for %s", str, name);
    status = EVALRESP_INP;
  }
  return status;
}

int evalresp_set_format (evalresp_log_t *log, evalresp_options *options,
    const char *format)
{
  int status = EVALRESP_OK, value;
  if (!(status = parse_option(log, "file format", sizeof(formats) / sizeof(formats[0]),
      formats, format, &value)))
  {
    options->format = value;
  }
  return status;
}

// these are separate because it simplifies calling from main routine

int evalresp_set_start_stage (evalresp_log_t *log, evalresp_options *options,
    const char *stage)
{
  return parse_int(log, "start stage", stage, &options->start_stage);
}

int evalresp_set_stop_stage (evalresp_log_t *log, evalresp_options *options,
    const char *stage)
{
  return parse_int(log, "stop stage", stage, &options->stop_stage);
}

int evalresp_set_b62_x (evalresp_log_t *log, evalresp_options *options,
    const char *b62_x)
{
  return parse_double(log, "block 62 x value", b62_x, &options->b62_x);
}
