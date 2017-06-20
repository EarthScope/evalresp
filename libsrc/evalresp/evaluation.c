
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code as a clean wrapper for calc_resp etc.

int
evalresp_new_options (evalresp_log_t *log, evalresp_options **options)
{
  int status = EVALRESP_OK;
  if (!(*options = calloc (1, sizeof (**options))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate options");
    status = EVALRESP_MEM;
  }
  else
  {
    (*options)->start_stage = EVALRESP_ALL_STAGES;
    (*options)->stop_stage = EVALRESP_ALL_STAGES;
  }
  return status;
}

void
evalresp_free_options (evalresp_options **options)
{
  if (*options)
  {
    free (*options);
    *options = NULL;
  }
}

int
evalresp_set_frequency (evalresp_log_t *log, evalresp_options *options,
                        const char *min_freq, const char *max_freq, const char *nfreq)
{
  int status = EVALRESP_OK;
  if (!(status = parse_double (log, "minimum frequency", min_freq, &options->min_freq)))
  {
    if (!(status = parse_double (log, "maximum frequency", max_freq, &options->max_freq)))
    {
      if (!(status = parse_int (log, "number of frequency bins", nfreq, &options->nfreq)))
      {
        if (options->max_freq < options->min_freq)
        {
          double tmp = options->max_freq;
          options->max_freq = options->min_freq;
          options->min_freq = tmp;
        }
      }
    }
  }
  return status;
}

typedef struct
{
  int index;
  const char *str;
} option_pair;

static int
parse_option (evalresp_log_t *log, const char *name, int noptions, option_pair *options,
              const char *str, int *value)
{
  int status = EVALRESP_OK, i, found = 0;
  ;
  char *copy;
  copy = strdup (str);
  for (i = 0; i < strlen (copy); ++i)
  {
    copy[i] = toupper (copy[i]);
  }
  for (i = 0; i < noptions; ++i)
  {
    if (!strcmp (options[i].str, str))
    {
      *value = options[i].index;
      found = 1;
    }
  }
  if (!found)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Could not match '%s' for %s", str, name);
    status = EVALRESP_INP;
  }
  return status;
}

static option_pair formats[] = {
    {evalresp_ap_user_format, "AP"},
    {evalresp_fap_user_format, "CS"},
    {evalresp_complex_user_format, "FAP"}};

int
evalresp_set_format (evalresp_log_t *log, evalresp_options *options,
                     const char *format)
{
  int status = EVALRESP_OK, value;
  if (!(status = parse_option (log, "file format", sizeof (formats) / sizeof (formats[0]),
                               formats, format, &value)))
  {
    options->format = value;
  }
  return status;
}

static option_pair units[] = {
    {evalresp_default_user_unit, "DEF"},
    {evalresp_displacement_user_unit, "DIS"},
    {evalresp_velocity_user_unit, "VEL"},
    {evalresp_acceleration_user_unit, "ACC"}};

int evalresp_set_unit (evalresp_log_t *log, evalresp_options *options,
                       const char *unit)
{
  int status = EVALRESP_OK, value;
  if (!(status = parse_option (log, "output unit", sizeof (units) / sizeof (units[0]),
                               formats, unit, &value)))
  {
    options->unit = value;
  }
  return status;
}

static const char *get_unit(evalresp_user_unit unit) {
  int i, n = sizeof (units) / sizeof (units[0]);
  for (i = 0; i < n; ++i) {
    if (unit == units[i].index) {
      return units[i].str;
    }
  }
  return units[0].str;  // default
}

int
evalresp_set_start_stage (evalresp_log_t *log, evalresp_options *options,
                          const char *stage)
{
  return parse_int (log, "start stage", stage, &options->start_stage);
}

int
evalresp_set_stop_stage (evalresp_log_t *log, evalresp_options *options,
                         const char *stage)
{
  return parse_int (log, "stop stage", stage, &options->stop_stage);
}

int
evalresp_set_b62_x (evalresp_log_t *log, evalresp_options *options,
                    const char *b62_x)
{
  return parse_double (log, "block 62 x value", b62_x, &options->b62_x);
}

// don't use alloc_response because it does too much
static int
local_alloc_response(evalresp_log_t *log, evalresp_response **response) {
  int status = EVALRESP_OK;
  if (!(*response = calloc(1, sizeof(**response)))) {
    evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot allocate response");
    status = EVALRESP_MEM;
  }
  return status;
}

static int
is_block_55(const evalresp_channel *channel) {
  return channel && channel->first_stage && channel->first_stage->first_blkt
      && channel->first_stage->first_blkt->type == LIST;
}

static int
calculate_default_freqs (evalresp_log_t *log, evalresp_options *options,
    evalresp_response *response)
{
  return 0;
}

static int
interpolate_b55 (evalresp_log_t *log, evalresp_channel *channel,
    evalresp_blkt **b55_save) {
  return 0;
}

static int
use_b55_freqs (evalresp_log_t *log, evalresp_channel *channel,
    evalresp_response *response)
{
  return 0;
}

static void
restore_b55 (evalresp_channel *channel, evalresp_blkt *b55_save) {
}

int
evalresp_channel_to_response (evalresp_log_t *log, evalresp_channel *channel,
                              evalresp_options *options, evalresp_response **response)
{
  int status = EVALRESP_OK;
  evalresp_blkt *b55_save = NULL;
  int is_55 = is_block_55(channel);

  if (!(status = local_alloc_response(log, response)))
  {
    if (!(status = calculate_default_freqs(log, options, *response)))
    {
      if (is_55)
      {
        /* if it's a b55 block then calc_resp is just going to copy its data
         * to the response.  so we need to make sure that frequencies agree
         * beforehand.  either by interpolating the blockette data (in which
         * case we keep a backup and restore it below) or by changing the
         * output frequencies to match.
         */
        if (options->b55_interpolate) {
          status = interpolate_b55 (log, channel, &b55_save);
        } else {
          status = use_b55_freqs (log, channel, *response);
        }
      }
    }
  }

  if (!status) {
    calc_resp(channel, (*response)->freqs, (*response)->nfreqs, (*response)->rvec,
        get_unit(options->unit), options->start_stage, options->stop_stage,
        options->use_total_sensitivity, options->b62_x, log);
  }

  if (b55_save)
  {
    restore_b55(channel, b55_save);
  }

  return status;
}
