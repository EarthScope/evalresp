
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "vcs_getopt.h"
#endif

#include "../libsrc/evalresp/private.h"
#include "evalresp/public.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

void
usage (char *program)
{
  printf ("\nEVALRESP V%s\n", REVNUM);
  printf ("\nUSAGE: %s STALST CHALST YYYY DAY MINFREQ", program);
  printf (" MAXFREQ NFREQ [options]\n\n");
  printf ("  OPTIONS:\n\n");
  printf ("    -f file              (directory-name|filename)\n");
  printf ("    -u units             ('dis'|'vel'|'acc'|'def')\n");
  printf ("    -t time-of-day       (HH:MM:SS)\n");
  printf ("    -s type-of-spacing   (log|lin)\n");
  printf ("    -n netid             ('II'|'IU'|'G'|'*'...)\n");
  printf ("    -l locid             ('01'|'AA,AB,AC'|'A?'|'*'...)\n");
  printf ("    -r resp_type         ('ap'=amp/pha | 'cs'=complex spectra |\n");
  printf ("                          'fap'=freq/amp/pha)\n");
  printf ("    -stage start [stop]  (start and stop are integer stage numbers)\n");
  printf ("    -stdio               (take input from stdin, output to stdout)\n");
  printf ("    -use-estimated-delay (use estimated delay instead of correction applied\n");
  printf ("                          in computation of ASYM FIR response)\n");
  printf ("    -il                  (interpolate List blockette output)\n");
  printf ("    -ii                  (interpolate List blockette input)\n");
  printf ("    -unwrap              (unwrap phase if the output is AP) \n");
  printf ("    -ts                  (use total sensitivity from stage 0 instead of\n");
  printf ("                          computed)\n");
  printf ("    -b62_x value         (sample value/volts where we compute response for\n");
  printf ("                          B62)\n");
  printf ("    -v                   (verbose; list parameters on stdout)\n");
  printf ("    -x                   (expect FDSN StationXML format, default autodetect)\n\n");
  printf ("  NOTES:\n\n");
  printf ("    (1) If the 'file' argument is a directory, that directory will be\n");
  printf ("        searched for files of the form RESP.NETID.STA.CHA\n");
  printf ("        of this type are created by rdseed when it is run with the\n");
  printf ("    (2) If the 'file' argument is a file, the format of the file is\n");
  printf ("        autodetected to be either StationXML or RESP, defaulting to RESP \n");
  printf ("        The expected format can be forced to StationXML using '-x'\n");
  printf ("    (3) If the 'file' argument is missing, the current directory\n");
  printf ("        will be searched for files of the form RESP.NETID.STA.CHA.\n");
  printf ("    (4) The directory indicated by the environment variable SEEDRESP\n");
  printf ("        will also be searched for the requested files (if it is defined),\n");
  printf ("        but if matching station-channel-network values are found in\n");
  printf ("        both directories, then the local files take precedence.\n");
  printf ("    (5) The NETID (above) indicates a one or two letter network code.\n");
  printf ("        If no network code exists in the SEED volume, a value of '',\n");
  printf ("         '\?\?', or '*' will match the 'no network' case.\n");
  printf ("    (6) The '-stage' option allows the user to specify the range of\n");
  printf ("        stages that will be included in the calculation.  If only one\n");
  printf ("        stage number is given, a response will only be calculated for\n");
  printf ("        that stage.  If both a start and stop stage number are given,\n");
  printf ("        any stage between (and including) the start and stop stages\n");
  printf ("        will be included in the calculation.\n");
  printf ("    (7) -b62_x defines a value in counts or volts where response is\n");
  printf ("        computed. This flag only is applied to responses with B62.\n\n");
  printf ("  EXAMPLES:\n\n");
  printf ("    evalresp AAK,ARU,TLY VHZ 1992 21 0.001 10 100 -f /EVRESP/NEW/rdseed.out\n");
  printf ("    evalresp KONO BHN,BHE 1992 1 0.001 10 100 -f /EVRESP/NEW -t 12:31:04 -v\n");
  printf ("    evalresp FRB BHE,BHZ 1994 31 0.001 10 100 -f resp.all_stations -n '*' -v\n\n");
}

