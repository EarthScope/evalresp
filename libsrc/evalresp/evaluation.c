
#include <stdlib.h>
#include <string.h>

/* NEEDED for M_PI on windows */
#define _USE_MATH_DEFINES
#include <math.h>

#include "./private.h"
#include "evalresp/constants.h"
#include "evalresp/public.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code as a clean wrapper for calc_resp etc.

int
evalresp_new_options (evalresp_logger *log, evalresp_options **options)
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
    (*options)->unit = evalresp_velocity_unit;
  }
  return status;
}

void
evalresp_free_options (evalresp_options **options)
{
  if (*options)
  {
    free ((*options)->filename);
    free (*options);
    *options = NULL;
  }
}

int
evalresp_set_filename (evalresp_logger *log, evalresp_options *options, const char *filename)
{
  options->filename = strdup (filename);
  return EVALRESP_OK;
}

int
evalresp_set_frequency (evalresp_logger *log, evalresp_options *options,
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
parse_option (evalresp_logger *log, const char *name, int noptions, option_pair *options,
              const char *str, int *value)
{
  int status = EVALRESP_OK, i, found = 0;
  size_t k;
  char *copy;
  copy = strdup (str);
  for (k = 0; k < strlen (copy); ++k)
  {
    copy[k] = toupper (copy[k]);
  }
  for (i = 0; i < noptions; ++i)
  {
    if (!strcmp (options[i].str, copy))
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
  free (copy);
  return status;
}

static option_pair formats[] = {
    {evalresp_ap_output_format, "AP"},
    {evalresp_fap_output_format, "FAP"},
    {evalresp_complex_output_format, "CS"}};

int
evalresp_set_format (evalresp_logger *log, evalresp_options *options,
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
    {evalresp_file_unit, "DEF"},
    {evalresp_displacement_unit, "DIS"},
    {evalresp_velocity_unit, "VEL"},
    {evalresp_acceleration_unit, "ACC"}};

int
evalresp_set_unit (evalresp_logger *log, evalresp_options *options,
                   const char *unit)
{
  int status = EVALRESP_OK, value;
  if (!(status = parse_option (log, "output unit", sizeof (units) / sizeof (units[0]),
                               units, unit, &value)))
  {
    options->unit = value;
  }
  return status;
}

const char *
evalresp_unit_string (const evalresp_unit unit)
{
  if (unit == evalresp_displacement_unit)
    return "Displacement (m)";
  else if (unit == evalresp_velocity_unit)
    return "Velocity (m/s)";
  else if (unit == evalresp_acceleration_unit)
    return "Acceleration (m/s**2)";
  else if (unit == evalresp_file_unit)
    return "Documented response unit";
  else
    return "Unrecognized response unit";
}

static option_pair spacings[] = {
    {0, "LOG"},
    {1, "LIN"}};

int
evalresp_set_spacing (evalresp_logger *log, evalresp_options *options,
                      const char *spacing)
{
  int status = EVALRESP_OK, value;
  if (!(status = parse_option (log, "frequency spacing", sizeof (spacings) / sizeof (spacings[0]),
                               spacings, spacing, &value)))
  {
    options->lin_freq = value;
  }
  return status;
}

int
evalresp_set_start_stage (evalresp_logger *log, evalresp_options *options,
                          const char *stage)
{
  return parse_int (log, "start stage", stage, &options->start_stage);
}

int
evalresp_set_stop_stage (evalresp_logger *log, evalresp_options *options,
                         const char *stage)
{
  return parse_int (log, "stop stage", stage, &options->stop_stage);
}

int
evalresp_set_b62_x (evalresp_logger *log, evalresp_options *options,
                    const char *b62_x)
{
  return parse_double (log, "block 62 x value", b62_x, &options->b62_x);
}

// don't use alloc_response because it does too much
static int
local_alloc_response (evalresp_logger *log, evalresp_response **response)
{
  int status = EVALRESP_OK;
  if (!(*response = calloc (1, sizeof (**response))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate response");
    status = EVALRESP_MEM;
  }
  return status;
}

/* The list blockettes of the stages that are evaluated.  *lists is NULL when there are none. */
static int
selected_lists (evalresp_logger *log, const evalresp_options *options,
                evalresp_channel *channel, evalresp_blkt ***lists, int *nlists)
{
  evalresp_stage *stage;
  evalresp_blkt *blkt;
  int n = 0;

  *lists = NULL;
  *nlists = 0;
  for (stage = channel->first_stage; stage; stage = stage->next_stage)
  {
    if (!stage_is_selected (options, stage))
      continue;
    for (blkt = stage->first_blkt; blkt; blkt = blkt->next_blkt)
    {
      if (blkt->type == LIST)
        n++;
    }
  }
  if (!n)
    return EVALRESP_OK;
  if (!(*lists = calloc (n, sizeof (**lists))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate list of blockettes 55");
    return EVALRESP_MEM;
  }
  for (stage = channel->first_stage; stage; stage = stage->next_stage)
  {
    if (!stage_is_selected (options, stage))
      continue;
    for (blkt = stage->first_blkt; blkt; blkt = blkt->next_blkt)
    {
      if (blkt->type == LIST)
        (*lists)[(*nlists)++] = blkt;
    }
  }
  return EVALRESP_OK;
}

static int
validate_freqs (evalresp_logger *log, evalresp_options *options)
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
calculate_default_freqs (evalresp_logger *log, evalresp_options *options,
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
save_doubles (evalresp_logger *log, const char *name, int n, double **dest, const double *src)
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

/* Use the caller's frequencies, which must be non-negative. */
static int
copy_requested_freqs (evalresp_logger *log, const double *freqs, int nfreqs,
                      evalresp_response *response)
{
  int status = EVALRESP_OK, i;

  if (nfreqs < 1)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot have fewer than one frequency");
    status = EVALRESP_INP;
  }
  for (i = 0; !status && i < nfreqs; ++i)
  {
    if (!(freqs[i] >= 0))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot have a negative frequency");
      status = EVALRESP_INP;
    }
  }
  if (!status && !(status = save_doubles (log, "frequencies", nfreqs, &response->freqs, freqs)))
  {
    response->nfreqs = nfreqs;
    if (!(response->rvec = calloc (response->nfreqs, sizeof (*response->rvec))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate complex result");
      status = EVALRESP_MEM;
    }
  }

  return status;
}

static void
discard_b55 (evalresp_blkt *b55_save)
{
  if (b55_save)
  {
    free (b55_save->blkt_info.list.freq);
    free (b55_save->blkt_info.list.amp);
    free (b55_save->blkt_info.list.phase);
    free (b55_save);
  }
}

static int
save_b55 (evalresp_logger *log, evalresp_blkt *b55, evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK;
  evalresp_list *list = &b55->blkt_info.list;
  if (!(*b55_save = calloc (1, sizeof (**b55_save))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate blockette 55");
    status = EVALRESP_MEM;
  }
  else
  {
    memcpy (*b55_save, b55, sizeof (*b55));
    /* interpolation will free the freq, phase and amp arrays, so the copy
     * owns further copies
     */
    (*b55_save)->blkt_info.list.freq = NULL;
    (*b55_save)->blkt_info.list.amp = NULL;
    (*b55_save)->blkt_info.list.phase = NULL;
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
    if (status)
    {
      discard_b55 (*b55_save);
      *b55_save = NULL;
    }
  }
  return status;
}

/* Saves every list or none. */
static int
save_lists (evalresp_logger *log, evalresp_blkt **lists, int nlists,
            evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK, i;
  for (i = 0; !status && i < nlists; ++i)
  {
    status = save_b55 (log, lists[i], &b55_save[i]);
  }
  if (status)
  {
    for (i = 0; i < nlists; ++i)
    {
      discard_b55 (b55_save[i]);
      b55_save[i] = NULL;
    }
  }
  return status;
}

static int
restrict_frequency_range (evalresp_logger *log, double lo, double hi, int *nfreqs, double *freqs)
{
  int status = EVALRESP_OK, nbefore = 0, nafter = 0, i;
  /* values within rounding error of a limit are inside the range */
  lo -= 1e-9 * fabs (lo);
  hi += 1e-9 * fabs (hi);
  while (*nfreqs && freqs[0] < lo)
  {
    for (i = 1; i < *nfreqs; ++i)
      freqs[i - 1] = freqs[i];
    nbefore++;
    (*nfreqs)--;
  }
  while (*nfreqs && freqs[*nfreqs - 1] > hi)
  {
    nafter++;
    (*nfreqs)--;
  }
  if (*nfreqs)
  {
    if (nbefore)
    {
      evalresp_log (log, EV_WARN, EV_WARN,
                    "Dropped %d values from start of frequency range because they are not included in blockette 55",
                    nbefore);
    }
    if (nafter)
    {
      evalresp_log (log, EV_WARN, EV_WARN,
                    "Dropped %d values from end of frequency range because they are not included in blockette 55",
                    nafter);
    }
  }
  else
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "No blockette 55 values within requested frequency range");
    status = EVALRESP_INP;
  }
  return status;
}

/* Sequence number of the stage holding the blockette. */
static int
stage_of_blkt (const evalresp_channel *channel, const evalresp_blkt *blkt)
{
  const evalresp_stage *stage;
  const evalresp_blkt *b;

  for (stage = channel->first_stage; stage; stage = stage->next_stage)
  {
    for (b = stage->first_blkt; b; b = b->next_blkt)
    {
      if (b == blkt)
        return stage->sequence_no;
    }
  }
  return -1;
}

/* Interpolation needs at least two rows at strictly increasing frequencies. */
static int
check_list_for_interpolation (evalresp_logger *log, const evalresp_channel *channel,
                              const evalresp_blkt *blkt)
{
  const evalresp_list *list = &blkt->blkt_info.list;
  int i;

  if (list->nresp < 2 || !list->freq || !list->amp || !list->phase)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "Stage %d is a response list with fewer than two rows, so it cannot be interpolated",
                  stage_of_blkt (channel, blkt));
    return EVALRESP_INP;
  }
  for (i = 1; i < list->nresp; ++i)
  {
    if (!(list->freq[i] > list->freq[i - 1]))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR,
                    "Stage %d is a response list whose frequencies do not strictly increase, so it cannot be interpolated",
                    stage_of_blkt (channel, blkt));
      return EVALRESP_INP;
    }
  }
  return EVALRESP_OK;
}

