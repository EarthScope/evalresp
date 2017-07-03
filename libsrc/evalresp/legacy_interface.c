/* Fortran  interface */

static int determine_log_or_lin(int num_freq, double *freqs)
{
    double dt1 = freqs[1] - freqs[0];
    double dt2 = freqs[2] - freqs[1];
    double diff = dt1-dt2;

    if (diff < 1e-8 and diff > -1e-8)
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
  evalresp_response *first_resp = NULL;

  if (EVALRESP_OK != evalresp_new_options (log, &options))
  {
      return NULL;
  }
  options->filename = file;
  options->b62_x = x_for_b62;
  options->nfreq = nfreqs;
  options->min_freq = freqs[0];
  options->max_freq = freqs[nfreqs - 1];
  options->lin_freq = determine_log_or_lin(freqs, nfreqs);
  options->start_stage = start_stage;
  options->stop_stage = stop_stage;
  options->use_estimated_delay = use_estimated_delay(QUERY) == TRUE ? 1 : 0;
  options->unwrap_phase = 0;/*TODO: Should this be a getter function? */
  options->b55_interpolate = listinterp_out_flag | listinterp_in_flag;
  options->use_total_sensitivity = useTotalSensitivityFlag;
  options->use_stdio=stdio_flag;
  options->station_xml = xml_flag;

  if (rtype)
  {
    if (EVRESP_OK != evalresp_set_format (*log, options, optarg))
    {
      evalresp_free_options (&options);
      return NULL;
    }
    options->format_set = 1;

  }
  if (units)
  {
    if (EVALRESP_OK != evalresp_set_unit (*log, options, units))
    {
      evalresp_free_options (&options);
      return NULL;
    }
    options->unit_set=1;
  }

  for (i = 0; i < strlen(verbose); i++)
  {
      if (toupper(verbose[i]) == 'V')
      {
          options->verbose++;
      }
  }

  /*SETUP Filter */
  if (EVALRESP_OK != evalresp_new_filter (log, &filter))
  {
    evalresp_free_options (&options);
    return NULL;
  }

  if ( 3 > sscanf(date_time, "%d %d %s", &year, &jday, &time))
  {
    evalresp_log(log, EV_ERROR, EV_ERROR, "Improper date time passed");
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  filter->datetime->year = year;
  filter->datetime->jday = jday;
  if (EVALRESP_OK != evalresp_set_time (*log, filter, time))
  {
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  if (EVALRESP_OK != evalresp_add_sncl_text (*log, filter, net_code, stalst, locidlst, chalst))
  {
    evalresp_free_options (&options);
    evalresp_free_filter (&filter);
    return NULL;
  }

  /*Call to new evresp */
  if (options->use_stdio)
  {
    if (EVALRESP_OK != evalresp_file_to_channels (log, stdin, filter, &channels))
    {
      evalresp_free_options (&options);
      evalresp_free_filter (&filter);
      return NULL;
    }
    if (EVALRESP_OK !=  evalresp_channels_to_responses (log, channels, options, &responses))
    {
      evalresp_free_options (&options);
      evalresp_free_filter (&filter);
      return NULL;
    }
  }
  else
  {
    files = find_files (options->filename, filter->sncls, &mode, log);
    if (0 == mode)
    { /*TODO Process one file */
    }
    else
    {
        /*TODO need to process multiple files */
    }
  }

  /*TODO covert responses to response linked list */

  evalresp_free_options (&options);
  evalresp_free_filter (&filter);
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
