#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "evalresp/constants.h"
#include "evalresp/public.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// new code giving a high level interface.

static char *prefixes[] = {"FAP", "AMP", "PHASE", "SPECTRA", "AMP/PHS"};

#define FILENAME_TEMPLATE "%s.%s.%s.%s.%s"

/* IGD This function is needed for MS Windows VS2013 and below */

int                                     /* O - Number of bytes formatted */
_evalresp_snprintf (char *buffer,       /* I - Output buffer */
                    size_t bufsize,     /* I - Size of output buffer */
                    const char *format, /* I - Printf-style format string */
                    ...)                /* I - Additional arguments as needed */
{
  va_list ap; /* Argument list */
  int bytes;  /* Number of bytes formatted */

  va_start (ap, format);
  bytes = vsnprintf (buffer, bufsize, format, ap);
  va_end (ap);

  return (bytes);
}

static int
print_file (evalresp_logger *log, int unwrap, evalresp_file_format format,
            int use_stdio, const evalresp_response *response)
{
  int status = EVALRESP_OK, length;
  char *filename = NULL, *prefix = prefixes[(use_stdio && (format == evalresp_fap_file_format)) ? 4 : format];
  length = _evalresp_snprintf (filename, 0, FILENAME_TEMPLATE, prefix,
                               response->network, response->station, response->locid, response->channel);
  if (!(filename = calloc (length + 1, sizeof (*filename))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate filename");
    status = EVALRESP_MEM;
  }
  else
  {
    (void)_evalresp_snprintf (filename, length + 1, FILENAME_TEMPLATE, prefix,
                              response->network, response->station, response->locid, response->channel);
    if (use_stdio)
    {
      fprintf (stdout, " --------------------------------------------------\n");
      fprintf (stdout, " %s\n", filename);
      fprintf (stdout, " --------------------------------------------------\n");
      status = evalresp_response_to_stream (log, response, unwrap, format, stdout);
      fprintf (stdout, " --------------------------------------------------\n");
    }
    else
    {
      status = evalresp_response_to_file (log, response, unwrap, format, filename);
    }
  }
  free (filename);
  filename = NULL;
  return status;
}

int
responses_to_cwd (evalresp_logger *log, const evalresp_responses *responses,
                  int unwrap, evalresp_output_format format, int use_stdio)
{
  int status = EVALRESP_OK, i;

  for (i = 0; !status && i < responses->nresponses; ++i)
  {
    switch (format)
    {
    case evalresp_fap_output_format:
      status = print_file (log, unwrap, evalresp_fap_file_format, use_stdio, responses->responses[i]);
      break;
    case evalresp_ap_output_format:
      if (!(status = print_file (log, unwrap, evalresp_amplitude_file_format, use_stdio, responses->responses[i])))
      {
        status = print_file (log, unwrap, evalresp_phase_file_format, use_stdio, responses->responses[i]);
      }
      break;
    case evalresp_complex_output_format:
      status = print_file (log, unwrap, evalresp_complex_file_format, use_stdio, responses->responses[i]);
      break;
    }
  }

  return status;
}

int
process_stdio (evalresp_logger *log, evalresp_options *options, evalresp_filter *filter, evalresp_responses **responses)
{
  int status = EVALRESP_OK;
  evalresp_channels *channels;

  if (options->filename && strlen (options->filename))
  {
    evalresp_log (log, EV_WARN, EV_WARN, "Using stdio so ignoring file '%s'", options->filename);
  }
  if (!(status = evalresp_file_to_channels (log, stdin, options, filter, &channels)))
  {
    status = evalresp_channels_to_responses (log, channels, options, responses);
  }
  evalresp_free_channels (&channels);

  return status;
}

// process a single named file
static int
process_file (evalresp_logger *log, evalresp_options *options, evalresp_filter *filter, const char *filename, evalresp_responses **responses)
{
  int status = EVALRESP_OK;
  evalresp_channels *channels = NULL;

  if (!(status = evalresp_filename_to_channels (log, filename, options, filter, &channels)))
  {
    status = evalresp_channels_to_responses (log, channels, options, responses);
  }
  evalresp_free_channels (&channels);
  return status;
}

static int
add_sncl (evalresp_logger *log, evalresp_filter *filter, evalresp_sncl *sncl)
{
  return evalresp_add_sncl_text (log, filter, sncl->network, sncl->station, sncl->locid, sncl->channel);
}

static int
process_cwd_files (evalresp_logger *log, evalresp_options *options, evalresp_filter *filter, struct matched_files *files, evalresp_responses **responses)
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
      if (!(status = add_sncl (log, sncl_filter, sncl)))
      {
        file = files->first_list;
        while (!status && file)
        {
          status = process_file (log, options, filter, file->name, responses);
          file = file->next_file;
        }
      }
      sncl_filter->datetime = NULL; // don't free this as shared
      evalresp_free_filter (&sncl_filter);
    }
  }
  return status;
}

int
process_cwd (evalresp_logger *log, evalresp_options *options,
             evalresp_filter *filter, evalresp_responses **responses)
{
  int status = EVALRESP_OK, mode;
  struct matched_files *files = NULL;

  files = find_files (options->filename, filter->sncls, &mode, log);
  switch (mode)
  {
  case 0:
    status = process_file (log, options, filter, options->filename, responses);
    break;
  default:
    // TODO - do we need to handle other modes?
    status = process_cwd_files (log, options, filter, files, responses);
    break;
  }
  free_matched_files (files);
  return status;
}

int
evalresp_cwd_to_cwd (evalresp_logger *log, evalresp_options *options, evalresp_filter *filter)
{
  int status, free_options = 0;
  evalresp_responses *responses = NULL;

  /* allow NULL options */
  if (!options)
  {
    status = evalresp_new_options (log, &options);
    free_options = 1;
  }

  if (options->verbose)
  {
    evalresp_log (log, EV_INFO, 0, "<< EVALRESP RESPONSE OUTPUT V%s >>", REVNUM);
  }
  if (options->use_stdio)
  {
    status = process_stdio (log, options, filter, &responses);
  }
  else
  {
    status = process_cwd (log, options, filter, &responses);
  }

  if (!status)
  {
    /* Traditionally, FAP is always unwrapped. */
    status = responses_to_cwd (log, responses,
                               options->unwrap_phase || options->format == evalresp_fap_output_format,
                               options->format, options->use_stdio);
  }

  evalresp_free_responses (&responses);
  if (free_options)
  {
    evalresp_free_options (&options);
  }
  return status;
}
