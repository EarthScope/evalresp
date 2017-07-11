#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <evalresp_log/log.h>
#include "evalresp/public_api.h"
#include "evalresp/public.h"
#include "evalresp/private.h"

/* Fortran  interface */

static int convert_responses_to_response_chain( evalresp_responses *responses, evalresp_response **first_resp)
{
  int i;
  int nresp;

  nresp = responses->nresponses;
  
  for (i = 1; i < nresp; i++)
  {
      responses->responses[i-1]->next = responses->responses[i];
  }
  responses->responses[i-1]->next = NULL;
  *first_resp = responses->responses[0];
  return EVALRESP_OK;
}

static int determine_log_or_lin(int num_freq, double *freqs)
{
    double dt1 = freqs[1] - freqs[0];
    double dt2 = freqs[2] - freqs[1];
    double diff = dt1-dt2;

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
            double x_for_b62, int xml_flag, evalresp_log_t *log)
{
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
  options->filename = strdup(file);
  options->b62_x = x_for_b62;
  options->nfreq = nfreqs;
  options->min_freq = freqs[0];
  options->max_freq = freqs[nfreqs - 1];
  options->lin_freq = determine_log_or_lin(nfreqs, freqs);
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay(QUERY_DELAY) == TRUE ? 1 : 0;
  options->unwrap_phase = 0;/*TODO: Should this be a getter function? */
  options->b55_interpolate = listinterp_out_flag | listinterp_in_flag;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  options->use_stdio=stdio_flag;
  options->station_xml = xml_flag;

  if (rtype)
  {
    if (EVALRESP_OK != evalresp_set_format (log, options, rtype))
    {
      evalresp_free_options (&options);
      return NULL;
    }
    options->format_set = 1;

  }
  if (units)
  {
    if (EVALRESP_OK != evalresp_set_unit (log, options, units))
    {
      evalresp_free_options (&options);
      return NULL;
    }
    options->unit_set=1;
  }

  if(verbose)
  {
    for (i = 0; i < strlen(verbose); i++)
    {
      if (toupper(verbose[i]) == 'V')
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

  if ( 3 > sscanf(date_time, "%d,%d,%s", &year, &jday, time))
  {
    evalresp_log(log, EV_ERROR, EV_ERROR, "Improper date time passed");
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

  /*TODO covert responses to response linked list */
  convert_responses_to_response_chain(responses, &first_resp);

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
        int xml_flag, evalresp_log_t *log)
{
  return evresp_itp (stalst, chalst, net_code, locidlst, date_time, units,
                     file, freqs, nfreqs, rtype, verbose, start_stage, stop_stage,
                     stdio_flag, 0, 0, 0.0, 0, x_for_b62, xml_flag, log);
}

void
print_chan (evalresp_channel *chan, int start_stage, int stop_stage,
            int stdio_flag, int listinterp_out_flag, int listinterp_in_flag,
            int useTotalSensitivityFlag, evalresp_log_t *log)
{
  evalresp_options *options = NULL;
  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
      return;
  }
  options->filename = strdup(curr_file);/*TODO this needs to be no longer global */
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay(QUERY_DELAY) == TRUE ? 1 : 0;
  options->b55_interpolate = listinterp_out_flag | listinterp_in_flag;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  options->use_stdio=stdio_flag;

  evalresp_channel_to_log(log, options, chan);
  evalresp_free_options (&options);
  return;
}

void
print_resp_itp (double *freqs, int nfreqs, evalresp_response *first,
                char *rtype, int stdio_flag, int listinterp_out_flag,
                double listinterp_tension, int unwrap_flag, evalresp_log_t *log)
{
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
    options->format_set = 1;

  }

  for(responses->nresponses = 0, ptr = first; ptr != NULL; responses->nresponses++, ptr = ptr->next);

  responses->responses = (evalresp_response **)calloc(responses->nresponses, sizeof(evalresp_response *));
  for(i = 0, ptr = first; ptr != NULL; i++, ptr = ptr->next)
  {
    responses->responses[i] =ptr;
  }

  evalresp_responses_to_cwd (log, responses, options->format, stdio_flag);
  free(responses->responses);
}

void
print_resp (double *freqs, int nfreqs, evalresp_response *first, char *rtype,
            int stdio_flag, evalresp_log_t *log)
{
  print_resp_itp (freqs, nfreqs, first, rtype, stdio_flag, 0, 0.0, 0, log);
}
