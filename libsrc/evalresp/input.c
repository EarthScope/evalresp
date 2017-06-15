
#ifndef EVALRESP_INPUT_H
#define EVALRESP_INPUT_H

#include <string.h>

#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_channels.h"
#include "evalresp_log/log.h"

// code from parse_fctns.c heavily refactored to (1) parse all lines and (2)
// read from strings rather than files.

void  // non-static only for testing
slurp_line (char **seed, char *line, int maxlen)
{
  int i = 0;
  for (;;)
  {
    char c = **seed;
    line[i++] = c;
    if (!c)
    {
      // don't move past end of input
      // already appended NULL
      return;
    }
    else
    {
      (*seed)++;
      if (c == '\n' || i == maxlen - 1)
      {
        line[i] = '\0';
        return;
      }
    }
  }
}

static int
end_of_string(char **seed) {
  return !**seed;
}

static int
blank_line(char *line) {
  int i;
  for (i = 0; i < strlen(line); ++i) {
    if (!isspace(line[i])) return 0;
  }
  return 1;
}

static void
drop_comments_and_blank_lines(char **seed) {
  char line[MAXLINELEN], *lookahead;
  while (!end_of_string(seed)) {
    lookahead = *seed;
    slurp_line(&lookahead, line, MAXLINELEN);
    if (*line != '#' && !blank_line(line)) return;
    *seed = lookahead;
  }
}

static void
remove_tabs_and_crlf(char *line) {
  int i;
  for (i = 0; i < strlen (line); ++i)
  {
    if (line[i] == '\t') {
      line[i] = ' ';
    }
  }
  for (--i; i && strspn(line+i, "\n\r"); --i) {
    line[i] = '\0';
  }
}

// TODO - return code is both length and error number (do we use these different error values?)
// TODO - error handling without multiple returns
// TODO - return_line must be MAXLINELEN in size (document this?  allocate it?)
int
read_line (evalresp_log_t *log, char **seed, char *return_line, int blkt_no, int fld_no, char *sep)
{
  char *lcl_ptr, line[MAXLINELEN];
  int lcl_blkt = -1, lcl_fld = -1;

  while (!end_of_string(seed) && (lcl_blkt != blkt_no || lcl_fld != fld_no)) {
    drop_comments_and_blank_lines(seed);
    if (end_of_string(seed)) return 0;
    slurp_line(seed, line, MAXLINELEN);
    remove_tabs_and_crlf(line);
    if (!parse_pref (&lcl_blkt, &lcl_fld, line, log)) {
      evalresp_log (log, ERROR, 0, "unrecognised prefix: '%s'", line);
      return UNDEF_PREFIX;
    }
  }

  return_line[0] = '\0';
  if (!(lcl_ptr = strstr (line, sep)))
  {
    evalresp_log (log, ERROR, 0, "separator '%s' not found in '%s'", sep, line);
    return UNDEF_SEPSTR;
  }
  for (lcl_ptr++; *lcl_ptr && isspace (*lcl_ptr); lcl_ptr++);
  if (!*lcl_ptr) {
    evalresp_log (log, ERROR, 0, "nothing to parse after '%s' in '%s'", sep, line);
    return UNDEF_SEPSTR;
  }
  strncpy (return_line, lcl_ptr, MAXLINELEN);
  return strlen (return_line);
}