/* The frequencies are restricted to the range common to all the lists. */
static int
interpolate_b55 (evalresp_logger *log, const evalresp_channel *channel,
                 evalresp_blkt **lists, int nlists,
                 evalresp_response *response, evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK, i;

  for (i = 1; i < response->nfreqs; ++i)
  {
    if (response->freqs[i] < response->freqs[i - 1])
    {
      evalresp_log (log, EV_ERROR, EV_ERROR,
                    "Frequencies must be in ascending order to interpolate blockette 55");
      return EVALRESP_INP;
    }
  }
  for (i = 0; i < nlists; ++i)
  {
    if ((status = check_list_for_interpolation (log, channel, lists[i])))
      return status;
  }
  if (!(status = save_lists (log, lists, nlists, b55_save)))
  {
    for (i = 0; !status && i < nlists; ++i)
    {
      evalresp_list *list = &lists[i]->blkt_info.list;
      status = restrict_frequency_range (log, list->freq[0], list->freq[list->nresp - 1],
                                         &response->nfreqs, response->freqs);
    }
    for (i = 0; !status && i < nlists; ++i)
    {
      evalresp_list *list = &lists[i]->blkt_info.list;
      status = interpolate_list_blockette (&list->freq, &list->amp, &list->phase, &list->nresp,
                                           response->freqs, response->nfreqs, log);
    }
  }
  return status;
}

