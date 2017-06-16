
#include <stdlib.h>
#include <string.h>

#include "./input.h"
#include "./private.h"
#include "./ugly.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

// code from parse_fctns.c heavily refactored to (1) parse all lines and (2)
// read from strings rather than files.

// non-static only for testing
void
slurp_line (const char **seed, char *line, int maxlen)
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
end_of_string (const char **seed)
{
  return !**seed;
}

static int
blank_line (char *line)
{
  int i;
  for (i = 0; i < strlen (line); ++i)
  {
    if (!isspace (line[i]))
      return 0;
  }
  return 1;
}

static void
drop_comments_and_blank_lines (const char **seed)
{
  char line[MAXLINELEN];
  const char *lookahead;
  while (!end_of_string (seed))
  {
    lookahead = *seed;
    slurp_line (&lookahead, line, MAXLINELEN);
    if (*line != '#' && !blank_line (line))
      return;
    *seed = lookahead;
  }
}

static void
remove_tabs_and_crlf (char *line)
{
  int i;
  for (i = 0; i < strlen (line); ++i)
  {
    if (line[i] == '\t')
    {
      line[i] = ' ';
    }
  }
  for (--i; i && strspn (line + i, "\n\r"); --i)
  {
    line[i] = '\0';
  }
}

// this was "next_line"
// TODO - return code is both length and error number (do we use these different error values?)
// TODO - error handling without multiple returns
// TODO - return_line must be MAXLINELEN in size (document this?  allocate it?)
// TODO - fld_no seems to be length in the seed specs?!  why are we checking this?!
// TODO - warning - original returned fld_no (as return val as well as param)
static int
read_line (evalresp_log_t *log, const char **seed, char *sep,
           int *blkt_no, int *fld_no, char *return_line)
{
  char *lcl_ptr, line[MAXLINELEN];

  drop_comments_and_blank_lines (seed);
  if (end_of_string (seed))
    return 0;

  slurp_line (seed, line, MAXLINELEN);
  remove_tabs_and_crlf (line);
  if (!parse_pref (blkt_no, fld_no, line, log))
  {
    evalresp_log (log, ERROR, 0, "unrecognised prefix: '%s'", line);
    return UNDEF_PREFIX;
  }

  return_line[0] = '\0';
  if (!(lcl_ptr = strstr (line, sep)))
  {
    evalresp_log (log, ERROR, 0, "separator '%s' not found in '%s'", sep, line);
    return UNDEF_SEPSTR;
  }
  for (lcl_ptr++; *lcl_ptr && isspace (*lcl_ptr); lcl_ptr++)
    ;
  if (!*lcl_ptr)
  {
    evalresp_log (log, ERROR, 0, "nothing to parse after '%s' in '%s'", sep, line);
    return UNDEF_SEPSTR;
  }
  strncpy (return_line, lcl_ptr, MAXLINELEN);
  return strlen (return_line);
}

// non-static only for testing
// this was "get_line"
// TODO - return code is both length and error number (do we use these different error values?)
// TODO - error handling without multiple returns
// TODO - return_line must be MAXLINELEN in size (document this?  allocate it?)
// TODO - fld_no seems to be length in the seed specs?!  why are we checking this?!
int
find_line (evalresp_log_t *log, const char **seed, char *sep, int blkt_no, int fld_no, char *return_line)
{
  int lcl_blkt, lcl_fld, length;

  while (!end_of_string (seed))
  {
    // TODO - do we need to suppress error logging in failures here
    if ((length = read_line (log, seed, sep, &lcl_blkt, &lcl_fld, return_line)))
    {
      if (blkt_no == lcl_blkt && fld_no == lcl_fld)
        return length;
    }
  }

  // TODO - extra error here?
  return 0;
}

// non-static only for testing
int
find_field (evalresp_log_t *log, const char **seed, char *sep,
            int blkt_no, int fld_no, int fld_wanted, char *return_field)
{
  char line[MAXLINELEN];

  /* first get the next non-comment line */
  find_line (log, seed, sep, blkt_no, fld_no, line);

  /* then parse the field that the user wanted from the line find_line returned */
  // TODO - how come length(return_field) isn't passed in here (or to this function)?
  return parse_field (line, fld_wanted, return_field, log);
}

