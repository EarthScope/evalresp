#include <check.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp/private.h"
#include "evalresp/public.h"
#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"
#include "legacy.h"
#include "old_fctns.h"
#include <setjmp.h>

extern int in_epoch (const char *datime, const char *beg_t, const char *end_t);
char SEEDUNITS[][UNITS_STR_LEN] =
    {"Undef Units", "Displacement", "Velocity", "Acceleration", "Counts",
     "Volts", "", "Pascals", "Tesla", "Centigrade"};

char FirstLine[MAXLINELEN];
int FirstField;
jmp_buf jump_buffer;

int
next_resp (FILE *fptr, evalresp_logger *log)
{
  int blkt_no, fld_no, test;
  char tmp_line[MAXLINELEN];

  while ((test = check_line (fptr, &blkt_no, &fld_no, tmp_line, log)) != 0 && blkt_no != 50)
    ;

  if (test && blkt_no == 50)
  {
    if (0 > parse_field (tmp_line, 2, FirstLine, log))
    {
      return 0;
    }
    return (1);
  }
  else
    return (0);
}

int
get_channel (FILE *fptr, evalresp_channel *chan, evalresp_logger *log)
{
  int blkt_no, fld_no;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* check to make sure a non-comment field exists and it is the sta/chan/date info.
     Note:  If it is the first channel (and, as a result, FirstLine contains a null
     string), then we have to get the next line.  Otherwise, the FirstLine argument
     has been set by a previous call to 'next_line' in another routine (when parsing
     a previous station-channel-network tuple's information) and this is the
     line that should be parsed to get the station name */

  chan->nstages = 0;
  chan->sensfreq = 0.0;
  chan->sensit = 0.0;
  chan->calc_sensit = 0.0;
  chan->calc_delay = 0.0;
  chan->estim_delay = 0.0;
  chan->applied_corr = 0.0;
  chan->sint = 0.0;

  if (!strlen (FirstLine))
  {
    if (0 > get_field (fptr, field, 50, 3, ":", 0, log))
    {
      return 0;
    }
  }
  else
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return 0;
    }
  }

  strncpy (chan->staname, field, STALEN);

  /* then (from the file) the Network ID */

  if (0 > get_field (fptr, field, 50, 16, ":", 0, log))
  {
    return 0;
  }
  if (!strncmp (field, "??", 2))
  {
    strncpy (chan->network, "", NETLEN);
  }
  else
  {
    strncpy (chan->network, field, NETLEN);
  }

  /* then (from the file) the Location Identifier (if it exists ... it won't for
     "old style" RESP files so assume old style RESP files contain a null location
     identifier) and the channel name */

  /* Modified to use 'next_line()' and 'parse_field()' directly
     to handle case where file contains "B052F03 Location:" and
     nothing afterward -- 10/19/2005 -- [ET] */
  /*  test_field(fptr,field,&blkt_no,&fld_no,":",0); */

  next_line (fptr, line, &blkt_no, &fld_no, ":", log);
  if (strlen (line) > 0) /* if data after "Location:" then */
  {
    if (0 > parse_field (line, 0, field, log)) /* parse location data */
    {
      return 0;
    }
  }
  else
  {
    /* if no data after "Location:" then */
    field[0] = '\0'; /* clear 'field' string */
  }

  if (blkt_no == 52 && fld_no == 3)
  {
    if (strlen (field) <= 0 || !strncmp (field, "??", 2))
    {
      strncpy (chan->locid, "", LOCIDLEN);
    }
    else
    {
      strncpy (chan->locid, field, LOCIDLEN);
    }
    if (0 > get_field (fptr, field, 52, 4, ":", 0, log))
    {
      return 0;
    }
    strncpy (chan->chaname, field, CHALEN);
  }
  else if (blkt_no == 52 && fld_no == 4)
  {
    strncpy (chan->locid, "", LOCIDLEN);
    strncpy (chan->chaname, field, CHALEN);
  }
  else
  {
#ifdef LIB_MODE
    return 0;
#else
    evalresp_log (log, EV_ERROR, 0,
                  "get_line; %s%s%3.3d%s%3.3d%s[%2.2d|%2.2d]%s%2.2d", "blkt",
                  " and fld numbers do not match expected values\n\tblkt_xpt=B",
                  52, ", blkt_found=B", blkt_no, "; fld_xpt=F", 3, 4,
                  ", fld_found=F", fld_no);
    return 0;
#endif
  }

  /* get the Start Date */

  if (0 > get_line (fptr, line, 52, 22, ":", log))
  {
    return 0;
  }
  strncpy (chan->beg_t, line, DATIMLEN);

  /* get the End Date */

  if (0 > get_line (fptr, line, 52, 23, ":", log))
  {
    return 0;
  }
  strncpy (chan->end_t, line, DATIMLEN);

  return (1);
}

