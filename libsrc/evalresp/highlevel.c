
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code giving a high level interface.

static char *prefixes[] = {"FAP", "AMP", "PHASE", "SPECTRA", "AMP/PHS"};

#define FILENAME_TEMPLATE "%s.%s.%s.%s.%s"

static int
print_file (evalresp_log_t *log, evalresp_file_format format,
            int use_stdio, const evalresp_response *response)
{
  int status = EVALRESP_OK, length;
  char *filename = NULL, *prefix = prefixes[use_stdio && evalresp_fap_output_format ? 4 : format];
  length = snprintf (filename, 0, FILENAME_TEMPLATE, prefix,
                     response->network, response->station, response->locid, response->channel);
  if (!(filename = calloc (length + 1, sizeof (*filename))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate filename");
    status = EVALRESP_MEM;
  }
  else
  {
    (void)snprintf (filename, length + 1, FILENAME_TEMPLATE, prefix,
                    response->network, response->station, response->locid, response->channel);
    if (use_stdio)
    {
      fprintf (stdout, " --------------------------------------------------\n");
      fprintf (stdout, " %s\n", filename);
      fprintf (stdout, " --------------------------------------------------\n");
      status = evalresp_response_to_stream (log, response, format, stdout);
      fprintf (stdout, " --------------------------------------------------\n");
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
      evalresp_free_responses (responses);
    }
    evalresp_free_channels (&channels);
  }

  return status;
}

// process a single named file
static int
process_file (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter, const char *filename)
{
  int status = EVALRESP_OK;
  evalresp_channels *channels = NULL;
  evalresp_responses *responses = NULL;

  if (!(status = evalresp_filename_to_channels (log, filename, filter, &channels)))
  {
    if (!(status = evalresp_channels_to_responses (log, channels, options, &responses)))
    {
      status = evalresp_responses_to_cwd (log, responses, options->format, options->use_stdio);
      evalresp_free_responses (responses);
    }
    evalresp_free_channels (&channels);
  }
  return status;
}

static int
process_cwd_files (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter, struct matched_files *files)
{
  int status = EVALRESP_OK, i;
  struct matched_files *files_for_sncls;
  struct file_list *file;

  for (i = 0, files_for_sncls = files;
       !status && i < filter->sncls->nscn;
       ++i, files_for_sncls = files_for_sncls->ptr_next)
  {
    // if i understand correctly, then we need to use a separate
    // filter for each sncl
    evalresp_sncl *sncl = filter->sncls->scn_vec[i];
    evalresp_filter *sncl_filter = NULL;
    if (!(status = evalresp_new_filter (log, &sncl_filter)))
    {
      sncl_filter->datetime = filter->datetime; // shared
      if (!(status = evalresp_add_sncl (log, sncl_filter, sncl)))
      {
        file = files->first_list;
        while (!status && files)
        {
          status = process_file (log, options, filter, file->name);
          file = file->next_file;
        }
      }
      sncl_filter->datetime = NULL; // don't free this as shared
      evalresp_free_filter (&sncl_filter);
    }
  }
  return status;
}

static int
process_cwd (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter)
{
  int status = EVALRESP_OK, mode;
  struct matched_files *files = NULL;

  files = find_files (options->filename, filter->sncls, &mode, log);
  switch (mode)
  {
  case 0:
    status = process_file (log, options, filter, options->filename);
    break;
  default:
    // TODO - do we need to handle other modes?
    status = process_cwd_files (log, options, filter, files);
    break;
  }
  free_matched_files (files);
  return status;
}

int
evalresp_cwd_to_cwd (evalresp_log_t *log, evalresp_options *options, evalresp_filter *filter)
{
  return (options->use_stdio) ? process_stdio (log, options, filter) : process_cwd (log, options, filter);
}