//int
//read_field (evalresp_log_t *log, char **seed, char *return_field,
//    int blkt_no, int fld_no, char *sep, int fld_wanted)
//{
//  char line[MAXLINELEN];
//
//  /* first get the next non-comment line */
//  read_line (log, seed, line, blkt_no, fld_no, sep);
//
//  /* then parse the field that the user wanted from the line get_line returned */
//  return parse_field (line, fld_wanted, return_field, log);
//}
//
//int
//read_channel (evalresp_log_t *log, char **seed, evalresp_channel *chan)
//{
//  int blkt_no, fld_no;
//  char field[MAXFLDLEN], line[MAXLINELEN];
//
//  /* check to make sure a non-comment field exists and it is the sta/chan/date info.
//     Note:  If it is the first channel (and, as a result, FirstLine contains a null
//     string), then we have to get the next line.  Otherwise, the FirstLine argument
//     has been set by a previous call to 'next_line' in another routine (when parsing
//     a previous station-channel-network tuple's information) and this is the
//     line that should be parsed to get the station name */
//
//  chan->nstages = 0;
//  chan->sensfreq = 0.0;
//  chan->sensit = 0.0;
//  chan->calc_sensit = 0.0;
//  chan->calc_delay = 0.0;
//  chan->estim_delay = 0.0;
//  chan->applied_corr = 0.0;
//  chan->sint = 0.0;
//
//  if (0 > read_field (log, seed, field, 50, 3, ":", 0))
//  {
//    return 0 /*TODO PARSE_ERROR should be returned */;
//  }
//
//  strncpy (chan->staname, field, STALEN);
//
//  /* then (from the file) the Network ID */
//
//  if (0 > get_field (fptr, field, 50, 16, ":", 0, log))
//  {
//    return 0 /*TODO PARSE_ERROR should be returned */;
//  }
//  if (!strncmp (field, "??", 2))
//  {
//    strncpy (chan->network, "", NETLEN);
//  }
//  else
//  {
//    strncpy (chan->network, field, NETLEN);
//  }
//
//  /* then (from the file) the Location Identifier (if it exists ... it won't for
//     "old style" RESP files so assume old style RESP files contain a null location
//     identifier) and the channel name */
//
//  /* Modified to use 'next_line()' and 'parse_field()' directly
//     to handle case where file contains "B052F03 Location:" and
//     nothing afterward -- 10/19/2005 -- [ET] */
//  /*  test_field(fptr,field,&blkt_no,&fld_no,":",0); */
//
//  next_line (fptr, line, &blkt_no, &fld_no, ":", log);
//  if (strlen (line) > 0) /* if data after "Location:" then */
//  {
//    if (0 > parse_field (line, 0, field, log)) /* parse location data */
//    {
//      return 0 /*TODO PARSE_ERROR should be returned */;
//    }
//  }
//  else
//  {
//    /* if no data after "Location:" then */
//    field[0] = '\0'; /* clear 'field' string */
//  }
//
//  if (blkt_no == 52 && fld_no == 3)
//  {
//    if (strlen (field) <= 0 || !strncmp (field, "??", 2))
//    {
//      strncpy (chan->locid, "", LOCIDLEN);
//    }
//    else
//    {
//      strncpy (chan->locid, field, LOCIDLEN);
//    }
//    if (0 > get_field (fptr, field, 52, 4, ":", 0, log))
//    {
//      return 0 /*TODO PARSE_ERROR should be returned */;
//    }
//    strncpy (chan->chaname, field, CHALEN);
//  }
//  else if (blkt_no == 52 && fld_no == 4)
//  {
//    strncpy (chan->locid, "", LOCIDLEN);
//    strncpy (chan->chaname, field, CHALEN);
//  }
//  else
//  {
//#ifdef LIB_MODE
//    return 0;
//#else
//    evalresp_log (log, ERROR, 0,
//                  "get_line; %s%s%3.3d%s%3.3d%s[%2.2d|%2.2d]%s%2.2d", "blkt",
//                  " and fld numbers do not match expected values\n\tblkt_xpt=B",
//                  52, ", blkt_found=B", blkt_no, "; fld_xpt=F", 3, 4,
//                  ", fld_found=F", fld_no);
//    return 0; /*TODO error code maybe? */
///*XXX error_return(PARSE_ERROR,
//                "get_line; %s%s%3.3d%s%3.3d%s[%2.2d|%2.2d]%s%2.2d", "blkt",
//                " and fld numbers do not match expected values\n\tblkt_xpt=B",
//                52, ", blkt_found=B", blkt_no, "; fld_xpt=F", 3, 4,
//                ", fld_found=F", fld_no);*/
//#endif
//  }
//
//  /* get the Start Date */
//
//  if (0 > get_line (fptr, line, 52, 22, ":", log))
//  {
//    return 0; /*TODO */
//  }
//  strncpy (chan->beg_t, line, DATIMLEN);
//
//  /* get the End Date */
//
//  if (0 > get_line (fptr, line, 52, 23, ":", log))
//  {
//    return 0; /*TODO */
//  }
//  strncpy (chan->end_t, line, DATIMLEN);
//
//  return (1);
//}

#endif