int
find_resp (FILE *fptr, evalresp_sncls *scn_lst, char *datime,
           evalresp_channel *this_channel, evalresp_logger *log)
{
  int test, i;
  evalresp_sncl *scn = NULL;

  while ((test = get_channel (fptr, this_channel, log)) != 0)
  {
    for (i = 0; i < scn_lst->nscn; i++)
    {
      scn = scn_lst->scn_vec[i];
      if (string_match (this_channel->staname, scn->station, "-g", log) && ((!strlen (scn->network) && !strlen (this_channel->network)) || string_match (this_channel->network, scn->network, "-g", log)) && string_match (this_channel->locid, scn->locid, "-g", log) && string_match (this_channel->chaname, scn->channel, "-g", log) && in_epoch (datime, this_channel->beg_t, this_channel->end_t))
      {
        scn->found = 1;
        return (i);
      }
    }
    if (!(test = next_resp (fptr, log)))
    {
      return (-1);
    }
  }
  return (-1);
}

int
get_resp (FILE *fptr, evalresp_sncl *scn, char *datime,
          evalresp_channel *this_channel, evalresp_logger *log)
{
  int test;

  while ((test = get_channel (fptr, this_channel, log)) != 0)
  {

    if (string_match (this_channel->staname, scn->station, "-g", log) && ((!strlen (scn->network) && !strlen (this_channel->network)) || string_match (this_channel->network, scn->network, "-g", log)) &&
        string_match (this_channel->locid, scn->locid, "-g", log) && string_match (this_channel->chaname, scn->channel, "-g", log) && in_epoch (datime, this_channel->beg_t, this_channel->end_t))
    {
      scn->found = 1;
      return (1);
    }
    else
    {
      if (!(test = next_resp (fptr, log)))
      {
        return (-1);
      }
    }
  }
  return (-1);
}

