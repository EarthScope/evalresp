
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code giving a high level interface.

static char *prefixes[] = {"FAP", "AMP", "PHASE", "SPECTRA"};

#define LABEL_TEMPLATE "%s.%s.%s.%s.%s"

static int
print_file (evalresp_log_t *log, evalresp_file_format format,
            int use_stdio, const evalresp_response *response)
{
  int status = EVALRESP_OK, length;
  char *filename = NULL, *prefix = prefixes[format];
  length = snprintf (filename, 0, LABEL_TEMPLATE, prefix,
                     response->network, response->station, response->locid, response->channel);
  if (!(filename = calloc (length, sizeof (*filename))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate filename");
    status = EVALRESP_MEM;
  }
  else
  {
    (void)snprintf (filename, length, LABEL_TEMPLATE, prefix,
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
