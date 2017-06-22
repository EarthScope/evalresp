
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code giving a high level interface.

static char *prefixes[] = {"FAP", "AMP", "PHASE", "SPECTRA"};

#define FILENAME_TEMPLATE "%s.%s.%s.%s.%s"

static int
print_file (evalresp_log_t *log, evalresp_file_format format,
            int use_stdio, const evalresp_response *response)
{
  int status = EVALRESP_OK, length;
  char *filename = NULL, *prefix = prefixes[format];
  length = snprintf (filename, 0, FILENAME_TEMPLATE, prefix,
                     response->network, response->station, response->locid, response->channel);
  if (!(filename = calloc (length, sizeof (*filename))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate filename");
    status = EVALRESP_MEM;
  }
  else
  {
    (void)snprintf (filename, length, FILENAME_TEMPLATE, prefix,
                    response->network, response->station, response->locid, response->channel);
    if (use_stdio)
    {
      fprintf (stdout, "--------------------------------------------------\n");
      fprintf (stdout, "%s\n", filename);
      fprintf (stdout, "--------------------------------------------------\n");
      status = evalresp_response_to_stream (log, response, format, stdout);
    }
    else
    {
      status = evalresp_response_to_file (log, response, format, filename);
    }
  }
  return status;
}

int
evalresp_responses_to_cwd (evalresp_log_t *log, const evalresp_responses *responses,
                           evalresp_output_format format, int use_stdio)
{
  int status = EVALRESP_OK, i;

  for (i = 0; !status && i < responses->nresponses; ++i)
  {
    switch (format)
    {
    case evalresp_fap_output_format:
      status = print_file (log, evalresp_fap_file_format, use_stdio, responses->responses[i]);
      break;
    case evalresp_ap_output_format:
      if (!(status = print_file (log, evalresp_amplitude_file_format, use_stdio, responses->responses[i])))
      {
        status = print_file (log, evalresp_phase_file_format, use_stdio, responses->responses[i]);
      }
      break;
    case evalresp_complex_output_format:
      status = print_file (log, evalresp_complex_file_format, use_stdio, responses->responses[i]);
      break;
    }
  }

  return status;
}

static int
process_stdio (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter)
{
  int status = EVALRESP_OK;
  evalresp_channels *channels;
  evalresp_responses *responses;

  if (options->filename && strlen (options->filename))
  {
    evalresp_log (log, EV_WARN, EV_WARN, "Using stdio so ignoring file '%s'", options->filename);
  }
  if (!(status = evalresp_file_to_channels (log, stdin, filter, &channels)))
  {
    if (!(status = evalresp_channels_to_responses (log, channels, options, &responses)))
    {
      status = evalresp_responses_to_cwd (log, responses, options->format, options->use_stdio);
    }
  }

  return status;
}

//static int
//cwd_to_files (evalresp_log_t *log)
//{
//  return 0;
//}

static int
process_cwd (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter)
{
  int status = EVALRESP_OK;
  // get files
  // for each file, read channels
  // write output
  return status;
}

int
evalresp_cwd_to_cwd (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter)
{
  return (options->use_stdio) ? process_stdio (log, options, filter) : process_cwd (log, options, filter);
}