evalresp_response *
old_evresp_itp (char *stalst, char *chalst, char *net_code,
                char *locidlst, char *date_time, char *units, char *file, double *freqs,
                int nfreqs, char *rtype, char *verbose, int start_stage, int stop_stage,
                int stdio_flag, int listinterp_out_flag, int listinterp_in_flag,
                double listinterp_tension, int useTotalSensitivityFlag,
                double x_for_b62, int xml_flag, evalresp_logger *log)
{
  evalresp_channel this_channel;
  evalresp_sncl *scn;
  struct string_array *sta_list, *chan_list;
  struct string_array *locid_list;
  int i, j, k, count = 0, which_matched, test = 1, mode, new_file = 0;
  int err_type;
  char out_name[MAXLINELEN], locid[LOCIDLEN + 1];
  char *locid_ptr, *end_locid_ptr;
  struct matched_files *flst_head = (struct matched_files *)NULL;
  struct matched_files *flst_ptr = NULL, *output_files = NULL;
  struct file_list *lst_ptr = NULL, *tmp_ptr = NULL, *out_file = NULL,
                   *tmp_file = NULL;
  evalresp_response *resp = NULL, *next_ptr = NULL;
  evalresp_response *prev_ptr = (evalresp_response *)NULL;
  evalresp_response *first_resp = (evalresp_response *)NULL;
  evalresp_complex *output = NULL;
  evalresp_sncls *scns = NULL;
  FILE *fptr = NULL;
  FILE *in_fptr = NULL;
  double *freqs_orig = NULL; /* for saving the original frequencies */
  int nfreqs_orig;

  /* Let's save the original frequencies requested by a user since they can be overwritten */
  /* if we process blockette 55 IGD for version 3.2.17 of evalresp*/

  nfreqs_orig = nfreqs;
  freqs_orig = (double *)malloc (sizeof (double) * nfreqs_orig);
  memcpy (freqs_orig, freqs, sizeof (double) * nfreqs_orig);

  /* set 'GblChanPtr' to point to 'this_channel' */

  GblChanPtr = &this_channel;

  /* clear out the FirstLine buffer */

  memset (FirstLine, 0, sizeof (FirstLine));

  /* if the verbose flag is set, then print some diagnostic output (other than errors) */

  if (verbose && !strcmp (verbose, "-v"))
  {
    evalresp_log (log, EV_INFO, 0, "<< EVALRESP RESPONSE OUTPUT V%s >>", REVNUM);
  }

  /* set the values of first_units and last_units to null strings */

  strncpy (this_channel.staname, "", STALEN);
  strncpy (this_channel.network, "", NETLEN);
  strncpy (this_channel.locid, "", LOCIDLEN);
  strncpy (this_channel.chaname, "", CHALEN);
  strncpy (this_channel.beg_t, "", DATIMLEN);
  strncpy (this_channel.end_t, "", DATIMLEN);
  strncpy (this_channel.first_units, "", MAXLINELEN);
  strncpy (this_channel.last_units, "", MAXLINELEN);

  /* and initialize the linked list of pointers to filters */

  this_channel.first_stage = (evalresp_stage *)NULL;

  /* parse the "stalst" string to form a list of stations */

  for (i = 0; i < (int)strlen (stalst); i++)
  {
    if (stalst[i] == ',')
      stalst[i] = ' ';
  }
  if (!(sta_list = ev_parse_line (stalst, log)))
  {
    return NULL;
  }

  /* remove any blank spaces from the beginning and end of the string */

  locid_ptr = locidlst;
  strncpy (locid, "", LOCIDLEN);
  while (*locid_ptr && *locid_ptr == ' ')
    locid_ptr++;
  end_locid_ptr = locid_ptr + strlen (locid_ptr) - 1;
  while (end_locid_ptr > locid_ptr && *end_locid_ptr == ' ')
    end_locid_ptr--;
  strncpy (locid, locid_ptr, (end_locid_ptr - locid_ptr + 1));

  /* parse the "locidlst" string to form a list of locations  */
  if (!(locid_list = parse_delim_line (locid, ",", log)))
  {
    return NULL;
  }

  /* parse the "chalst" string to form a list of channels */
  for (i = 0; i < (int)strlen (chalst); i++)
  {
    if (chalst[i] == ',')
      chalst[i] = ' ';
  }

  if (!(chan_list = ev_parse_line (chalst, log)))
  {
    return NULL;
  }

  /* then form a set of network-station-locid-channel tuples to search for */
  scns = alloc_scn_list (
      chan_list->nstrings * sta_list->nstrings * locid_list->nstrings, log);
  if (!scns)
  {
    return NULL;
  }
  for (i = 0; i < sta_list->nstrings; i++)
  {
    for (j = 0; j < locid_list->nstrings; j++)
    {
      for (k = 0; k < chan_list->nstrings; k++, count++)
      {
        scn = scns->scn_vec[count];
        strncpy (scn->station, sta_list->strings[i], STALEN);
        // treat '??' as '*' after long discussion w rob, ilya and eric
        if (strlen (locid_list->strings[j]) == strspn (locid_list->strings[j], "?"))
        {
          strcpy (scn->locid, "*");
        }
        else if (strlen (locid_list->strings[j]) == strspn (locid_list->strings[j], " "))
        {
          memset (scn->locid, 0, LOCIDLEN);
        }
        else
        {
          strncpy (scn->locid, locid_list->strings[j], LOCIDLEN);
        }
        strncpy (scn->channel, chan_list->strings[k], CHALEN);
        strncpy (scn->network, net_code, NETLEN);
      }
    }
  }
#ifdef LOG_LABEL
  sprintf (myLabel, "[%s.%s.%s.%s]", scn->network, scn->station, scn->locid, scn->channel);
#else
  myLabel[0] = '\0';
#endif
  /* if input is from stdin, set fptr to stdin, else find whatever matching files there are */
  if (stdio_flag)
  {
    in_fptr = stdin;
    mode = 0;
  }
  else
  {
    flst_head = find_files (file, scns, &mode, log);
    flst_ptr = flst_head;
  }

  /* find the responses for each of the station channel pairs as they occur in the file */

  if (!mode && !stdio_flag)
  {
    curr_file = file;
    if (!(in_fptr = fopen (file, "r")))
    {
      evalresp_log (log, EV_ERROR, 0, "%s failed to open file %s\n", myLabel, file);
      return NULL;
    }
  }

  /* allocate space for the first response */
  resp = alloc_response (nfreqs, log);
  if (!resp)
  {
    return NULL;
  }

  for (i = 0; i < scns->nscn && (mode || test); i++)
  {
    /* allocate space for 'matched_files' pointer used to determine if a file has already been read */

    if (!stdio_flag)
      output_files = alloc_matched_files (log);

    /* then check the mode to determine if are parsing one file or a list
         of files (note: if input is from stdin, is one file) */

    if (!mode)
    {

      /* convert from xml format if necessary, logging error messages to stderr. */
      evalresp_xml_stream_to_resp_file (log, xml_flag, in_fptr, NULL, &fptr);
      if (in_fptr && in_fptr != stdin && xml_flag)
      {
        fclose (in_fptr);
        in_fptr = NULL;
      }
      else
      {
        fptr = in_fptr;
      }
      if (!fptr && !xml_flag)
      {
        return NULL;
      }
      //if (x2r_xml2resp_on_flag (&fptr, xml_flag, log))
      //  return NULL;
      //if (x2r_xml2resp_auto(&fptr, X2R_ERROR)) return NULL;

      which_matched = 0;
      while (test && which_matched >= 0)
      {
        if (!(err_type = setjmp (jump_buffer)))
        {
          new_file = 0;
          which_matched = find_resp (fptr, scns, date_time,
                                     &this_channel, log);

          /* found a station-channel-network that matched.  First construct
                     an output filename and compare to other output files. If this
                     filename doesn't match any of them, (or if it is the first
                     file found) parse the channel's response information.
                     Otherwise skip it (since a match has already been found) */

          sprintf (out_name, "%s.%s.%s.%s", this_channel.network,
                   this_channel.staname, this_channel.locid,
                   this_channel.chaname);
#ifdef LOG_LABEL
          sprintf (myLabel, "[%s]", out_name);
#else
          myLabel[0] = '\0';
#endif
          if (!stdio_flag)
          {
            tmp_file = output_files->first_list;
            for (k = 0; k < output_files->nfiles; k++)
            {
              out_file = tmp_file;
              if (!strcmp (out_file->name, out_name))
                break;
              tmp_file = out_file->next_file;
            }
          }
          if ((stdio_flag && !new_file) || !output_files->nfiles)
          {
            if (!stdio_flag)
            {
              output_files->nfiles++;
              out_file = alloc_file_list (log);
              output_files->first_list = out_file;
              out_file->name = alloc_char (strlen (out_name) + 1, log);
              strcpy (out_file->name, out_name);
            }
            new_file = 1;
          }
          else if ((stdio_flag && !new_file) || k == output_files->nfiles)
          {
            if (!stdio_flag)
            {
              output_files->nfiles++;
              out_file->next_file = alloc_file_list (log);
              tmp_file = out_file->next_file;
              out_file = tmp_file;
              out_file->name = alloc_char (strlen (out_name) + 1, log);
              strcpy (out_file->name, out_name);
            }
            new_file = 1;
          }
          else
            new_file = 0;

          if (new_file && which_matched >= 0)
          {
            /* fill in station-channel-net information for the response */

            strncpy (resp->station, this_channel.staname, STALEN);
            strncpy (resp->locid, this_channel.locid, LOCIDLEN);
            strncpy (resp->channel, this_channel.chaname, CHALEN);
            strncpy (resp->network, this_channel.network, NETLEN);
            output = resp->rvec;

            /* found a station channel pair that matched a response, so parse
                         the response into a channel/filter list */

            test = parse_channel (fptr, &this_channel, log);

            if (listinterp_in_flag && this_channel.first_stage->first_blkt->type == LIST)
            { /* flag set for interpolation and stage type is "List" */
              interpolate_list_blockette (
                  &(this_channel.first_stage->first_blkt->blkt_info.list.freq),
                  &(this_channel.first_stage->first_blkt->blkt_info.list.amp),
                  &(this_channel.first_stage->first_blkt->blkt_info.list.phase),
                  &(this_channel.first_stage->first_blkt->blkt_info.list.nresp),
                  freqs, nfreqs, log);
            }

            /* check the filter sequence that was just read */
            check_channel (log, &this_channel);

            /* If we process blockette 55, we should recompute resp->rvec */
            /* because the number of output responses is generally different from */
            /* what is the user requested */
            /*if we don't use blockette 55, we should set the frequencies to the original */
            /* user defined position if we did mess up with frequencies in -possible - blockette 55*/
            /* containing previous file. Modifications by I.Dricker IGD*/

            free (resp->rvec);
            /* 'freqs' array is passed in and should not be freed -- 10/18/2005 -- [ET] */
            /*        free(freqs); */

            if (this_channel.first_stage->first_blkt != NULL && this_channel.first_stage->first_blkt->type == LIST)
            {
              /*to prevent segmentation in case of bogus input files */
              nfreqs =
                  this_channel.first_stage->first_blkt->blkt_info.list.nresp;
              freqs = (double *)malloc (sizeof (double) * nfreqs); /* malloc a new vector */
              memcpy (freqs,
                      this_channel.first_stage->first_blkt->blkt_info.list.freq,
                      sizeof (double) * nfreqs); /*cp*/
              resp->rvec = alloc_complex (nfreqs, log);
              output = resp->rvec;
              resp->nfreqs = nfreqs;
              resp->freqs = (double *)malloc (
                  sizeof (double) * nfreqs); /* malloc a new vector */
              memcpy (resp->freqs,
                      this_channel.first_stage->first_blkt->blkt_info.list.freq,
                      sizeof (double) * nfreqs); /*cp*/
            }
            else
            {
              nfreqs = nfreqs_orig;
              freqs = (double *)malloc (sizeof (double) * nfreqs);  /* malloc a new vector */
              memcpy (freqs, freqs_orig, sizeof (double) * nfreqs); /*cp*/
              resp->rvec = alloc_complex (nfreqs, log);
              output = resp->rvec;
              resp->nfreqs = nfreqs;
              resp->freqs = (double *)malloc (
                  sizeof (double) * nfreqs); /* malloc a new vector */
              memcpy (resp->freqs, freqs_orig,
                      sizeof (double) * nfreqs); /*cp*/
            }

            /* normalize the response of the filter sequence */
            norm_resp (&this_channel, start_stage, stop_stage);

            /* calculate the response at the requested frequencies */
            calc_resp (&this_channel, freqs, nfreqs, output, units,
                       start_stage, stop_stage,
                       useTotalSensitivityFlag, x_for_b62);

            /* diagnostic output, if the user requested it */

            if (verbose && !strcmp (verbose, "-v"))
            {
              print_chan (&this_channel, start_stage, stop_stage,
                          stdio_flag, listinterp_out_flag,
                          listinterp_in_flag,
                          useTotalSensitivityFlag);
            }

            free (freqs); /* free array that was allocated above */

            /* and, finally, free the memory associated with this channel/filter
                         list and continue searching for the next match */

            free_channel (&this_channel);
            if (first_resp == (evalresp_response *)NULL)
            {
              first_resp = resp;
            }
            next_ptr = alloc_response (nfreqs, log);
            resp->next = next_ptr;
            prev_ptr = resp;
            resp = next_ptr;
          }
          else
          {
            strncpy (FirstLine, "", MAXLINELEN);
            test = next_resp (fptr, log);
          }
        }
        else
        {
          if (new_file)
            output_files->nfiles--;
          free_channel (&this_channel);
          /* catch errors that cause parsing to fail midstream */
          if (err_type == PARSE_ERROR || err_type == UNRECOG_FILTYPE || err_type == UNDEF_SEPSTR || err_type == IMPROP_DATA_TYPE || err_type == RE_COMP_FAILED || err_type == UNRECOG_UNITS)
          {
            strncpy (FirstLine, "", MAXLINELEN);
            test = next_resp (fptr, log);
          }
          else if (err_type == UNDEF_PREFIX)
          {
            test = 0;
          }
        }
      }
      if (!stdio_flag)
        free_matched_files (output_files); /* added 3/28/2006 -- [ET] */

      /* allocated one too many responses */

      evalresp_free_response (&resp);
      if (prev_ptr != (evalresp_response *)NULL)
        prev_ptr->next = (evalresp_response *)NULL;
      break;
    }
    else if (mode)
    {
      lst_ptr = flst_ptr->first_list;
      scn = scns->scn_vec[i];
    next_scn:
      for (j = 0; j < flst_ptr->nfiles; j++)
      {
        if (!stdio_flag)
        {
          in_fptr = fopen (lst_ptr->name, "r");
        }
        if (in_fptr)
        {

          /* convert from xml format if necessary, logging error messages to stderr. */
          evalresp_xml_stream_to_resp_file (log, xml_flag, in_fptr, NULL, &fptr);
          if (in_fptr && in_fptr != stdin && xml_flag)
          {
            fclose (in_fptr);
            in_fptr = NULL;
          }
          else
          {
            fptr = in_fptr;
          }
          if (!fptr)
          {
            return NULL;
          }
          //if (x2r_xml2resp_on_flag (&fptr, xml_flag, log))
          //  return NULL;
          //if (x2r_xml2resp_auto(&fptr, X2R_ERROR)) return NULL;

          curr_file = lst_ptr->name;
        look_again:
          if (!(err_type = setjmp (jump_buffer)))
          {
            new_file = 0;
            which_matched = get_resp (fptr, scn, date_time, &this_channel, log);
            if (which_matched >= 0)
            {

              /* found a station-channel-network that matched.  First construct
                             an output filename and compare to other output files. If this
                             filename doesn't match any of them, (or if it is the first
                             file found) parse the channel's response information.
                             Otherwise skip it (since a match has already been found) */

              sprintf (out_name, "%s.%s.%s.%s",
                       this_channel.network, this_channel.staname,
                       this_channel.locid, this_channel.chaname);
#ifdef LOG_LABEL
              sprintf (myLabel, "[%s]", out_name);
#else
              myLabel[0] = '\0';
#endif
              tmp_file = output_files->first_list;
              for (k = 0; k < output_files->nfiles; k++)
              {
                out_file = tmp_file;
                if (!strcmp (out_file->name, out_name))
                  break;
                tmp_file = out_file->next_file;
              }
              if (!output_files->nfiles)
              {
                output_files->nfiles++;
                out_file = alloc_file_list (log);
                output_files->first_list = out_file;
                out_file->name = alloc_char (
                    strlen (out_name) + 1, log);
                strcpy (out_file->name, out_name);
                new_file = 1;
              }
              else if (k == output_files->nfiles)
              {
                output_files->nfiles++;
                out_file->next_file = alloc_file_list (log);
                tmp_file = out_file->next_file;
                out_file = tmp_file;
                out_file->name = alloc_char (
                    strlen (out_name) + 1, log);
                strcpy (out_file->name, out_name);
                new_file = 1;
              }
              else
                new_file = 0;

              if (new_file)
              {
                /* fill in station-channel-net information for the response */

                strncpy (resp->station, this_channel.staname,
                         STALEN);
                strncpy (resp->locid, this_channel.locid,
                         LOCIDLEN);
                strncpy (resp->channel, this_channel.chaname,
                         CHALEN);
                strncpy (resp->network, this_channel.network,
                         NETLEN);
                output = resp->rvec;

                /* parse the response into a channel/filter list */

                test = parse_channel (fptr, &this_channel, log);

                /* IGD 01/04/01 Add code preventing a user from defining output units as DIS and ACC if
                                 the input units are PRESSURE after */
                if (strncmp (this_channel.first_units, "PA -", 4) == 0)
                {
                  if (strcmp (units, "VEL") != 0)
                  {
                    if (strcmp (units, "DEF") != 0)
                    {
                      evalresp_log (log, EV_WARN, 0,
                                    "%s: OUTPUT %s does not make sense if INPUT is PRESSURE\n",
                                    myLabel, units);
                      strcpy (units, "VEL");
                      evalresp_log (log, EV_WARN, 0,
                                    "%s      OUTPUT units are reset and interpreted as PRESSURE\n",
                                    myLabel);
                    }
                  }
                }
                /* IGD 08/21/06 Add code preventing a user from defining output units as DIS and ACC if
                                 the input units are TESLA */
                if (strncmp (this_channel.first_units, "T -", 3) == 0)
                {
                  if (strcmp (units, "VEL") != 0)
                  {
                    if (strcmp (units, "DEF") != 0)
                    {
                      evalresp_log (log, EV_WARN, 0,
                                    "%s: OUTPUT %s does not make sense if INPUT is MAGNETIC FLUX\n",
                                    myLabel, units);
                      strcpy (units, "VEL");
                      evalresp_log (log, EV_WARN, 0,
                                    "%s      OUTPUT units are reset and interpreted as TESLA\n",
                                    myLabel);
                    }
                  }
                }

                /* IGD 10/03/13 Add code preventing a user from defining output units as DIS and ACC if
                                 the input units are CENTIGRADE */
                if (strncmp (this_channel.first_units, "C -", 3) == 0)
                {
                  if (strcmp (units, "VEL") != 0)
                  {
                    if (strcmp (units, "DEF") != 0)
                    {
                      evalresp_log (log, EV_WARN, 0,
                                    "%s: OUTPUT %s does not make sense if INPUT is TEMPERATURE\n",
                                    myLabel, units);
                      strcpy (units, "VEL");
                      evalresp_log (log, EV_WARN, 0,
                                    "%s      OUTPUT units are reset and interpreted as  DEGREES CENTIGRADE\n",
                                    myLabel);
                    }
                  }
                }

                if (listinterp_in_flag && this_channel.first_stage->first_blkt->type == LIST)
                {
                  /* flag set for interpolation and stage type is "List" */
                  interpolate_list_blockette (
                      &(this_channel.first_stage->first_blkt->blkt_info.list.freq),
                      &(this_channel.first_stage->first_blkt->blkt_info.list.amp),
                      &(this_channel.first_stage->first_blkt->blkt_info.list.phase),
                      &(this_channel.first_stage->first_blkt->blkt_info.list.nresp),
                      freqs, nfreqs, log);
                }

                /* check the filter sequence that was just read */
                check_channel (log, &this_channel);

                /* If we process blockette 55, we should recompute resp->rvec */
                /* because the number of output responses is generally different from */
                /* what is the user requested */
                /*if we don't use blockette 55, we should set the frequencies to the original */
                /* user defined position if we did mess up with frequencies in -possible - blockette 55*/
                /* containing previous file. Modifications by I.Dricker / IGD */

                free (resp->rvec);
                /* 'freqs' array is passed in and should not be freed -- 10/18/2005 -- [ET] */
                /* free(freqs); */
                if (this_channel.first_stage->first_blkt != NULL && this_channel.first_stage->first_blkt->type == LIST)
                {
                  /* This is to prevent segmentation if the response input is bogus responses */
                  nfreqs =
                      this_channel.first_stage->first_blkt->blkt_info.list.nresp;
                  freqs = (double *)malloc (
                      sizeof (double) * nfreqs); /* malloc a new vector */
                  memcpy (freqs,
                          this_channel.first_stage->first_blkt->blkt_info.list.freq,
                          sizeof (double) * nfreqs); /*cp*/
                  resp->rvec = alloc_complex (nfreqs, log);
                  output = resp->rvec;
                  resp->nfreqs = nfreqs;
                  resp->freqs = (double *)malloc (
                      sizeof (double) * nfreqs); /* malloc a new vector */
                  memcpy (resp->freqs,
                          this_channel.first_stage->first_blkt->blkt_info.list.freq,
                          sizeof (double) * nfreqs); /*cp*/
                }
                else
                {
                  nfreqs = nfreqs_orig;
                  freqs = (double *)malloc (
                      sizeof (double) * nfreqs); /* malloc a new vector */
                  memcpy (freqs, freqs_orig,
                          sizeof (double) * nfreqs); /*cp*/
                  resp->rvec = alloc_complex (nfreqs, log);
                  output = resp->rvec;
                  resp->nfreqs = nfreqs;
                  resp->freqs = (double *)malloc (
                      sizeof (double) * nfreqs); /* malloc a new vector */
                  memcpy (resp->freqs, freqs_orig,
                          sizeof (double) * nfreqs); /*cp*/
                }

                /* normalize the response of the filter sequence */
                norm_resp (&this_channel, start_stage,
                           stop_stage);

                /* calculate the response at the requested frequencies */
                calc_resp (&this_channel, freqs, nfreqs, output,
                           units, start_stage, stop_stage,
                           useTotalSensitivityFlag, x_for_b62);

                /* diagnostic output, if the user requested it */

                if (verbose && !strcmp (verbose, "-v"))
                {
                  print_chan (&this_channel, start_stage,
                              stop_stage, stdio_flag,
                              listinterp_out_flag,
                              listinterp_in_flag,
                              useTotalSensitivityFlag);
                }

                free (freqs); /* free array that was allocated above */

                /* and, finally, free the memory associated with this
                                 channel/filter list and continue searching for the
                                 next match */

                free_channel (&this_channel);
                if (first_resp == (evalresp_response *)NULL)
                {
                  first_resp = resp;
                }
                next_ptr = alloc_response (nfreqs, log);
                resp->next = next_ptr;
                prev_ptr = resp;
                resp = next_ptr;
              }
              FirstField = 0;
              strncpy (FirstLine, "", MAXLINELEN);
              if (!stdio_flag)
              {
                fclose (fptr);
              }
            }
            else
            {
              strncpy (FirstLine, "", MAXLINELEN);
              test = next_resp (fptr, log);
              if (!test)
              {
                if (!stdio_flag)
                {
                  fclose (fptr);
                }
              }
            }

            /* if not the last file in the list, move on to the next one */
            if (lst_ptr->next_file != (struct file_list *)NULL)
            {
              tmp_ptr = lst_ptr->next_file;
              lst_ptr = tmp_ptr;
            }
          }
          else
          {
            if (new_file)
              output_files->nfiles--;
            /* catch errors that cause parsing to fail midstream */
            if (err_type == PARSE_ERROR || err_type == UNRECOG_FILTYPE || err_type == UNDEF_SEPSTR || err_type == IMPROP_DATA_TYPE || err_type == RE_COMP_FAILED || err_type == UNRECOG_UNITS)
            {
              strncpy (FirstLine, "", MAXLINELEN);
              test = next_resp (fptr, log);
            }
            else if (err_type == UNDEF_PREFIX)
            {
              test = 0;
            }
            free_channel (&this_channel);
            if (!test)
            {
              FirstField = 0;
              strncpy (FirstLine, "", MAXLINELEN);
              if (!stdio_flag)
                fclose (fptr);
            }
            else
              goto look_again;

            /* if not the last file in the list, move on to the next one */
            if (lst_ptr->next_file != (struct file_list *)NULL)
            {
              tmp_ptr = lst_ptr->next_file;
              lst_ptr = tmp_ptr;
            }
          }
        }
      }
      /* if not the last station-channel-network in the list, move on to the next one */
      if (i < (scns->nscn - 1))
      {
        flst_ptr = flst_ptr->ptr_next;
        lst_ptr = flst_ptr->first_list;
        i++;
        scn = scns->scn_vec[i];
        goto next_scn;
      }
      if (!stdio_flag)
        free_matched_files (output_files);

      /* allocated one too many responses */

      evalresp_free_response (&resp);
      if (prev_ptr != (evalresp_response *)NULL)
        prev_ptr->next = (evalresp_response *)NULL;

    } /* end else if mode */

  } /* end for loop */

  /* added file close if single input file -- 2/13/2006 -- [ET]: */
  if (!mode && !stdio_flag) /* if single file was opened then */
    fclose (fptr);          /* close input file */

  /* and print a list of WARNINGS about the station-channel pairs that were not
       found in the input RESP files */

  for (i = 0; i < scns->nscn; i++)
  {
    scn = scns->scn_vec[i];
    if (!scn->found)
    {
      evalresp_log (log, EV_WARN, 0,
                    "%s: no response found for NET=%s,STA=%s,LOCID=%s,CHAN=%s,DATE=%s\n",
                    myLabel, scn->network, scn->station, scn->locid,
                    scn->channel, date_time);
    }
  }
  evalresp_free_sncls (scns);
  if (flst_head != (struct matched_files *)NULL)
    free_matched_files (flst_head);
  free_string_array (chan_list);
  free_string_array (locid_list);
  free_string_array (sta_list);

  free (freqs_orig); /* added 3/28/2006 -- [ET] */

  return (first_resp);
}

