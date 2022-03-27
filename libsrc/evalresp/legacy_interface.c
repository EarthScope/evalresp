#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "evalresp/private.h"
#include "evalresp/public.h"
#include "evalresp/public_api.h"
#include <evalresp_log/log.h>
#include <stdlib.h>
#include <string.h>

/* define a global flag to use if using "default" units */
int def_units_flag;
/* define global variables for use in printing error messages */
char *curr_file;

int
evresp_1 (char *sta, char *cha, char *net, char *locid, char *datime,
          char *units, char *file, double *freqs, int nfreqs, double *resp,
          char *rtype, char *verbose, int start_stage, int stop_stage,
          int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
          int xml_flag)
{
  evalresp_response *first = (evalresp_response *)NULL;
  int i, j;

  // some eyeball checks to make sure fortran is passing things ok
  // printf("freqs: %f-%f\n", freqs[0], freqs[nfreqs-1]);
  // printf("x_for_b62: %f\n", x_for_b62);

  first = evresp (sta, cha, net, locid, datime, units, file, freqs, nfreqs,
                  rtype, verbose, start_stage, stop_stage, stdio_flag, useTotalSensitivityFlag,
                  x_for_b62, xml_flag);

  /* check the output.  If no response found, return 1, else if more than one response
     found, return -1 */

  if (first == (evalresp_response *)NULL)
  {
    return (1);
  }
  else if (first->next != (evalresp_response *)NULL)
  {
    evalresp_free_response (&first);
    return (-1);
  }

  /* if only one response found, convert from complex output vector into multiplexed
     real output for FORTRAN (real1, imag1, real2, imag2, ..., realN, imagN) */

  for (i = 0, j = 0; i < nfreqs; i++)
  {
    resp[j++] = (float)first->rvec[i].real;
    resp[j++] = (float)first->rvec[i].imag;
  }

  /* free up dynamically allocated space */

  evalresp_free_response (&first);

  /* and return to FORTRAN program */

  return (0);
}

/* 2/6/2006 -- [ET]  Moved from 'evalresp.c' to 'evresp.c' */
/* 12-JUL-2017 -- [Dylan]  Moved from 'eivresp.c' to 'legacy_interface.cc' */
int
use_estimated_delay (int flag)
{
  /* WE USE THOSE WEIRD magic numbers here because
     * there is a chance that use_delay_flag is not
     * defined: in user program which uses evresp()
     * when use_estimated_delay() is not used before evresp().
     */
  int magic_use_delay = 35443647;
  int magic_dont_use_delay = -90934324;
  static int use_delay_flag = FALSE;
  if (TRUE == flag)
    use_delay_flag = magic_use_delay;
  if (FALSE == flag)
    use_delay_flag = magic_dont_use_delay;

  if (use_delay_flag == magic_use_delay)
    return TRUE;
  return FALSE;
}

static int
convert_responses_to_response_chain (evalresp_responses *responses, evalresp_response **first_resp)
{
  int i;
  int nresp;

  *first_resp = NULL;
  if(!responses) {
      return -1;
  }
  nresp = responses->nresponses;
  if(nresp == 0 || !responses->responses) {
      return -1;
  }
  for (i = 1; i < nresp; i++)
  {
    responses->responses[i - 1]->next = responses->responses[i];
  }
  responses->responses[i - 1]->next = NULL;
  *first_resp = responses->responses[0];
  return EVALRESP_OK;
}

static int
determine_log_or_lin (int num_freq, double *freqs)
{
  double dt1 = freqs[1] - freqs[0];
  double dt2 = freqs[2] - freqs[1];
  double diff = dt1 - dt2;

  if (diff < 1e-8 && diff > -1e-8)
  {
    return 1; /*linear */
  }
  return 0; /*not linear step */
}

