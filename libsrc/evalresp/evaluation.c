
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
    (*options)->min_freq = EVALRESP_NO_FREQ;
    (*options)->max_freq = EVALRESP_NO_FREQ;
    (*options)->nfreq = 1;
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
      status = parse_int (log, "number of frequency bins", nfreq, &options->nfreq);
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
    {evalresp_ap_output_format, "AP"},
    {evalresp_fap_output_format, "CS"},
    {evalresp_complex_output_format, "FAP"}};

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
    {evalresp_default_unit, "DEF"},
    {evalresp_displacement_unit, "DIS"},
    {evalresp_velocity_unit, "VEL"},
    {evalresp_acceleration_unit, "ACC"}};

int
evalresp_set_unit (evalresp_log_t *log, evalresp_options *options,
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

static const char *
get_unit (evalresp_unit unit)
{
  int i, n = sizeof (units) / sizeof (units[0]);
  for (i = 0; i < n; ++i)
  {
    if (unit == units[i].index)
    {
      return units[i].str;
    }
  }
  return units[0].str; // default
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
local_alloc_response (evalresp_log_t *log, evalresp_response **response)
{
  int status = EVALRESP_OK;
  if (!(*response = calloc (1, sizeof (**response))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate response");
    status = EVALRESP_MEM;
  }
  return status;
}

static int
is_block_55 (const evalresp_channel *channel)
{
  return channel && channel->first_stage && channel->first_stage->first_blkt && channel->first_stage->first_blkt->type == LIST;
}

static int
validate_freqs (evalresp_log_t *log, evalresp_options *options)
{
  int status = EVALRESP_OK;

  options->min_freq = options->min_freq < 0 ? 1.0 : options->min_freq;
  options->max_freq = options->max_freq < 0 ? options->min_freq : options->max_freq;
  if (options->max_freq < options->min_freq)
  {
    double tmp = options->max_freq;
    options->max_freq = options->min_freq;
    options->min_freq = tmp;
  }
  if (!options->lin_freq && options->min_freq == 0)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot have zero frequency with logarithmic spacing");
    status = EVALRESP_INP;
  }
  else if (options->nfreq < 1)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot have a negative number of frequency bins");
    status = EVALRESP_INP;
  }

  return status;
}

static int
calculate_default_freqs (evalresp_log_t *log, evalresp_options *options,
                         evalresp_response *response)
{
  int status = EVALRESP_OK, i;
  double delta = 0, lo, hi;

  if (!(status = validate_freqs (log, options)))
  {
    if (options->lin_freq)
    {
      lo = options->min_freq;
      hi = options->max_freq;
    }
    else
    {
      lo = log10 (options->min_freq);
      hi = log10 (options->max_freq);
    }
    delta = options->nfreq == 1 ? 0 : (hi - lo) / (options->nfreq - 1);
    if (!(status = calloc_doubles (log, "frequencies", options->nfreq, &response->freqs)))
    {
      response->nfreqs = options->nfreq;
      for (i = 0; i < response->nfreqs; ++i)
      {
        response->freqs[i] = lo + i * delta;
        if (!options->lin_freq)
        {
          response->freqs[i] = pow (10, response->freqs[i]);
        }
      }
    }
  }

  if (!status)
  {
    if (!(response->rvec = calloc (response->nfreqs, sizeof (*response->rvec))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate complex result");
      status = EVALRESP_MEM;
    }
  }

  return status;
}

static int
save_doubles (evalresp_log_t *log, const char *name, int n, double **dest, double *src)
{
  int status = EVALRESP_OK, i;
  if (!(status = calloc_doubles (log, name, n, dest)))
  {
    for (i = 0; i < n; ++i)
    {
      (*dest)[i] = src[i];
    }
  }
  return status;
}

static int
save_b55 (evalresp_log_t *log, evalresp_channel *channel,
          evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK;
  evalresp_blkt *b55 = channel->first_stage->first_blkt;
  evalresp_list *list = &b55->blkt_info.list;
  if (!(*b55_save = calloc (1, sizeof (**b55_save))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate blockette 55");
    status = EVALRESP_MEM;
  }
  else
  {
    memcpy (*b55_save, b55, sizeof (*b55));
    /* interpolation will free the freq, phase and amp arrays, so we must
     * allocate further copies
     */
    if (!(status = save_doubles (log, "b55 frequencies", list->nresp,
                                 &(*b55_save)->blkt_info.list.freq, list->freq)))
    {
      if (!(status = save_doubles (log, "b55 amplitudes", list->nresp,
                                   &(*b55_save)->blkt_info.list.amp, list->amp)))
      {
        status = save_doubles (log, "b55 phases", list->nresp,
                               &(*b55_save)->blkt_info.list.phase, list->phase);
      }
    }
  }
  return status;
}

static int
interpolate_b55 (evalresp_log_t *log, evalresp_channel *channel,
                 evalresp_response *response, evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK;
  evalresp_blkt *b55 = channel->first_stage->first_blkt;
  evalresp_list *list = &b55->blkt_info.list;
  if (!(status = save_b55 (log, channel, b55_save)))
  {
    interpolate_list_blockette (&list->freq, &list->amp, &list->phase, &list->nresp,
                                response->freqs, response->nfreqs, log);
  }
  return status;
}

static int
use_b55_freqs (evalresp_log_t *log, evalresp_channel *channel,
               evalresp_response *response)
{
  int status = EVALRESP_OK;
  evalresp_blkt *b55 = channel->first_stage->first_blkt;
  evalresp_list *list = &b55->blkt_info.list;
  free (response->freqs);
  if (!(status = calloc_doubles (log, "frequency array", list->nresp, &response->freqs)))
  {
    memcpy (response->freqs, list->freq, list->nresp * sizeof (*response->freqs));
    response->nfreqs = list->nresp;
  }
  if (!status)
  {
    free (response->rvec);
    if (!(response->rvec = calloc (response->nfreqs, sizeof (*response->rvec))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate complex result");
      status = EVALRESP_MEM;
    }
  }
  return status;
}

static void
restore_b55 (evalresp_channel *channel, evalresp_blkt *b55_save)
{
  evalresp_blkt *b55 = channel->first_stage->first_blkt;
  evalresp_list *list = &b55->blkt_info.list;
  evalresp_list *list_save = &b55_save->blkt_info.list;
  list->nresp = list_save->nresp;
  free (list->freq);
  list->freq = list_save->freq;
  free (list->amp);
  list->amp = list_save->amp;
  free (list->phase);
  list->phase = list_save->phase;
}

int
evalresp_channel_to_response (evalresp_log_t *log, evalresp_channel *channel,
                              evalresp_options *options, evalresp_response **response)
{
  int status = EVALRESP_OK, free_options = 0;
  evalresp_blkt *b55_save = NULL;

  /* allow NULL options */
  if (!options)
  {
    status = evalresp_new_options (log, &options);
    free_options = 1;
  }

  if (!(status = local_alloc_response (log, response)))
  {
    if (!(status = calculate_default_freqs (log, options, *response)))
    {
      if (is_block_55 (channel))
      {
        /* if it's a b55 block then calc_resp is just going to copy its data
         * to the response.  so we need to make sure that frequencies agree
         * beforehand.  either by interpolating the blockette data (in which
         * case we keep a backup and restore it below) or by changing the
         * output frequencies to match.
         */
        if (options->b55_interpolate)
        {
          status = interpolate_b55 (log, channel, *response, &b55_save);
        }
        else
        {
          status = use_b55_freqs (log, channel, *response);
        }
      }
    }
  }

  if (!status)
  {
    norm_resp (channel, options->start_stage, options->stop_stage, log);
    calc_resp (channel, (*response)->freqs, (*response)->nfreqs, (*response)->rvec,
               get_unit (options->unit), options->start_stage, options->stop_stage,
               options->use_total_sensitivity, options->b62_x, log);
  }

  if (b55_save)
  {
    restore_b55 (channel, b55_save);
  }
  if (free_options)
  {
    evalresp_free_options (&options);
  }
  free (b55_save);

  return status;
}