// this was "read_channel"
static int
read_channel_header (evalresp_log_t *log, const char **seed, evalresp_channel *chan)
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

  if (0 > find_field (log, seed, ":", 50, 3, 0, field))
  {
    return 0 /*TODO PARSE_ERROR should be returned */;
  }

  strncpy (chan->staname, field, STALEN);

  /* then (from the file) the Network ID */

  if (0 > find_field (log, seed, ":", 50, 16, 0, field))
  {
    return 0 /*TODO PARSE_ERROR should be returned */;
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

  if (read_line (log, seed, ":", &blkt_no, &fld_no, line) > 0) /* if data after "Location:" */
  {
    if (0 > parse_field (line, 0, field, log)) /* parse location data */
    {
      return 0 /*TODO PARSE_ERROR should be returned */;
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
    if (0 > find_field (log, seed, ":", 52, 4, 0, field))
    {
      return 0 /*TODO PARSE_ERROR should be returned */;
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
    evalresp_log (log, ERROR, 0,
                  "get_line; %s%s%3.3d%s%3.3d%s[%2.2d|%2.2d]%s%2.2d", "blkt",
                  " and fld numbers do not match expected values\n\tblkt_xpt=B",
                  52, ", blkt_found=B", blkt_no, "; fld_xpt=F", 3, 4,
                  ", fld_found=F", fld_no);
    return 0; /*TODO error code maybe? */
  }

  /* get the Start Date */

  if (0 > find_line (log, seed, ":", 52, 22, line))
  {
    return 0; /*TODO */
  }
  strncpy (chan->beg_t, line, DATIMLEN);

  /* get the End Date */

  if (0 > find_line (log, seed, ":", 52, 23, line))
  {
    return 0; /*TODO */
  }
  strncpy (chan->end_t, line, DATIMLEN);

  return (1);
}

// this was parse_pz
// TODO - add error returns
static void
read_pz (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
         evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, check_fld, blkt_read, npoles, nzeros;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [53] the first field expected is a F03, if from a blockette [43], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_pz; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = first_field == 3 ? 53 : 43;

  if (0 > parse_field (first_line, 0, field, log))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, ERROR, 0,
                  "parse_pz; parsing (Poles & Zeros), illegal filter type ('%s')",
                  field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  switch (*field)
  {
  case 'A':
    blkt_ptr->type = LAPLACE_PZ;
    break;
  case 'B':
    blkt_ptr->type = ANALOG_PZ;
    break;
  case 'D':
    blkt_ptr->type = IIR_PZ;
    break;
  default:
    evalresp_log (log, ERROR, 0,
                  "parse_pz; parsing (Poles & Zeros), unexpected filter type ('%c')",
                  *field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  /* set the check-field counter to the next expected field */
  check_fld = first_field + 1;

  /* then, if is a B053F04, get the stage sequence number (from the file) */

  if (check_fld == 4)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* then the A0 normalization factor */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.pole_zero.a0 = get_double (field, log);

  /* the A0 normalization frequency */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.pole_zero.a0_freq = get_double (field, log);

  /* the number of zeros */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  nzeros = get_int (field, log);
  blkt_ptr->blkt_info.pole_zero.nzeros = nzeros;

  /* remember to allocate enough space for the number of zeros to follow */

  blkt_ptr->blkt_info.pole_zero.zeros = alloc_complex (nzeros, log);

  /* set the expected field to the current value (9 or 10 for [53] or [43])
     to the current value + 5 (14 or 15 for [53] or [43] respectively) */

  check_fld += 5;

  /* the number of poles */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  npoles = get_int (field, log);
  blkt_ptr->blkt_info.pole_zero.npoles = npoles;

  /* remember to allocate enough space for the number of poles to follow */

  blkt_ptr->blkt_info.pole_zero.poles = alloc_complex (npoles, log);

  /* set the expected field to the current value (14 or 15 for [53] or [43])
     to the current value - 4 (10 or 11 for [53] or [43] respectively) */

  check_fld -= 4;

  /* get the zeros */

  for (i = 0; i < nzeros; i++)
  {
    if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
    {
      return; /*TODO */
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].real = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].imag = atof (field);
  }

  /* set the expected field to the current value (10 or 11 for [53] or [43])
     to the current value + 5 (15 or 16 for [53] or [43] respectively) */

  check_fld += 5;

  /* and then get the poles */

  for (i = 0; i < npoles; i++)
  {
    if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
    {
      return; /*TODO */
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      /*XXX error_return(PARSE_ERROR, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')"); */
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].real = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].imag = atof (field);
  }
}

// was is_IIR_coeffs
static int
is_iir_coeffs (const char **seed)
{
  /* IGD Very narrow-specified function.                                    */
  /* It is used to check out if we are using a FIR or IIR coefficients      */
  /* in the  blockette 54. This information is contained in the 10th field  */
  /* of this blockette (number of denominators)                             */
  /* if the number of denominators is 0: we got FIR. Otherwise, it is IIR   */
  /* is_IIR_coeff reads the text response file; finds the proper line and   */
  /* decides if the response is IIR or FIR                                  */
  /* The text file is then fseek to the original position and               */
  /*   the function returns:                                                */
  /* 1 if it is IIR; 0 if it is not IIR;                                    */
  /*  it returns 0  in case of the error, so use it with a caution!         */
  /* IGD I.Dricker ISTI i.dricker@isti.com 07/00 for evalresp 3.2.17        */
  char line[MAXLINELEN];
  const char *lookahead = *seed;
  int i, denoms;
  for (i = 0; i < 80; i++)
  { /* enough to make sure we are getting field 10; not to much to get to the next blockette */
    (void)sscanf (lookahead, "%s", line);
    if (!strncmp (line, "B054F10", 7))
    {
      break;
    }
  }
  if (!strncmp (line, "B054F10", 7))
  {
    for (i = 0; i < 4; i++)
    {
      (void)sscanf (lookahead, "%s", line);
    }
    denoms = atoi (line);
    return !denoms;
  }
  else
  {
    return 0;
  }
}

// this was parse_iir_coeff
/*TODO this should be int to error out the rest of the  processing */
static void
read_iir_coeff (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
                evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, check_fld, blkt_read, ncoeffs, ndenom;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = first_field == 3 ? 54 : 44;

  if (0 > parse_field (first_line, 0, field, log))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  // TODO - collesce below to compare string
  if (strlen (field) != 1)
  {
    evalresp_log (log, ERROR, 0,
                  "parse_coeff; parsing (IIR_COEFFS), illegal filter type ('%s')",
                  field);
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (*field == 'D')
  {
    blkt_ptr->type = IIR_COEFFS;
  }
  else
  {
    evalresp_log (log, ERROR, 0,
                  "parse_coeff; parsing (IIR_COEFFS), unexpected filter type ('%c')",
                  *field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  /* set the check-field counter to the next expected field */
  check_fld = first_field + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */
  if (check_fld == 4)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* the number of coefficients */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.coeff.nnumer = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.coeff.numer = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */

  check_fld += 2;

  /* the number of denominators */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ndenom = get_int (field, log);

  /* if the number of denominators is zero, then is not an IIR filter. */

  if (ndenom == 0)
  {
    // TODO - all these weird log formats that could have simpler strings
    evalresp_log (log, ERROR, 0, "%s%s",
                  "parse_coeff; This is not IIR filter , because number of denominators is zero!\n",
                  "\tshould be represented as blockette [53] filters");
    return /*TODO UNRECOG_FILTYPE should be returned */;
  }
  blkt_ptr->blkt_info.coeff.ndenom = ndenom;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.coeff.denom = alloc_double (ndenom, log);

  /* set the expected field to the current value (10 or 11 for [54] or [44])
     to the current value - 2 (8 or 9 for [54] or [44] respectively) */

  check_fld -= 2;

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld, 1, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_coeff: %s%s%s",
                    "numerators must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.coeff.numer[i] = atof (field);
  }

  check_fld += 3; /*IGD : we need field 11 now */

  for (i = 0; i < ndenom; i++)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld, 1, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_coeff: %s%s%s",
                    "denominators must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.coeff.denom[i] = atof (field);
  }
}

// this was "parse_coeff"
/*TODO this should be int to error out the rest of the  processing */
static void
read_coeff (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
            evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, check_fld, blkt_read, ncoeffs, ndenom;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = FirstField == 3 ? 54 : 44;

  if (0 > parse_field (first_line, 0, field, log))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, ERROR, 0,
                  "parse_coeff; parsing (FIR_ASYM), illegal filter type ('%s')",
                  field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  if (*field == 'D')
  {
    blkt_ptr->type = FIR_ASYM;
  }
  else
  {
    evalresp_log (log, ERROR, 0,
                  "parse_coeff; parsing (FIR_ASYM), unexpected filter type ('%c')",
                  *field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  /* set the check-field counter to the next expected field */
  check_fld = FirstField + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */
  if (check_fld == 4)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->output_units)
  {
    return; /* TODO */
  }

  /* the number of coefficients */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */

  check_fld += 2;

  /* the number of denominators */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ndenom = get_int (field, log);

  /* if the number of denominators is not zero, then is not a FIR filter.
     evalresp cannot evaluate IIR and Analog filters that are represented
     as numerators and denominators (and it is not recommended that these
     filter types be represented this way because of the loss of accuracy).
     These filter types should be represented as poles and zeros.  If one
     of these unsupported filters is detected, print error and exit. */

  if (ndenom)
  {
    evalresp_log (log, ERROR, 0, "%s%s",
                  "parse_coeff; Unsupported filter type, IIR and Analog filters\n",
                  "\tshould be represented as blockette [53] filters");
    return /*TODO UNRECOG_FILTYPE should be returned */;
  }

  /* set the expected field to the current value (10 or 11 for [54] or [44])
     to the current value - 2 (8 or 9 for [54] or [44] respectively) */

  check_fld -= 2;

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > find_field (log, seed, " ", blkt_read, check_fld, 1, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_coeff: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }
}

// this was "parse_list"
/*TODO this should be int to error out the rest of the  processing */
static void
read_list (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
           evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, blkt_read, check_fld, nresp, format;
  char field[MAXFLDLEN], line[MAXLINELEN];
  const char *lookahead;

  blkt_ptr->type = LIST;

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_list; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = FirstField == 3 ? 55 : 45;

  /* set the check-field counter to the next expected field */
  check_fld = FirstField;

  /* then, if first is a B055F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
    {
      return; /*TODO */
    }
  }
  else
  {
    strncpy (line, first_line, MAXLINELEN);
    check_fld++;
  }

  /* the units (in first, then out) */

  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* the number of responses */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  nresp = get_int (field, log);
  blkt_ptr->blkt_info.list.nresp = nresp;

  /* remember to allocate enough space for the number frequency, amplitude, phase tuples
     that follow */
  blkt_ptr->blkt_info.list.freq = alloc_double (nresp, log);
  blkt_ptr->blkt_info.list.amp = alloc_double (nresp, log);
  blkt_ptr->blkt_info.list.phase = alloc_double (nresp, log);

  /* then get the response information */

  if (blkt_read == 55)
  { /* This is blockette 55 */

    /*we now check if the B055F07-11 has a numbering field and set format accordingly */
    lookahead = *seed;
    if (0 > find_line (log, &lookahead, " ", blkt_read, check_fld, line))
    {
      return; /*TODO */
    }
    format = count_fields (line) - 5;

    /*format == 0 if no number of responses in the file */
    /*format == 1 if number of responses in the sec column*/

    if (format > 1) /*Wrong format */
    {
      evalresp_log (log, ERROR, 0, "parse_list: %s",
                    "Unknown format for B055F07-11");
      return /*TODO PARSE_ERROR should be returned */;
    }

    for (i = 0; i < nresp; i++)
    {
      if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
      {
        return; /*TODO */
      }
      if (0 > parse_field (line, format, field, log)) /* Frequency */
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if (0 > parse_field (line, 1 + format, field, log)) /* the amplitude of the Fourier transform */
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if (0 > parse_field (line, 3 + format, field, log)) /* Phase of the transform */
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }
  else
  { /* This is blockette 45 - leave at as in McSweeny's version */
    for (i = 0; i < nresp; i++)
    {
      if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
      {
        return; /*TODO */
      }
      if (0 > parse_field (line, 0, field, log))
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if (0 > parse_field (line, 1, field, log))
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if (0 > parse_field (line, 3, field, log))
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }
}

// this was "parse_generic"
static void
read_generic (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
              evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, blkt_read, check_fld, ncorners;
  char field[MAXFLDLEN], line[MAXLINELEN];

  blkt_ptr->type = GENERIC;

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_generic; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = first_field == 3 ? 56 : 46;

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B056F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
    {
      return; /*TODO */
    }
  }
  else
  {
    strncpy (line, FirstLine, MAXLINELEN);
    check_fld++;
  }

  /* next (from the file) should be the units (in first, then out) */

  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* the number of responses */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ncorners = get_int (field, log);
  blkt_ptr->blkt_info.generic.ncorners = ncorners;

  /* remember to allocate enough space for the number corner_frequency, corner_slope pairs
     that follow */

  blkt_ptr->blkt_info.generic.corner_freq = alloc_double (ncorners, log);
  blkt_ptr->blkt_info.generic.corner_slope = alloc_double (ncorners, log);

  /* then get the response information */

  for (i = 0; i < ncorners; i++)
  {
    if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
    {
      return; /*TODO */
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_generic: %s%s%s",
                    "corner_freqs must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.generic.corner_freq[i] = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_generic: %s%s%s",
                    "corner_slopes must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.generic.corner_slope[i] = atof (field);
  }
}

// this was "parse_deci"
static int
read_deci (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
           evalresp_blkt *blkt_ptr)
{
  int blkt_read, check_fld, sequence_no = 0;
  double srate;
  char field[MAXFLDLEN];

  blkt_ptr->type = DECIMATION;

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_deci; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return PARSE_ERROR;
  }

  blkt_read = first_field == 3 ? 57 : 47;
  check_fld = first_field;

  /* then, if first is a B057F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return PARSE_ERROR;
    }
    sequence_no = get_int (field, log);
    check_fld++;
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return PARSE_ERROR;
    }
  }
  else
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return PARSE_ERROR;
    }
    check_fld++;
  }

  /* next (from the file) input sample rate, convert to input sample interval */
  srate = get_double (field, log);
  if (srate)
  {
    blkt_ptr->blkt_info.decimation.sample_int = 1.0 / srate;
  }

  /* get the decimation factor and decimation offset */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.deci_fact = get_int (field, log);
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.deci_offset = get_int (field, log);

  /* the estimated delay */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.estim_delay = get_double (field, log);

  /* and, finally, the applied correction.  Note:  the calculated delay is left undefined
     by this routine, although space does exist in this filter type for this parameter */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.applied_corr = get_double (field, log);

  /* return the sequence number of the stage for verification */

  return (sequence_no);
}

// this was "parse_gain"
static int
read_gain (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
           evalresp_blkt *blkt_ptr)
{
  int i, blkt_read, check_fld, sequence_no = 0, nhist = 0;
  char field[MAXFLDLEN], line[MAXLINELEN];

  blkt_ptr->type = GAIN;

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_gain; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 of F05",
                  ", fld_found=F", FirstField);
    return PARSE_ERROR;
  }

  blkt_read = first_field == 3 ? 58 : 48;
  check_fld = first_field;

  /* then, if first is a B058F03, get the stage sequence number (from FirstLine)
     and the symmetry type (from the file), otherwise, get the symmetry (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return PARSE_ERROR;
    }
    sequence_no = get_int (field, log);
    check_fld++;
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return PARSE_ERROR;
    }
  }
  else
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return PARSE_ERROR;
    }
    check_fld++;
  }

  /* then get the gain and frequency of gain (these correspond to sensitivity and frequency of
     sensitivity for stage 0 Sensitivity/Gain filters) */

  blkt_ptr->blkt_info.gain.gain = get_double (field, log);
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.gain.gain_freq = get_double (field, log);

  /* if there is a history, skip it. First determine number of lines to skip */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return PARSE_ERROR;
  }
  nhist = get_int (field, log);

  /* then skip them  */

  for (i = 0; i < nhist; i++)
  {
    if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
    {
      return PARSE_ERROR; /*TODO */
    }
  }

  /* return the sequence number of the stage for verification */

  return (sequence_no);
}