int
parse_args (int argc, char *argv[], evalresp_options *options, evalresp_filter *filter, evalresp_logger **log)
{
  int status = EVALRESP_OK, i;
  int first_switch = 0, flags_argc, option, index, format_set = 0;
  char *program = argv[0], **flags_argv;
  char *minfreq, *location = NULL, *network = NULL;

  struct option cmdline_flags[] = {
      {"file", required_argument, 0, 'f'},
      {"units", required_argument, 0, 'u'},
      {"time", required_argument, 0, 't'},
      {"spacing", required_argument, 0, 's'},
      {"network", required_argument, 0, 'n'},
      {"location", required_argument, 0, 'l'},
      {"response", required_argument, 0, 'r'},
      {"stage", required_argument, 0, 'S'},
      {"stdio", no_argument, &options->use_stdio, 1},
      {"use-estimated-delay", no_argument, 0, 'U'},
      {"use-delay", no_argument, 0, 'U'},
      {"il", no_argument, &options->b55_interpolate, 1},
      {"ii", no_argument, &options->b55_interpolate, 1},
      {"unwrap", no_argument, &options->unwrap_phase, 1},
      {"ts", no_argument, &options->use_total_sensitivity, 1},
      {"b62_x", required_argument, 0, 'b'},
      {"verbose", no_argument, 0, 'v'},
      {"xml", no_argument, &options->station_xml, 1},
      {0, 0, 0, 0}};

  if (argc < 5)
  {
    evalresp_log (*log, EV_ERROR, EV_ERROR, "Too few arguments");
    status = EVALRESP_INP;
  }

  if (!status)
  {
    while (++first_switch < argc && (strncmp (argv[first_switch], "-", 1) != 0 || is_real (argv[first_switch], *log)))
      ;
    if (first_switch < 5)
    {
      evalresp_log (*log, EV_ERROR, EV_ERROR,
                    "Not all of the required inputs are present (%d missing), type '%s' for usage",
                    8 - first_switch, program);
      status = EVALRESP_INP;
    }
    else
    {
      minfreq = first_switch > 5 ? argv[5] : "1.0";
      status = evalresp_set_frequency (*log, options,
                                       minfreq, first_switch > 6 ? argv[6] : minfreq, first_switch > 7 ? argv[7] : "1");
    }
  }

  if (!status)
  {
    for (i = 8; i < first_switch; ++i)
    {
      if (argv[i] != NULL && argv[i][0] != '\0')
      {
        evalresp_log (*log, EV_WARN, EV_WARN, "Unrecognized parameter:  %s\n", argv[i]);
      }
    }
  }

  if (argc - 1 >= first_switch)
  {
    flags_argc = argc - first_switch + 1;
    flags_argv = argv + first_switch - 1;

    while (!status && -1 != (option = getopt_long_only (flags_argc, flags_argv, ":f:u:t:s:n:l:r:S:Ub:vx", cmdline_flags, &index)))
    {
      switch (option)
      {

      case 0: /* This is to handle ones that get set automatically */
        break;

      case 'f':
        status = evalresp_set_filename (*log, options, optarg);
        break;

      case 'u':
        status = evalresp_set_unit (*log, options, optarg);
        break;

      case 't':
        status = evalresp_set_time (*log, filter, optarg);
        break;

      case 's':
        status = evalresp_set_spacing (*log, options, optarg);
        break;

      case 'n':
        network = strdup (optarg);
        break;

      case 'l':
        location = strdup (optarg);
        break;

      case 'r':
        status = evalresp_set_format (*log, options, optarg);
        format_set = 1;
        break;

      case 'S':
        status = evalresp_set_start_stage (*log, options, optarg);
        if (!status && optind < flags_argc && is_int (flags_argv[optind], *log))
        {
          status = evalresp_set_stop_stage (*log, options, flags_argv[optind++]);
        }
        break;

      case 'U':
        options->use_estimated_delay = 1;
        break;

      case 'b':
        status = evalresp_set_b62_x (*log, options, optarg);
        break;

      case 'v':
        options->verbose++;
        break;

      case 'x':
        options->station_xml = 1;
        break;

      case ':': /* invalid argument for flag */
        if (cmdline_flags[index].name)
        {
          evalresp_log (*log, EV_ERROR, EV_ERROR, "Missing argument for option '%s'", cmdline_flags[index].name);
        }
        else
        {
          evalresp_log (*log, EV_ERROR, EV_ERROR, "Missing argument for option '%c'", optopt);
        }
        status = EVALRESP_INP;
        break;

      case '?': /* unrecognized flag */
        evalresp_log (*log, EV_WARN, EV_WARN, "Unrecognized option: %s\n", flags_argv[optind - 1]);
        break;

      default:
        evalresp_log (*log, EV_ERROR, EV_ERROR, "Unexpected option '%c'", option);
        status = EVALRESP_ERR;
      }
    }
  }

  // default appears to be different for stdio (see commit 974d6764)
  if (!format_set && options->use_stdio)
  {
    options->format = evalresp_fap_output_format;
  }

  if (!status)
  {
    if (!(status = evalresp_add_sncl_all (*log, filter, network, argv[1], location, argv[2])))
    {
      if (!(status = evalresp_set_year (*log, filter, argv[3])))
      {
        status = evalresp_set_julian_day (*log, filter, argv[4]);
      }
    }
  }

  // TODO - construct log to stderr that uses verbose as the verbosity level

  if (status)
  {
    usage (program);
  }

  free (network);
  free (location);
  return status;
}

int
main (int argc, char *argv[])
{
  int status = EVALRESP_OK;
  evalresp_logger *log = NULL;
  evalresp_options *options = NULL;
  evalresp_filter *filter = NULL;

  if (!(status = evalresp_new_options (log, &options)))
  {
    if (!(status = evalresp_new_filter (log, &filter)))
    {
      if (!(status = parse_args (argc, argv, options, filter, &log)))
      {
        status = evalresp_cwd_to_cwd (log, options, filter);
      }
    }
  }

  // TODO - free the log allocated in parse_args
  evalresp_free_options (&options);
  evalresp_free_filter (&filter);
  return status;
}