/* All the lists must be sampled at the same frequencies. */
static int
use_b55_freqs (evalresp_logger *log, const evalresp_channel *channel,
               evalresp_blkt **lists, int nlists, evalresp_response *response)
{
  int status = EVALRESP_OK, i;
  evalresp_list *list = &lists[0]->blkt_info.list;

  for (i = 1; i < nlists; ++i)
  {
    if (!list_matches_freqs (&lists[i]->blkt_info.list, list->freq, list->nresp))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR,
                    "Response list stages %d and %d have different frequencies, so they can only be combined by interpolating",
                    stage_of_blkt (channel, lists[0]), stage_of_blkt (channel, lists[i]));
      return EVALRESP_INP;
    }
  }
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
restore_b55 (evalresp_blkt *b55, evalresp_blkt *b55_save)
{
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

/* A NULL freqs selects the grid described by options. */
static int
channel_to_response (evalresp_logger *log, evalresp_channel *channel,
                     evalresp_options *options, const double *freqs, int nfreqs,
                     evalresp_response **response)
{
  int status = EVALRESP_OK, free_options = 0, nlists = 0, i;
  evalresp_blkt **lists = NULL, **b55_save = NULL;
  evalresp_log_context ctx;

  evalresp_log_context_init (&ctx, log, "%s.%s.%s.%s", channel->network,
                             channel->staname, channel->locid, channel->chaname);
  log = &ctx.logger;

  /* allow NULL options */
  if (!options)
  {
    status = evalresp_new_options (log, &options);
    free_options = 1;
  }

  if (!(status = local_alloc_response (log, response)))
  {
    if (!(status = freqs ? copy_requested_freqs (log, freqs, nfreqs, *response)
                         : calculate_default_freqs (log, options, *response)))
    {
      if (!(status = selected_lists (log, options, channel, &lists, &nlists)) && nlists)
      {
        /* a b55 block is copied to the response row by row, so the
         * frequencies must agree beforehand: either by interpolating the
         * blockette data (in which case we keep a backup and restore it
         * below) or by changing the output frequencies to match.
         */
        if (options->b55_interpolate)
        {
          if (!(b55_save = calloc (nlists, sizeof (*b55_save))))
          {
            evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate blockette 55 backups");
            status = EVALRESP_MEM;
          }
          else
          {
            status = interpolate_b55 (log, channel, lists, nlists, *response, b55_save);
          }
        }
        else
        {
          status = use_b55_freqs (log, channel, lists, nlists, *response);
        }
      }
    }
  }

  if (!status)
  {
    if (!(status = normalize_response (log, options, channel)))
    {
      if (!(status = calculate_response (log, options, channel, (*response)->freqs, (*response)->nfreqs, (*response)->rvec)))
      {
        strncpy ((*response)->network, channel->network, NETLEN);
        strncpy ((*response)->station, channel->staname, STALEN);
        strncpy ((*response)->locid, channel->locid, LOCIDLEN);
        strncpy ((*response)->channel, channel->chaname, CHALEN);
        if (options->verbose)
        {
          evalresp_channel_to_log (log, options, channel);
        }
      }
    }
  }

  for (i = 0; b55_save && i < nlists; ++i)
  {
    if (b55_save[i])
    {
      restore_b55 (lists[i], b55_save[i]);
      free (b55_save[i]);
    }
  }
  if (free_options)
  {
    evalresp_free_options (&options);
  }
  free (b55_save);
  free (lists);

  if (status && *response)
  {
    evalresp_free_response (response);
  }

  return status;
}

int
evalresp_channel_to_response (evalresp_logger *log, evalresp_channel *channel,
                              evalresp_options *options, evalresp_response **response)
{
  return channel_to_response (log, channel, options, NULL, 0, response);
}

int
evalresp_channel_to_response_freqs (evalresp_logger *log, evalresp_channel *channel,
                                    evalresp_options *options, const double *freqs,
                                    int nfreqs, evalresp_response **response)
{
  if (!freqs)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "No frequencies given");
    return EVALRESP_INP;
  }
  return channel_to_response (log, channel, options, freqs, nfreqs, response);
}

int
evalresp_channels_to_responses (evalresp_logger *log, evalresp_channels *channels,
                                evalresp_options *options, evalresp_responses **responses)
{
  return channels_to_responses (log, channels, options, NULL, 0, responses);
}

int
channels_to_responses (evalresp_logger *log, evalresp_channels *channels,
                       evalresp_options *options, const double *freqs, int nfreqs,
                       evalresp_responses **responses)
{
  int status = EVALRESP_OK, i;
  if (!*responses && !(*responses = calloc (1, sizeof (**responses))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate responses");
    status = EVALRESP_MEM;
  }
  else
  {
    for (i = 0; !status && i < channels->nchannels; ++i)
    {
      evalresp_response *response = NULL;
      if (!(status = channel_to_response (log, channels->channels[i], options, freqs, nfreqs, &response)))
      {
        (*responses)->nresponses++;
        if (!((*responses)->responses = realloc ((*responses)->responses, (*responses)->nresponses * sizeof (*(*responses)->responses))))
        {
          evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate array for new response");
          status = EVALRESP_MEM;
        }
        else
        {
          (*responses)->responses[(*responses)->nresponses - 1] = response;
        }
      }
    }
  }
  return status;
}