// this was "parse_fir"
static void
read_fir (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
          evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, blkt_read, check_fld, ncoeffs;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_fir; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = first_field == 3 ? 61 : 41;
  check_fld = first_field;

  /* then, if first is a B061F03, get the stage sequence number (from FirstLine)
     and the symmetry type (from the file), otherwise, get the symmetry (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld += 2;
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
  }
  else
  {
    if (0 > parse_field (first_line, 0, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    check_fld++;
  }

  /* then get the symmetry type */

  if (strlen (field) != 1)
  {
    evalresp_log (log, ERROR, 0,
                  "parse_fir; parsing (FIR), illegal symmetry type ('%s')",
                  field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  switch (*field)
  {
  case 'A':
    blkt_ptr->type = FIR_ASYM; /* no symmetry */
    break;
  case 'B':
    blkt_ptr->type = FIR_SYM_1; /* odd number coefficients with symmetry */
    break;
  case 'C':
    blkt_ptr->type = FIR_SYM_2; /* even number coefficients with symmetry */
    break;
  default:
    evalresp_log (log, ERROR, 0,
                  "parse_fir; parsing (FIR), unexpected symmetry type ('%c')",
                  *field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  /* next should be the units (in first, then out) */
  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* the number of coefficients */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > find_field (log, seed, " ", blkt_read, check_fld, 1, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_fir: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }
}

// this was "parse_ref"
static void
read_ref (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
          evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int this_blkt_no = 60, blkt_no, fld_no, i, j, prev_blkt_no = 60;
  int nstages, stage_num, nresps, lcl_nstages;
  char field[MAXFLDLEN];
  evalresp_blkt *last_blkt;
  evalresp_stage *last_stage, *this_stage;

  blkt_ptr->type = REFERENCE;
  this_stage = stage_ptr;

  /* first get the number of stages (from the input line) */

  if (first_field != 3)
  {
    evalresp_log (log, ERROR, 0, "parse_ref; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (0 > parse_field (first_line, 0, field, log))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (!is_int (field, log))
  {
    evalresp_log (log, ERROR, 0, "parse_ref; value '%s' %s", field,
                  " cannot be converted to the number of stages");
    return /*TODO PARSE_ERROR should be returned */;
  }
  nstages = atoi (field);
  blkt_ptr->blkt_info.reference.num_stages = nstages;

  /* then (from the file) read all of the stages in sequence */

  for (i = 0; i < nstages; i++)
  {

    /* determine the stage number in the sequence of stages */
    if (0 > find_field (log, seed, ":", this_blkt_no, 4, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the stage sequence number");
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_num = atoi (field);
    blkt_ptr->blkt_info.reference.stage_num = stage_num;

    /* set the stage sequence number and the pointer to the first blockette */

    this_stage->sequence_no = stage_num;
    curr_seq_no = this_stage->sequence_no;

    /* then the number of responses in this stage */

    if (0 > find_field (log, seed, ":", this_blkt_no, 5, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, ERROR, 0, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the number of responses");
      return /*TODO PARSE_ERROR should be returned */;
    }
    nresps = atoi (field);
    blkt_ptr->blkt_info.reference.num_responses = nresps;

    /* then, for each of the responses in this stage, get the first line of the next
         response to determine the type of response to read */

    for (j = 0; j < nresps; j++)
    {
      first_field = read_line (log, seed, ":", &blkt_no, &fld_no, first_line);
      last_blkt = blkt_ptr;
      switch (blkt_no)
      {
      case 43:
        blkt_ptr = alloc_pz (log);
        read_pz (log, seed, first_field, first_line, blkt_ptr, this_stage);
        break;
      case 44:
        blkt_ptr = alloc_fir (log);
        read_coeff (log, seed, first_field, first_line, blkt_ptr, this_stage);
        break;
      case 45:
        blkt_ptr = alloc_list (log);
        read_list (log, seed, first_field, first_line, blkt_ptr, this_stage);
        break;
      case 46:
        blkt_ptr = alloc_generic (log);
        read_generic (log, seed, first_field, first_line, blkt_ptr, this_stage);
        break;
      case 47:
        blkt_ptr = alloc_deci (log);
        read_deci (log, seed, first_field, first_line, blkt_ptr);
        break;
      case 48:
        blkt_ptr = alloc_gain (log);
        read_gain (log, seed, first_field, first_line, blkt_ptr);
        break;
      case 41:
        blkt_ptr = alloc_fir (log);
        read_fir (log, seed, first_field, first_line, blkt_ptr, this_stage);
        break;
      case 60:
        evalresp_log (log, ERROR, 0,
                      "parse_ref; unexpected end of stage (at blockette [%3.3d])",
                      prev_blkt_no);
        return /*TODO PARSE_ERROR should be returned */;
      default:
        /* code to ignore unexected blockette/field lines might be useful here, but need
                 example code to test it - SBH. 2004.079
                 If we want it, replace this error_return and break with a "continue;"
                 */
        evalresp_log (log, ERROR, 0,
                      "parse_ref; unexpected filter type (blockette [%3.3d])",
                      blkt_no);
        return /*TODO UNRECOG_FILTYPE should be returned */;
      }
      last_blkt->next_blkt = blkt_ptr;
      prev_blkt_no = blkt_no;
    }

    if (i != (nstages - 1))
    { /* if not the last stage in this filter */

      /* set the last stage's next_stage to point to the new stage and the first_blkt
             for that stage to point to a new blockette [60] type filter */

      last_stage = this_stage;
      this_stage = alloc_stage (log);
      blkt_ptr = alloc_ref (log);
      last_stage->next_stage = this_stage;
      this_stage->first_blkt = blkt_ptr;

      /* set the filter type for the new blockette [60] filter stage */

      blkt_ptr->type = REFERENCE;

      /* and set the number of stages again ... */

      if (0 > find_field (log, seed, ":", this_blkt_no, 3, 0, field))
      {
        return /*TODO PARSE_ERROR should be returned */;
      }
      if (!is_int (field, log))
      {
        evalresp_log (log, ERROR, 0, "parse_ref; value '%s' %s", field,
                      " cannot be converted to the new stage sequence number");
        return /*TODO PARSE_ERROR should be returned */;
      }

      lcl_nstages = atoi (field);
      if (lcl_nstages != nstages)
      {
        evalresp_log (log, ERROR, 0,
                      "parse_ref; internal RESP format error, %s%d%s%d",
                      "\n\tstage expected = ", nstages, ", stage found = ",
                      lcl_nstages);
        return /*TODO PARSE_ERROR should be returned */;
      }
      blkt_ptr->blkt_info.reference.num_stages = nstages;
    }
  }
}

// this was "parse_polynomial"
static void
read_polynomial (evalresp_log_t *log, const char **seed, int first_field, char *first_line,
                 evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int i, blkt_read, check_fld, ncoeffs;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, ERROR, 0, "parse_polynomial; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return /*TODO PARSE_ERROR should be returned */;
  }

  blkt_read = first_field == 3 ? 62 : 42;

  if (0 > parse_field (first_line, 0, field, log))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  // TODO - coallesce into strcmp
  if (strlen (field) != 1)
  {
    evalresp_log (log, ERROR, 0,
                  "parse_polynomial; parsing (Polynomial), illegal filter type ('%s')",
                  field);
    return /*TODO PARSE_ERROR should be returned */;
  }
  if (*field == 'P')
  {
    blkt_ptr->type = POLYNOMIAL;
  }
  else
  {
    evalresp_log (log, ERROR, 0,
                  "parse_polynomial; parsing (Polynomial), unexpected filter type ('%c')",
                  *field);
    return /*TODO PARSE_ERROR should be returned */;
  }

  check_fld = first_field + 1;

  if (check_fld == 4)
  {
    if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->input_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  if (0 > find_line (log, seed, ":", blkt_read, check_fld++, line))
  {
    return; /*TODO */
  }
  stage_ptr->output_units = check_units (line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return; /* TODO */
  }

  /* Polynomial Approximation Type */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.approximation_type = field[0];

  /* Valid Frequency Units */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.frequency_units = field[0];

  /* Lower Valid Frequency Bound */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.lower_freq_bound = get_double (field, log);

  /* Upper Valid Frequency Bound */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.upper_freq_bound = get_double (field, log);

  /* Lower Bound of Approximation */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.lower_approx_bound = get_double (field, log);

  /* Upper Bound of Approximation */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.upper_approx_bound = get_double (field, log);

  /* Maximum Absolute Error */
  if (0 > find_field (log, seed, ":", blkt_read, check_fld++, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  blkt_ptr->blkt_info.polynomial.max_abs_error = get_double (field, log);

  /* the number of coefficients */

  if (0 > find_field (log, seed, ":", blkt_read, check_fld, 0, field))
  {
    return /*TODO PARSE_ERROR should be returned */;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.polynomial.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coeffs */

  blkt_ptr->blkt_info.polynomial.coeffs = calloc (ncoeffs, sizeof (double));
  blkt_ptr->blkt_info.polynomial.coeffs_err = calloc (ncoeffs, sizeof (double));

  check_fld += 1;

  /* get the coefficients and their errors */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > find_line (log, seed, " ", blkt_read, check_fld, line))
    {
      return; /*TODO */
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "polynomial: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.polynomial.coeffs[i] = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return /*TODO PARSE_ERROR should be returned */;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, ERROR, 0, "polynomial: %s%s%s",
                    "coeffs errors must be real numbers (found '", field, "')");
      return /*TODO PARSE_ERROR should be returned */;
    }
    blkt_ptr->blkt_info.polynomial.coeffs_err[i] = atof (field);
  }
}

// this was "parse_channel"
static int
read_channel_data (evalresp_log_t *log, const char **seed, evalresp_channel *chan)
{

  // TODO - assigments for no_units and tmp_stage2 made blindly to fix compiler warning.  bug?
  int blkt_no, read_blkt = 0, no_units = 0;
  int curr_seq_no, last_seq_no;
  evalresp_blkt *blkt_ptr, *last_blkt = NULL;
  evalresp_stage *this_stage, *last_stage, *tmp_stage, *tmp_stage2 = NULL;
  int first_field;
  char first_line[MAXLINELEN];

  /* initialize the channel's sequence of stages */

  last_stage = (evalresp_stage *)NULL;
  curr_seq_no = last_seq_no = 0;
  this_stage = alloc_stage (log);
  chan->first_stage = this_stage;
  chan->nstages++;
  tmp_stage = alloc_stage (log);

  /* start processing the response information */

  while ((read_line (log, seed, ":", &blkt_no, &first_field, first_line)) != 0 && blkt_no != 50)
  {
    switch (blkt_no)
    {
    case 53:
      blkt_ptr = alloc_pz (log);
      read_pz (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 54:
      /* IGD : as we add an IIR case to this blockette, we cannot simply assume that blockette 54 is FIR */
      /*The field 10 should be distinguish between the IIR and FIR */
      if (is_iir_coeffs (seed))
      { /*IGD New IIR case */
        blkt_ptr = alloc_coeff (log);
        read_iir_coeff (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      }
      else
      { /*IGD this is the original case here */
        blkt_ptr = alloc_fir (log);
        read_coeff (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      }
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 55:
      blkt_ptr = alloc_list (log);
      read_list (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 56:
      blkt_ptr = alloc_generic (log);
      read_generic (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 57:
      blkt_ptr = alloc_deci (log);
      curr_seq_no = read_deci (log, seed, first_field, first_line, blkt_ptr);
      break;
    case 58:
      blkt_ptr = alloc_gain (log);
      curr_seq_no = read_gain (log, seed, first_field, first_line, blkt_ptr);
      break;
    case 60: /* never see a blockette [41], [43]-[48] without a [60], parse_ref handles these */
      blkt_ptr = alloc_ref (log);
      tmp_stage2 = alloc_stage (log);
      read_ref (log, seed, first_field, first_line, blkt_ptr, tmp_stage2);
      curr_seq_no = tmp_stage2->sequence_no;
      tmp_stage2->first_blkt = blkt_ptr;
      break;
    case 61:
      blkt_ptr = alloc_fir (log);
      read_fir (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 62:
      blkt_ptr = alloc_polynomial (log);
      read_polynomial (log, seed, first_field, first_line, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    default:
      /*
             2004.079 - SBH changed to allow code to skip unrecognized lines in RESP file. Just continue
             to the next line.
             error_return(UNRECOG_FILTYPE, "parse_chan; unrecognized filter type (blockette [%c])",
             blkt_no);
             break;
             */
      continue;
    }
    if (blkt_no != 60)
    {
      if (!read_blkt++)
      {
        this_stage->first_blkt = blkt_ptr;
        this_stage->sequence_no = curr_seq_no;
        last_stage = this_stage;
        no_units = 1;
      }
      else if (last_seq_no != curr_seq_no)
      {
        chan->nstages++;
        last_stage = this_stage;
        this_stage = alloc_stage (log);
        this_stage->sequence_no = curr_seq_no;
        last_stage->next_stage = this_stage;
        this_stage->first_blkt = blkt_ptr;
        last_stage = this_stage;
        no_units = 1;
      }
      else
        last_blkt->next_blkt = blkt_ptr;

      if (no_units && blkt_no != 57 && blkt_no != 58)
      {
        this_stage->input_units = tmp_stage->input_units;
        this_stage->output_units = tmp_stage->output_units;
        no_units = 0;
      }

      last_blkt = blkt_ptr;
      last_seq_no = curr_seq_no;
    }
    else
    {
      if (!read_blkt++)
      {
        this_stage = tmp_stage2;
        free_stages (chan->first_stage);
        chan->first_stage = this_stage;
      }
      else if (last_seq_no != curr_seq_no)
      {
        this_stage = tmp_stage2;
        last_stage->next_stage = this_stage;
        chan->nstages++;
      }
      else
      {
        blkt_ptr = tmp_stage2->first_blkt;
        last_blkt->next_blkt = blkt_ptr;
        if (this_stage != (evalresp_stage *)NULL && tmp_stage2->next_stage != (evalresp_stage *)NULL)
        {
          this_stage->next_stage = tmp_stage2->next_stage;
        }
      }

      while (this_stage->next_stage != (evalresp_stage *)NULL)
      {
        this_stage = this_stage->next_stage;
        chan->nstages++;
      }
      blkt_ptr = this_stage->first_blkt;
      while (blkt_ptr->next_blkt != (evalresp_blkt *)NULL)
      {
        blkt_ptr = blkt_ptr->next_blkt;
      }
      last_blkt = blkt_ptr;
      last_stage = this_stage;
      curr_seq_no = this_stage->sequence_no;
      last_seq_no = curr_seq_no;
    }
  }
  free_stages (tmp_stage);
  return (FirstField);
}

// non-static only for testing
int
open_file (evalresp_log_t *log, const char *filename, FILE **in)
{
  int status = EVALRESP_OK;
  if (!(*in = fopen (filename, "r")))
  {
    evalresp_log (log, ERROR, ERROR, "Cannot open %s", filename);
    status = EVALRESP_IO;
  }
  return status;
}

// non-static only for testing
int
file_to_char (evalresp_log_t *log, FILE *in, char **seed)
{
  size_t buffer_len = 1024, used_len = 0, space;
  int status = EVALRESP_OK;

  *seed = NULL;
  if (!(*seed = malloc (buffer_len)))
  {
    evalresp_log (log, ERROR, ERROR, "Cannot allocate buffer memory");
    status = EVALRESP_MEM;
  }
  else
  {
    while (!status && !feof (in))
    {
      if (!(space = buffer_len - used_len))
      {
        buffer_len *= 2;
        if (!(*seed = realloc (*seed, buffer_len)))
        {
          evalresp_log (log, ERROR, ERROR, "Cannot reallocate buffer memory");
          status = EVALRESP_MEM;
        }
        space = buffer_len - used_len;
      }
      if (!status)
      {
        used_len += fread (*seed + used_len, 1, space, in);
        if (ferror (in))
        {
          evalresp_log (log, ERROR, ERROR, "Error reading input");
          status = EVALRESP_IO;
        }
      }
    }
  }
  if (!status && used_len == buffer_len)
  {
    if (!(*seed = realloc (*seed, ++buffer_len)))
    {
      evalresp_log (log, ERROR, ERROR, "Cannot reallocate buffer memory");
      status = EVALRESP_MEM;
    }
  }
  if (!status)
  {
    *(*seed + used_len) = '\0';
  }
  else
  {
    free (*seed);
    *seed = NULL;
  }
  return status;
}

int evalresp_char_to_channels (evalresp_log_t *log, const char *seed_or_xml,
                               const evalresp_filter *filter, evalresp_channels **channels)
{
  const char *seed_ptr = seed_or_xml;
  evalresp_channel channel;
  int status = EVALRESP_OK;

  status = read_channel_header(log, &seed_ptr, &channel);
  status = read_channel_data(log, &seed_ptr, &channel);

  return status;
}

int evalresp_file_to_channels (evalresp_log_t *log, FILE *file,
                               const evalresp_filter *filter, evalresp_channels **channels)
{
  char *seed = NULL;
  int status = EVALRESP_OK;
  if (!(status = file_to_char(log, file, &seed))) {
    status = evalresp_char_to_channels (log, seed, filter, channels);
  }
  free(seed);
  return status;
}

int
evalresp_filename_to_channels (evalresp_log_t *log, const char *filename,
                               const evalresp_filter *filter, evalresp_channels **channels)
{
  FILE *file = NULL;
  int status = EVALRESP_OK;
  if (!(status = open_file (log, filename, &file))) {
    status = evalresp_file_to_channels (log, file, filter, channels);
  }
  if (file) {
    fclose(file);
  }
  return status;
}
