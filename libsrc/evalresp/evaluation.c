
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
evalresp_set_filename (evalresp_log_t *log, evalresp_options *options, const char *filename)
{
  options->filename = strdup (filename);
  return EVALRESP_OK;
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
                               units, unit, &value)))
  {
    options->unit = value;
  }
  return status;
}

static option_pair spacings[] = {
    {0, "LOG"},
    {1, "LIN"}};

int
evalresp_set_spacing (evalresp_log_t *log, evalresp_options *options,
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
/*XXX not used
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
*/

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
restrict_frequency_range (evalresp_log_t *log, double lo, double hi, int *nfreqs, double *freqs)
{
  int status = EVALRESP_OK, nbefore = 0, nafter = 0, i;
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

static int
interpolate_b55 (evalresp_log_t *log, evalresp_channel *channel,
                 evalresp_response *response, evalresp_blkt **b55_save)
{
  int status = EVALRESP_OK;
  evalresp_blkt *b55 = channel->first_stage->first_blkt;
  evalresp_list *list = &b55->blkt_info.list;
  if (!(status = save_b55 (log, channel, b55_save)))
  {
    if (!(status = restrict_frequency_range (log, list->freq[0], list->freq[list->nresp - 1],
                                             &response->nfreqs, response->freqs)))
    {
      status = interpolate_list_blockette (&list->freq, &list->amp, &list->phase, &list->nresp,
                                           response->freqs, response->nfreqs, log);
    }
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
  if (status && *response)
  {
    evalresp_free_response (*response);
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

int
evalresp_channels_to_responses (evalresp_log_t *log, evalresp_channels *channels,
                                evalresp_options *options, evalresp_responses **responses)
{
  int status = EVALRESP_OK, i;
  if (NULL == *responses && !(*responses = calloc (1, sizeof (**responses))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate responses");
    status = EVALRESP_MEM;
  }
  else
  {
    for (i = 0; !status && i < channels->nchannels; ++i)
    {
      evalresp_response *response = NULL;
      if (!(status = evalresp_channel_to_response (log, channels->channels[i], options, &response)))
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

int
calculate_response (evalresp_log_t *log, evalresp_options *options,
                    evalresp_channel *chan, double *freq, int nfreqs,
                    evalresp_complex *output)
{
  evalresp_blkt *blkt_ptr;
  evalresp_stage *stage_ptr;
  int i, j, units_code, eval_flag = 0, nc = 0, sym_fir = 0;
  double w;
  int matching_stages = 0, has_stage0 = 0;
  evalresp_complex of, val;
  double corr_applied, calc_delay, estim_delay, delay;
  int status = EVALRESP_OK;

  /*  if(options->start_stage && options->start_stage > chan->nstages) {
     error_return(NO_STAGE_MATCHED, "calc_resp: %s options->start_stage=%d, highest stage found=%d)",
     "No Matching Stages Found (requested",start_stage, chan->nstages);
     } */

  /* for each frequency */

  for (i = 0; i < nfreqs; i++)
  {
    w = 2 * M_PI * freq[i];
    val.real = 1.0;
    val.imag = 0.0;

    /* loop through the stages and filters for each stage, calculating
         the response for each frequency for all stages */

    stage_ptr = chan->first_stage;
    units_code = stage_ptr->input_units;
    for (j = 0; j < chan->nstages; j++)
    {
      nc = 0;
      sym_fir = 0;
      if (!stage_ptr->sequence_no)
        has_stage0 = 1;
      if (options->start_stage >= 0 && options->stop_stage && (stage_ptr->sequence_no < options->start_stage || stage_ptr->sequence_no > options->stop_stage))
      {
        stage_ptr = stage_ptr->next_stage;
        continue;
      }
      else if (options->start_stage >= 0 && !options->stop_stage && stage_ptr->sequence_no != options->start_stage)
      {
        stage_ptr = stage_ptr->next_stage;
        continue;
      }
      matching_stages++;
      blkt_ptr = stage_ptr->first_blkt;
      while (blkt_ptr)
      {
        eval_flag = 0;
        switch (blkt_ptr->type)
        {
        case ANALOG_PZ:
        case LAPLACE_PZ:
          analog_trans (blkt_ptr, freq[i], &of);
          eval_flag = 1;
          break;
        case IIR_PZ:
          if (blkt_ptr->blkt_info.pole_zero.nzeros || blkt_ptr->blkt_info.pole_zero.npoles)
          {
            iir_pz_trans (blkt_ptr, w, &of);
            eval_flag = 1;
          }
          break;
        case FIR_SYM_1:
        case FIR_SYM_2:
          if (blkt_ptr->type == FIR_SYM_1)
            nc = (double)blkt_ptr->blkt_info.fir.ncoeffs * 2 - 1;
          else if (blkt_ptr->type == FIR_SYM_2)
            nc = (double)blkt_ptr->blkt_info.fir.ncoeffs * 2;
          if (blkt_ptr->blkt_info.fir.ncoeffs)
          {
            fir_sym_trans (blkt_ptr, w, &of);
            sym_fir = 1;
            eval_flag = 1;
          }
          break;
        case FIR_ASYM:
          nc = (double)blkt_ptr->blkt_info.fir.ncoeffs;
          if (blkt_ptr->blkt_info.fir.ncoeffs)
          {
            fir_asym_trans (blkt_ptr, w, &of);
            sym_fir = -1;
            eval_flag = 1;
          }
          break;
        case DECIMATION: /* IGD 10/05/13 Logic updated to include calc_delay on demand */
          if (blkt_ptr->type != IIR_PZ && nc != 0)
          {
            /* IGD 08/27/08 Use estimated delay instead of calculated */
            estim_delay =
                (double)blkt_ptr->blkt_info.decimation.estim_delay;
            corr_applied =
                blkt_ptr->blkt_info.decimation.applied_corr;
            calc_delay = ((nc - 1) / 2.0) * blkt_ptr->blkt_info.decimation.sample_int;
            /* Asymmetric FIR coefficients require a delay correction */
            if (sym_fir == -1)
            {
              if (options->use_estimated_delay)
              {
                delay = estim_delay;
              }
              else
              {
                delay = corr_applied - calc_delay;
              }
            }
            /* Otherwise delay has already been handled in fir_sym_trans() */
            else
            {
              delay = 0;
            }
            calc_time_shift (delay, w, &of);
            eval_flag = 1;
          }
          break;
        case LIST: /* This option is added in version 2.3.17 I.Dricker*/
          calc_list (blkt_ptr, i, &of); /*compute real and imag parts for the i-th ampl and phase */
          eval_flag = 1;
          break;
        case POLYNOMIAL: /* IGD 06/01/2013*/
          if ((status = calc_polynomial (blkt_ptr, &of, options->b62_x, log)))
          {
            return status;
          }
          eval_flag = 1;
          break;
        case IIR_COEFFS: /* This option is added in version 2.3.17 I.Dricker*/
          iir_trans (blkt_ptr, w, &of);
          eval_flag = 1;
          break;
        default:
          break;
        }
        if (eval_flag)
          zmul (&val, &of);
        blkt_ptr = blkt_ptr->next_blkt;
      }
      stage_ptr = stage_ptr->next_stage;
    }

    /* if no matching stages were found, then report the error */

    if (!matching_stages && !has_stage0)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "calc_resp: %s start_stage=%d, highest stage found=%d)",
                    "No Matching Stages Found (requested", options->start_stage,
                    chan->nstages);
      return NO_STAGE_MATCHED;
    }
    else if (!matching_stages)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "calc_resp: %s start_stage=%d, highest stage found=%d)",
                    "No Matching Stages Found (requested", options->start_stage,
                    chan->nstages - 1);
      return NO_STAGE_MATCHED;
    }

    /*  Write output for freq[i] in output[i] (note: unit_scale_fact is set by the
     * 'check_units' function that is used to convert to 'MKS' units when the
     * the response was given as a displacement, velocity, or acceleration in units other
     * than meters) */
    if (0 == options->use_total_sensitivity)
    {
      output[i].real = val.real * chan->calc_sensit * chan->unit_scale_fact;
      output[i].imag = val.imag * chan->calc_sensit * chan->unit_scale_fact;
    }
    else
    {
      output[i].real = val.real * chan->sensit * chan->unit_scale_fact;
      output[i].imag = val.imag * chan->sensit * chan->unit_scale_fact;
    }

    if ((status = convert_to_units (units_code, options->unit_set ? options->unit : evalresp_velocity_unit, &output[i], w, log)))
    {
      return status;
    }
  }
  return EVALRESP_OK;
}