START_TEST (test_legacy_1)
{
  evalresp_response *old_first = NULL, *new_first = NULL, *old_ptr, *new_ptr, *ptr;
  int oldcount = 0, newcount = 0, i;
  double freqs[3];
  freqs[0] = 2;
  freqs[1] = 3;
  freqs[2] = 4;

  ck_assert_msg (NULL != (old_first = old_evresp_itp ("ANMO", "BH1", "IU", "00",
                                                      "2015,1,00:00:00", "VEL",
                                                      "./data/response-1",
                                                      freqs, 3, "AP", NULL,
                                                      -1, 0, 0, 0, 0.0, 0, 0, 0, 0, NULL)),
                 "Failed to run old evresp_itp");
  ck_assert_msg (NULL != (new_first = evresp_itp ("ANMO", "BH1", "IU", "00",
                                                  "2015,1,00:00:00", "VEL",
                                                  "./data/response-1",
                                                  freqs, 3, "AP", NULL,
                                                  -1, 0, 0, 0, 0.0, 0, 0, 0, 0)),
                 "Failed to run evresp_itp");

  for (oldcount = 0, old_ptr = old_first; old_ptr != NULL; oldcount++, old_ptr = old_ptr->next)
    ;
  for (newcount = 0, new_ptr = new_first; new_ptr != NULL; newcount++, new_ptr = new_ptr->next)
    ;
  ck_assert_msg (oldcount == newcount, "number of responses do not match (new)%d != (old)%d", newcount, oldcount);
  old_ptr = old_first;
  new_ptr = new_first;
  while (old_ptr && new_ptr)
  {
    ck_assert (strcmp (old_ptr->station, new_ptr->station) == 0);
    ck_assert (strcmp (old_ptr->network, new_ptr->network) == 0);
    ck_assert (strcmp (old_ptr->locid, new_ptr->locid) == 0);
    ck_assert (strcmp (old_ptr->channel, new_ptr->channel) == 0);
    ck_assert (old_ptr->nfreqs == new_ptr->nfreqs);
    for (i = 0; i < old_ptr->nfreqs; i++)
    {
      ck_assert (old_ptr->freqs[i] == new_ptr->freqs[i]);
      ck_assert (old_ptr->rvec[i].real == new_ptr->rvec[i].real);
      ck_assert (old_ptr->rvec[i].imag == new_ptr->rvec[i].imag);
    }
    old_ptr = old_ptr->next;
    new_ptr = new_ptr->next;
  }
  old_ptr = old_first;
  new_ptr = new_first;
  while (old_ptr)
  {
    ptr = old_ptr->next;
    free (old_ptr);
    old_ptr = ptr;
  }
  while (new_ptr)
  {
    ptr = new_ptr->next;
    free (new_ptr);
    new_ptr = ptr;
  }
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_legacy_1);
  //  tcase_add_test (tc, test_legacy_2);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-legacy.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