/* old main call */
evalresp_response *
evresp_itp (char *stalst, char *chalst, char *net_code,
            char *locidlst, char *date_time, char *units, char *file, double *freqs,
            int nfreqs, char *rtype, char *verbose, int start_stage, int stop_stage,
            int stdio_flag, int listinterp_out_flag, int listinterp_in_flag,
            double listinterp_tension, int useTotalSensitivityFlag,
            double x_for_b62, int xml_flag)
{
  evalresp_logger *log = NULL;
  evalresp_options *options = NULL;
  evalresp_filter *filter = NULL;
  evalresp_responses *responses = NULL;
  evalresp_response *first_resp = NULL;
  int i, year, jday;
  char time[100];

  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
    return NULL;
  }
  options->filename = (file) ? strdup (file) : NULL;
  options->b62_x = x_for_b62;
  options->nfreq = nfreqs;
  options->min_freq = freqs[0];
  options->max_freq = freqs[nfreqs - 1];
  options->lin_freq = determine_log_or_lin (nfreqs, freqs);
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay (QUERY_DELAY) == TRUE ? 1 : 0;
  options->unwrap_phase = 0; /*TODO: Should this be a getter function? */
  options->b55_interpolate = listinterp_out_flag | listinterp_in_flag;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  options->use_stdio = stdio_flag;
  options->station_xml = xml_flag;

  if (rtype)
  {
    if (EVALRESP_OK != evalresp_set_format (log, options, rtype))
    {
      evalresp_free_options (&options);
      return NULL;
    }
  }
  if (units)
  {
    if (EVALRESP_OK != evalresp_set_unit (log, options, units))
    {
      evalresp_free_options (&options);
      return NULL;
    }
  }
  else if (def_units_flag)
  {
    options->unit = evalresp_file_unit;
  }

  if (verbose)
  {
    for (i = 0; i < strlen (verbose); i++)
    {
      if (toupper (verbose[i]) == 'V')
      {
        options->verbose++;
      }
    }
  }

  /*SETUP Filter */
  if (EVALRESP_OK != evalresp_new_filter (log, &filter))
  {
    evalresp_free_options (&options);
    return NULL;
  }

  if (3 > sscanf (date_time, "%d,%d,%s", &year, &jday, time))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Improper date time passed");
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  filter->datetime->year = year;
  filter->datetime->jday = jday;
  if (EVALRESP_OK != evalresp_set_time (log, filter, time))
  {
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  if (EVALRESP_OK != evalresp_add_sncl_text (log, filter, net_code, stalst, locidlst, chalst))
  {
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  /*Call to new evresp */
  if (options->use_stdio)
  {
    process_stdio (log, options, filter, &responses);
  }
  else
  {
    process_cwd (log, options, filter, &responses);
  }

  if (responses)
  {
    convert_responses_to_response_chain (responses, &first_resp);
  }

  evalresp_free_options (&options);
  evalresp_free_filter (&filter);
  free (responses); /* we want to keep the allocated responses but not the wrapper struct */
  return first_resp;
}

/* wrapped main call */
evalresp_response *
evresp (char *stalst, char *chalst, char *net_code,
        char *locidlst, char *date_time, char *units, char *file, double *freqs,
        int nfreqs, char *rtype, char *verbose, int start_stage, int stop_stage,
        int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
        int xml_flag)
{
  return evresp_itp (stalst, chalst, net_code, locidlst, date_time, units,
                     file, freqs, nfreqs, rtype, verbose, start_stage, stop_stage,
                     stdio_flag, 0, 0, 0.0, 0, x_for_b62, xml_flag);
}

void
print_chan (evalresp_channel *chan, int start_stage, int stop_stage,
            int stdio_flag, int listinterp_out_flag, int listinterp_in_flag,
            int useTotalSensitivityFlag)
{
  evalresp_logger *log = NULL;
  evalresp_options *options = NULL;
  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
    return;
  }
  options->filename = strdup (curr_file); /*TODO this needs to be no longer global */
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay (QUERY_DELAY) == TRUE ? 1 : 0;
  options->b55_interpolate = listinterp_out_flag | listinterp_in_flag;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  options->use_stdio = stdio_flag;

  evalresp_channel_to_log (log, options, chan);
  evalresp_free_options (&options);
  return;
}

void
print_resp_itp (double *freqs, int nfreqs, evalresp_response *first,
                char *rtype, int stdio_flag, int listinterp_out_flag,
                double listinterp_tension, int unwrap_flag)
{
  evalresp_logger *log = NULL;
  evalresp_responses responses[1];
  evalresp_response *ptr;
  evalresp_options *options = NULL;
  int i;

  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
    return;
  }
  options->nfreq = nfreqs;
  options->min_freq = freqs[0];
  options->max_freq = freqs[nfreqs - 1];
  options->unwrap_phase = unwrap_flag;
  options->b55_interpolate = listinterp_out_flag | (listinterp_tension != 0);

  if (rtype)
  {
    if (EVALRESP_OK != evalresp_set_format (log, options, rtype))
    {
      evalresp_free_options (&options);
      return;
    }
  }

  for (responses->nresponses = 0, ptr = first; ptr != NULL; responses->nresponses++, ptr = ptr->next)
    ;

  responses->responses = (evalresp_response **)calloc (responses->nresponses, sizeof (evalresp_response *));
  for (i = 0, ptr = first; ptr != NULL; i++, ptr = ptr->next)
  {
    responses->responses[i] = ptr;
  }

  responses_to_cwd (log, responses, options->unwrap_phase, options->format, stdio_flag);
  free (responses->responses);
}

void
print_resp (double *freqs, int nfreqs, evalresp_response *first, char *rtype,
            int stdio_flag)
{
  print_resp_itp (freqs, nfreqs, first, rtype, stdio_flag, 0, 0.0, 0);
}

void
calc_resp (evalresp_channel *chan, double *freq, int nfreqs,
           evalresp_complex *output, const char *out_units, int start_stage,
           int stop_stage, int useTotalSensitivityFlag, double x_for_b62)
{
  evalresp_logger *log = NULL;
  evalresp_options *options = NULL;
  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
    return;
  }
  options->b62_x = x_for_b62;
  options->nfreq = nfreqs;
  options->min_freq = freq[0];
  options->max_freq = freq[nfreqs - 1];
  options->lin_freq = determine_log_or_lin (nfreqs, freq);
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay (QUERY_DELAY) == TRUE ? 1 : 0;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  if (out_units)
  {
    if (EVALRESP_OK != evalresp_set_unit (log, options, out_units))
    {
      evalresp_free_options (&options);
      return;
    }
  }
  else if (def_units_flag)
  {
    options->unit = evalresp_file_unit;
  }

  calculate_response (log, options, chan, freq, nfreqs, output);

  evalresp_free_options (&options);
}
void
norm_resp (evalresp_channel *chan, int start_stage, int stop_stage)
{
  evalresp_logger *log = NULL;
  evalresp_options *options = NULL;
  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
    return;
  }
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;

  normalize_response (log, options, chan);

  evalresp_free_options (&options);
}
