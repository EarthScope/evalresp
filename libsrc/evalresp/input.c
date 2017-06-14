
#ifndef EVALRESP_INPUT_H
#define EVALRESP_INPUT_H

#include "./ugly.h"
#include "./private.h"
#include "evalresp/public_channels.h"
#include "evalresp_log/log.h"

// code from parse_fctns.c heavily refactored to (1) parse all lines and (2)
// read from strings rather than files.

void
slurp_line(char **seed, char *line, int maxlen)
{
  int i;
  for (i = 0; **seed && **seed != '\n' && i < maxlen-1; i++, (*seed)++) line[i] = **seed;
  line[i] = '\0';
  if (**seed == '\n') (*seed)++;
}

//int
//read_line (evalresp_log_t *log, char **seed, char *return_line, int blkt_no, int fld_no, char *sep)
//{
//  char *lcl_ptr, *start, line[MAXLINELEN];
//  int lcl_blkt, lcl_fld, test;
//  int tmpint;
//  char tmpstr[200];
//  int i;
//
//  start = *seed;
//  while (*test && *test == '#')
//  {
//    strncpy (line, "", MAXLINELEN - 1);
//    (void)fgets (line, MAXLINELEN, fptr);
//    test = fgetc (fptr);
//  }
//
//  if (test == EOF)
//  {
//    return (0);
//  }
//  else
//  {
//    ungetc (test, fptr);
//    (void)fgets (line, MAXLINELEN, fptr);
//
//    for (i = 0; i < strlen (line); i++)
//    {
//      if ('\t' == line[i])
//        line[i] = ' ';
//    }
//
//    /* check for blank line */
//    tmpint = sscanf (line, "%s", tmpstr);
//
//    if (tmpint == EOF)
//    {
//      return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
//    }
//
//    tmpint = strlen (line); /* strip any trailing CR or LF chars */
//    while (tmpint > 0 && line[tmpint - 1] < ' ')
//      line[--tmpint] = '\0';
//  }
//
//  /*if(!line)
//     error_return(UNEXPECTED_EOF, "get_line; no more non-comment lines found in file");*/
//
//  test = parse_pref (&lcl_blkt, &lcl_fld, line, log);
//  if (!test)
//  {
//    evalresp_log (log, ERROR, 0,
//                  "get_line; unrecogn. prefix on the following line:\n\t  '%s'",
//                  line);
//    return UNDEF_PREFIX;
//    /*XXX error_return(UNDEF_PREFIX,
//                "get_line; unrecogn. prefix on the following line:\n\t  '%s'",
//                line); */
//  }
//
//  /* check the blockette and field numbers found on the line versus the expected values */
//
//  if (blkt_no != lcl_blkt)
//  {
//    /* try to parse the next line */
//    return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
//    /*
//         removed by SBH 2004.079
//         if(fld_no != lcl_fld) {
//         error_return(PARSE_ERROR,"get_line; %s%s%3.3d%s%3.3d%s%2.2d%s%2.2d","blkt",
//         " and fld numbers do not match expected values\n\tblkt_xpt=B",
//         blkt_no, ", blkt_found=B", lcl_blkt, "; fld_xpt=F", fld_no,
//         ", fld_found=F", lcl_fld);
//         }
//         */
//  }
//  else if (fld_no != lcl_fld)
//  {
//    /* try to parse the next line */
//    return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
//    /*
//         removed by SBH 2004.079
//         error_return(PARSE_ERROR,"get_line (parsing blockette [%3.3d]); %s%2.2d%s%2.2d",
//         lcl_blkt, "unexpected fld number\n\tfld_xpt=F", fld_no,
//         ", fld_found=F", lcl_fld, lcl_blkt);
//         */
//  }
//
//  if ((lcl_ptr = strstr (line, sep)) == (char *)NULL)
//  {
//    evalresp_log (log, ERROR, 0, "get_line; seperator string not found");
//    return UNDEF_SEPSTR;
//    /*XXX error_return(UNDEF_SEPSTR, "get_line; seperator string not found"); */
//  }
//  else if ((lcl_ptr - line) > (int)(strlen (line) - 1))
//  {
//    evalresp_log (log, ERROR, 0,
//                  "get_line; nothing to parse after seperator string");
//    return UNDEF_SEPSTR;
//    /*XXX error_return(UNDEF_SEPSTR,
//                "get_line; nothing to parse after seperator string"); */
//  }
//
//  lcl_ptr++;
//  while (*lcl_ptr && isspace (*lcl_ptr))
//  {
//    lcl_ptr++;
//  }
//
//  if ((lcl_ptr - line) > (int)strlen (line))
//  {
//    evalresp_log (log, ERROR, 0,
//                  "get_line; no non-white space after seperator string");
//    return UNDEF_SEPSTR;
//    /*XXX error_return(UNDEF_SEPSTR,
//                "get_line; no non-white space after seperator string"); */
//  }
//
//  strncpy (return_line, lcl_ptr, MAXLINELEN);
//  return (strlen (return_line));
//}
//
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
//  return (parse_field (line, fld_wanted, return_field, log));
//
//  /* and return the length of the field */
//  /*XXX this is the same as what parse_field returns
//    return (strlen(return_field)); */
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
