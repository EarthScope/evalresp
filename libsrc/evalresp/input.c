
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./input.h"
#include "./private.h"
#include "./regexp.h" // TODO - should all private imports be relative like this?
#include "evalresp/constants.h"
#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp/dom_to_seed.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"

#ifdef _WIN32
// https://stackoverflow.com/questions/16647819/timegm-cross-platform
#define timegm _mkgmtime
#endif

// code from parse_fctns.c heavily refactored to (1) parse all lines and (2)
// read from strings rather than files.

static int
read_int (evalresp_logger *log, char *in_line, int *value)
{
  if (!is_int (in_line, log))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "read_int; '%s' is not an integer",
                  in_line);
    return EVALRESP_PAR;
  }
  else
  {
    *value = atoi (in_line);
    return EVALRESP_OK;
  }
}

static int
read_double (evalresp_logger *log, char *in_line, double *value)
{
  if (!is_real (in_line, log))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "read_double; '%s' is not a real number",
                  in_line);
    return EVALRESP_PAR;
  }
  else
  {
    *value = atof (in_line);
    return EVALRESP_OK;
  }
}

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

static int
read_pref (evalresp_logger *log, char *const line, int *blkt_no, int *fld_no)
{
  char fldstr[FLDSTRLEN], blktstr[BLKTSTRLEN];

  strncpy (fldstr, "", FLDSTRLEN);
  strncpy (blktstr, "", BLKTSTRLEN);
  if (*line != 'B' || strlen (line) < 7)
    return EVALRESP_PAR;

  strncpy (blktstr, (line + 1), 3);
  strncpy (fldstr, (line + 5), 2);
  *(blktstr + 3) = '\0';
  *(fldstr + 2) = '\0';

  if (!is_int (blktstr, log))
  {
    evalresp_log (log, EV_ERROR, 0, "parse_pref; prefix '%s' cannot be %s",
                  blktstr, "converted to a blockette number");
    return EVALRESP_PAR;
  }
  *blkt_no = atoi (blktstr);
  if (!is_int (fldstr, log))
  {
    evalresp_log (log, EV_ERROR, 0, "parse_pref; prefix '%s' cannot be %s",
                  fldstr, "converted to a blockette number");
    return EVALRESP_PAR;
  }
  *fld_no = atoi (fldstr);
  return EVALRESP_OK;
}

// this was "next_line"
static int
read_line (evalresp_logger *log, const char **seed, char *sep,
           int *blkt_no, int *fld_no, char *return_line)
{
  char *lcl_ptr, line[MAXLINELEN];
  int status;

  return_line[0] = '\0';

  drop_comments_and_blank_lines (seed);
  if (end_of_string (seed))
  {
    return EVALRESP_EOF;
  }

  slurp_line (seed, line, MAXLINELEN);
  remove_tabs_and_crlf (line);
  if ((status = read_pref (log, line, blkt_no, fld_no)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "unrecognised prefix: '%s'", line);
    return status;
  }

  if (!(lcl_ptr = strstr (line, sep)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "separator '%s' not found in '%s'", sep, line);
    return EVALRESP_PAR;
  }
  for (lcl_ptr++; *lcl_ptr && isspace (*lcl_ptr); lcl_ptr++)
    ;
  if (!*lcl_ptr)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "nothing to parse after '%s' in '%s'", sep, line);
    return EVALRESP_PAR;
  }
  strncpy (return_line, lcl_ptr, MAXLINELEN);
  return EVALRESP_OK;
}

// non-static only for testing
// this was "get_line"
int
find_line (evalresp_logger *log, const char **seed, char *sep, int blkt_no, int fld_no, char *return_line)
{
  int lcl_blkt, lcl_fld, status;
  char line[MAXLINELEN];
  const char *lookahead;

  while (!end_of_string (seed))
  {
    drop_comments_and_blank_lines (seed);
    if (end_of_string (seed))
      break;

    lookahead = *seed;
    slurp_line (&lookahead, line, MAXLINELEN);
    remove_tabs_and_crlf (line);
    if ((status = read_pref (log, line, &lcl_blkt, &lcl_fld)))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "unrecognised prefix: '%s'", line);
      return status;
    }
    else if (blkt_no == lcl_blkt && fld_no == lcl_fld)
    {
      return read_line (log, seed, sep, &lcl_blkt, &lcl_fld, return_line);
    }
    else
    {
      *seed = lookahead;
    }
  }

  return EVALRESP_EOF;
}

static int
number_of_fields (evalresp_logger *log, char *line, int *count)
{
  char *lcl_ptr, *new_ptr;
  char lcl_field[50];
  int nfields = 0, test;

  lcl_ptr = line;
  /* added test of 'strstr()' result -- 10/21/2005 -- [ET] */
  while (*lcl_ptr && (test = sscanf (lcl_ptr, "%s", lcl_field)) != 0 && (new_ptr = strstr (lcl_ptr, lcl_field)) != NULL)
  {
    lcl_ptr = new_ptr + strlen (lcl_field);
    nfields++;
  }
  *count = nfields;
  return EVALRESP_OK;
}

/* was parse_field */
static int
get_field_to_parse (evalresp_logger *log, char *line, int fld_no, char *return_field)
{
  char *lcl_ptr, *new_ptr;
  char lcl_field[MAXFLDLEN];
  int nfields, i;

  number_of_fields (log, line, &nfields);
  if (fld_no >= nfields)
  {
    if (nfields > 0)
    {
      evalresp_log (log, EV_ERROR, 0, "%s%d%s%d%s",
                    "parse_field; Input field number (", fld_no,
                    ") exceeds number of fields on line(", nfields, ")");
    }
    else
    {
      evalresp_log (log, EV_ERROR, 0, "%s",
                    "parse_field; Data fields not found on line");
    }
    return EVALRESP_PAR;
  }

  lcl_ptr = line;
  /* added test of 'strstr()' result -- 10/21/2005 -- [ET] */
  for (i = 0; i < fld_no; i++)
  {
    sscanf (lcl_ptr, "%s", lcl_field);
    if ((new_ptr = strstr (lcl_ptr, lcl_field)) == NULL)
      break;
    lcl_ptr = new_ptr + strlen (lcl_field);
  }

  sscanf (lcl_ptr, "%s", return_field);
  return EVALRESP_OK;
}

// non-static only for testing
int
find_field (evalresp_logger *log, const char **seed, char *sep,
            int blkt_no, int fld_no, int fld_wanted, char *return_field)
{
  int status = EVALRESP_OK;
  char line[MAXLINELEN];

  /* first get the next non-comment line */
  if (!(status = find_line (log, seed, sep, blkt_no, fld_no, line)))
  {
    /* then parse the field that the user wanted from the line find_line returned */
    status = get_field_to_parse (log, line, fld_wanted, return_field);
  }
  return status;
}

static int
find_int_field (evalresp_logger *log, const char **seed, char *sep,
                int blkt_no, int fld_no, int fld_wanted, int *value)
{
  int status = EVALRESP_OK;
  char field[MAXFLDLEN];

  if (!(status = find_field (log, seed, sep, blkt_no, fld_no, fld_wanted, field)))
  {
    status = read_int (log, field, value);
  }
  return status;
}

static int
find_double_field (evalresp_logger *log, const char **seed, char *sep,
                   int blkt_no, int fld_no, int fld_wanted, double *value)
{
  int status = EVALRESP_OK;
  char field[MAXFLDLEN];

  if (!(status = find_field (log, seed, sep, blkt_no, fld_no, fld_wanted, field)))
  {
    status = read_double (log, field, value);
  }
  return status;
}

static int
parse_int_field (evalresp_logger *log, char *line, int fld_wanted, int *value)
{
  int status = EVALRESP_OK;
  char field[MAXFLDLEN];

  if (!(status = get_field_to_parse (log, line, fld_wanted, field)))
  {
    status = read_int (log, field, value);
  }
  return status;
}

static int
reg_string_match (evalresp_logger *log, const char *string, char *expr, char *type_flag)
{
  char lcl_string[MAXLINELEN], regexp_pattern[MAXLINELEN];
  int i = 0, glob_type, test;
  register regexp *prog;
  register char *lcl_ptr;

  memset (lcl_string, 0, sizeof (lcl_string));
  memset (regexp_pattern, 0, sizeof (regexp_pattern));
  strncpy (lcl_string, string, strlen (string));
  lcl_ptr = expr;
  if (!strcmp (type_flag, "-r"))
    glob_type = 0;
  else if (!strcmp (type_flag, "-g"))
    glob_type = 1;
  else
  {
    evalresp_log (log, EV_ERROR, 0, " string_match; improper pattern type (%s)\n",
                  type_flag);
    return 0; /*TODO error improper pattern type */
  }
  while (*lcl_ptr && i < (MAXLINELEN - 1))
  {
    if (glob_type && *lcl_ptr == '?')
    {
      regexp_pattern[i++] = '.';
      lcl_ptr++;
    }
    else if (glob_type && *lcl_ptr == '*')
    {
      regexp_pattern[i++] = '.';
      regexp_pattern[i++] = '*';
      lcl_ptr++;
    }
    else
      regexp_pattern[i++] = *(lcl_ptr++);
  }
  regexp_pattern[i] = '\0';

  if ((prog = evr_regcomp (regexp_pattern, log)) == NULL)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "string_match; pattern '%s' didn't compile", regexp_pattern);
    return 0; /*TODO RE_COMP_FAILED*/
  }
  lcl_ptr = lcl_string;
  test = evr_regexec (prog, lcl_ptr, log);

  free (prog);
  return (test);
}

/* Parse unit strings.
 *
 * When requested units are not DEFAULT (units of the documented
 * response), parse the units to determine:
 * a) the scale factor needed to convert to the length unit to meters
 * and set channel->unit_scale_fact.
 * b) the appropriate value of DIS, VEL or ACC and set units.
 *
 * Some other units are detected, as exceptions to ones that can be
 * converted to DIS, VEL or ACC in meters.  These are left as
 * historical artifacts.
 *
 * previously named 'check_units'
 * */
static int
parse_units (evalresp_logger *log, evalresp_options const *const options, char *line, evalresp_channel *channel, int *units)
{
  int i, first_flag = 0, status = EVALRESP_OK;

  if (!strlen (channel->first_units))
  {
    first_flag = 1;
    strncpy (channel->first_units, line, MAXLINELEN);
    channel->unit_scale_fact = 1.0;
  }
  else
  {
    strncpy (channel->last_units, line, MAXLINELEN);
  }

  if (options && options->unit == evalresp_file_unit)
  {
    *units = DEFAULT;
    return status;
  }

  for (i = 0; i < (int)strlen (line); i++)
  {
    line[i] = toupper (line[i]);
  }

  /* IGD 02/03/01 a restricted case of pressure data is added
     * We will play with string_match ater if more requests show
     * up for pressure data.
     ********************************************/
  if (strncasecmp (line, "PA", 2) == 0)
  {
    *units = PRESSURE;
  }
  else if (strncasecmp (line, "KPA", 2) == 0)
  {
    *units = PRESSURE;
    channel->unit_scale_fact = 1.0;
  }
  /* IGD 08/21/06 Added support for TESLA */
  else if (strncasecmp (line, "T -", 3) == 0)
  {
    *units = TESLA;
  }
  else if (strncasecmp (line, "NT -", 3) == 0)
  {
    *units = TESLA;
    channel->unit_scale_fact = 1.0e9;
  }
  /* IHD 10/03/13 Adding DEGREES CENTIGRADE */
  else if (strncasecmp (line, "C -", 3) == 0)
  {
    *units = CENTIGRADE;
  }
  else if (reg_string_match (log, line, "^[CNM]?M/S\\*\\*2|^[CNM]?M/SEC\\*\\*2", "-r"))
  {
    if (first_flag && !strncmp ("NM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp ("MM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp ("CM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e2;
    *units = ACC;
  }
  else if (reg_string_match (log, line, "^[CNM]?M/S|^[CNM]?M/SEC", "-r"))
  {
    if (first_flag && !strncmp (line, "NM", 2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp (line, "MM", 2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp (line, "CM", 2))
      channel->unit_scale_fact = 1.0e2;
    *units = VEL;
  }
  else if (reg_string_match (log, line, "^[CNM]?M[^A-Z/]?", "-r"))
  {
    if (first_flag && !strncmp (line, "NM", 2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp (line, "MM", 2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp (line, "CM", 2))
      channel->unit_scale_fact = 1.0e2;
    *units = DIS;
  }
  else if (reg_string_match (log, line, "^COUNTS?[^A-Z]?", "-r") || reg_string_match (log, line, "^DIGITAL[^A-Z]?", "-r"))
  {
    *units = COUNTS;
  }
  else if (reg_string_match (log, line, "^V[^A-Z]?", "-r") || reg_string_match (log, line, "^VOLTS[^A-Z]?", "-r"))
  {
    *units = VOLTS;
  }
  else
  {
    *units = UNDEF_UNITS;
    evalresp_log (log, EV_WARN, EV_WARN,
                  "units found ('%s') cannot be converted to %s", line, evalresp_unit_string(options->unit));
    status = EVALRESP_PAR;
  }

  return status;
}

static int
read_units_first_line_known (evalresp_logger *log, evalresp_options const *const options, const char **seed, int blkt_read, int *check_fld,
                             char *line, evalresp_channel *channel, int *input_units, int *output_units,
                             char **input_units_str, char **output_units_str)
{
  int status = EVALRESP_OK;

  //*input_units = check_units (channel, line, log);
  if (input_units_str && line)
    *input_units_str = strdup(line);
  parse_units (log, options, line, channel, input_units);

  if (!(status = find_line (log, seed, ":", blkt_read, (*check_fld)++, line)))
  {
    //*output_units = check_units (channel, line, log);
    if (output_units_str && line)
      *output_units_str = strdup(line);
    parse_units (log, options, line, channel, output_units);
  }

  return status;
}

static int
read_units (evalresp_logger *log, evalresp_options const *const options, const char **seed, int blkt_read, int *check_fld,
            evalresp_channel *channel, int *input_units, int *output_units,
            char **input_units_str, char **output_units_str)
{
  int status = EVALRESP_OK;
  char line[MAXLINELEN];

  if (!(status = find_line (log, seed, ":", blkt_read, (*check_fld)++, line)))
  {
    status = read_units_first_line_known (log, options, seed, blkt_read, check_fld, line,
                                          channel, input_units, output_units,
                                          input_units_str, output_units_str);
  }

  return status;
}

// this was "read_channel"
static int
read_channel_header (evalresp_logger *log, const char **seed, char *first_line,
                     evalresp_channel *chan)
{
  int blkt_no, fld_no, status = EVALRESP_OK;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* check to make sure a non-comment field exists and it is the sta/chan/date info.
     Note:  If it is the first channel (and, as a result, first_line contains a null
     string), then we have to get the next line.  Otherwise, the first_line argument
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

  if (!strlen (first_line))
  {
    if ((status = find_field (log, seed, ":", 50, 3, 0, field)))
    {
      return status;
    }
  }
  else
  {
    if ((status = get_field_to_parse (log, first_line, 0, field)))
    {
      return status;
    }
  }

  strncpy (chan->staname, field, STALEN);

  /* then (from the file) the Network ID */

  if ((status = find_field (log, seed, ":", 50, 16, 0, field)))
  {
    return status;
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

  /* Modified to use 'next_line()' and 'get_field_to_parse()' directly
     to handle case where file contains "B052F03 Location:" and
     nothing afterward -- 10/19/2005 -- [ET] */
  if (!read_line (log, seed, ":", &blkt_no, &fld_no, line)) /* if data after "Location:" */
  {
    if ((status = get_field_to_parse (log, line, 0, field))) /* parse location data */
    {
      // TODO - log reason
      return status;
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
    if ((status = find_field (log, seed, ":", 52, 4, 0, field)))
    {
      // TODO - log reason
      return status;
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
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "read_channel_header; %s%s%3.3d%s%3.3d%s[%2.2d|%2.2d]%s%2.2d", "blkt",
                  " and fld numbers do not match expected values\n\tblkt_xpt=B",
                  52, ", blkt_found=B", blkt_no, "; fld_xpt=F", 3, 4,
                  ", fld_found=F", fld_no);
    return EVALRESP_PAR;
  }

  /* get the Start Date */
  if ((status = find_line (log, seed, ":", 52, 22, line)))
  {
    return status;
  }
  strncpy (chan->beg_t, line, DATIMLEN);

  /* get the End Date */
  if ((status = find_line (log, seed, ":", 52, 23, line)))
  {
    return status;
  }
  strncpy (chan->end_t, line, DATIMLEN);

  return status;
}

// this was parse_pz
static int
read_pz (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
         evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, check_fld, blkt_read, npoles, nzeros;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [53] the first field expected is a F03, if from a blockette [43], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_pz; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 53 : 43;

  if ((status = get_field_to_parse (log, first_line, 0, field)))
  {
    return status;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_pz; parsing (Poles & Zeros), illegal filter type ('%s')",
                  field);
    return EVALRESP_PAR;
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
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_pz; parsing (Poles & Zeros), unexpected filter type ('%c')",
                  *field);
    return EVALRESP_PAR;
  }

  /* set the check-field counter to the next expected field */
  check_fld = first_field + 1;

  /* then, if is a B053F04, get the stage sequence number (from the file) */

  if (check_fld == 4)
  {
    if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
  }

  if ((status = read_units (log, options, seed, blkt_read, &check_fld, channel,
                            &stage_ptr->input_units, &stage_ptr->output_units,
                            &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* then the A0 normalization factor */

  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.pole_zero.a0)))
  {
    return status;
  }

  /* the A0 normalization frequency */

  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.pole_zero.a0_freq)))
  {
    return status;
  }

  /* the number of zeros */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld, 0, &nzeros)))
  {
    return status;
  }
  blkt_ptr->blkt_info.pole_zero.nzeros = nzeros;

  /* remember to allocate enough space for the number of zeros to follow */

  blkt_ptr->blkt_info.pole_zero.zeros = alloc_complex (nzeros, log);

  /* set the expected field to the current value (9 or 10 for [53] or [43])
     to the current value + 5 (14 or 15 for [53] or [43] respectively) */

  check_fld += 5;

  /* the number of poles */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld, 0, &npoles)))
  {
    return status;
  }
  blkt_ptr->blkt_info.pole_zero.npoles = npoles;

  /* remember to allocate enough space for the number of poles to follow */

  blkt_ptr->blkt_info.pole_zero.poles = alloc_complex (npoles, log);

  /* set the expected field to the current value (14 or 15 for [53] or [43])
     to the current value - 4 (10 or 11 for [53] or [43] respectively) */

  check_fld -= 4;

  /* get the zeros */

  for (i = 0; i < nzeros; i++)
  {
    if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
    if ((status = get_field_to_parse (log, line, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].real = atof (field);
    if ((status = get_field_to_parse (log, line, 2, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].imag = atof (field);
  }

  /* set the expected field to the current value (10 or 11 for [53] or [43])
     to the current value + 5 (15 or 16 for [53] or [43] respectively) */

  check_fld += 5;

  /* and then get the poles */

  for (i = 0; i < npoles; i++)
  {
    if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
    if ((status = get_field_to_parse (log, line, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].real = atof (field);
    if ((status = get_field_to_parse (log, line, 2, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].imag = atof (field);
  }

  return status;
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
  /* Returns 1 if it is IIR; 0 if it is not IIR;                            */
  /* Tt returns 0  in case of the error, so use it with a caution!          */
  /* IGD I.Dricker ISTI i.dricker@isti.com 07/00 for evalresp 3.2.17        */
  /* IGD I.Dricker ISTI i.dricker@isti.com 07/17: Fully rewritten           */
  char *substr = NULL;
  char words[5][MAXLINELEN];
  int denoms = 0;

  substr = strstr (*seed, "B054F10");
  if (substr)
  {

    /* Parsing (B054F10     Number of denominators:                0) */
    sscanf (substr, "%s %s %s %s %s", words[0], words[1], words[2], words[3], words[4]);
    denoms = atoi (words[4]);
  }
  if (0 == denoms)
    return 0;
  return 1;
}

// this was parse_iir_coeff
static int
read_iir_coeff (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
                evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, check_fld, blkt_read, ncoeffs, ndenom;
  char field[MAXFLDLEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 54 : 44;

  if ((status = get_field_to_parse (log, first_line, 0, field)))
  {
    return status;
  }
  if (!strcmp (field, "D"))
  {
    blkt_ptr->type = IIR_COEFFS;
  }
  else
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_coeff; parsing (IIR_COEFFS), unexpected filter type ('%s')",
                  field);
    return EVALRESP_PAR;
  }

  /* set the check-field counter to the next expected field */
  check_fld = first_field + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */
  if (check_fld == 4)
  {
    if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
  }

  if ((status = read_units (log, options, seed, blkt_read, &check_fld, channel,
                            &stage_ptr->input_units, &stage_ptr->output_units,
                            &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* the number of coefficients */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &ncoeffs)))
  {
    return status;
  }
  blkt_ptr->blkt_info.coeff.nnumer = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */
  blkt_ptr->blkt_info.coeff.numer = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */
  check_fld += 2;

  /* the number of denominators */
  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld, 0, &ndenom)))
  {
    return status;
  }

  /* if the number of denominators is zero, then is not an IIR filter. */

  if (ndenom == 0)
  {
    // TODO - all these weird log formats that could have simpler strings
    evalresp_log (log, EV_ERROR, EV_ERROR, "%s%s",
                  "parse_coeff; This is not IIR filter , because number of denominators is zero!\n",
                  "\tshould be represented as blockette [53] filters");
    return EVALRESP_PAR;
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
    if ((status = find_field (log, seed, " ", blkt_read, check_fld, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_coeff: %s%s%s",
                    "numerators must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.coeff.numer[i] = atof (field);
  }

  check_fld += 3;

  for (i = 0; i < ndenom; i++)
  {
    if ((status = find_field (log, seed, " ", blkt_read, check_fld, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_coeff: %s%s%s",
                    "denominators must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.coeff.denom[i] = atof (field);
  }

  return status;
}

// this was "parse_coeff"
static int
read_coeff (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
            evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, check_fld, blkt_read, ncoeffs, ndenom;
  char field[MAXFLDLEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 54 : 44;

  if ((status = get_field_to_parse (log, first_line, 0, field)))
  {
    return status;
  }
  if (!strcmp (field, "D"))
  {
    blkt_ptr->type = FIR_ASYM;
  }
  else
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_coeff; parsing (FIR_ASYM), unexpected filter type ('%s')",
                  field);
    return EVALRESP_PAR;
  }

  /* set the check-field counter to the next expected field */
  check_fld = first_field + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */
  if (check_fld == 4)
  {
    if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
  }

  if ((status = read_units (log, options, seed, blkt_read, &check_fld, channel,
                            &stage_ptr->input_units, &stage_ptr->output_units,
                            &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* the number of coefficients */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &ncoeffs)))
  {
    return status;
  }
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */

  check_fld += 2;

  /* the number of denominators */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld, 0, &ndenom)))
  {
    return status;
  }

  /* if the number of denominators is not zero, then is not a FIR filter.
     evalresp cannot evaluate IIR and Analog filters that are represented
     as numerators and denominators (and it is not recommended that these
     filter types be represented this way because of the loss of accuracy).
     These filter types should be represented as poles and zeros.  If one
     of these unsupported filters is detected, print error and exit. */

  if (ndenom)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "%s%s",
                  "parse_coeff; Unsupported filter type, IIR and Analog filters\n",
                  "\tshould be represented as blockette [53] filters");
    return EVALRESP_PAR;
  }

  /* set the expected field to the current value (10 or 11 for [54] or [44])
     to the current value - 2 (8 or 9 for [54] or [44] respectively) */

  check_fld -= 2;

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if ((status = find_field (log, seed, " ", blkt_read, check_fld, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_coeff: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }

  return status;
}

// this was "parse_list"
static int
read_list (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
           evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, blkt_read, check_fld, nresp, format;
  char field[MAXFLDLEN], line[MAXLINELEN];
  const char *lookahead;

  blkt_ptr->type = LIST;

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 55 : 45;

  /* set the check-field counter to the next expected field */
  check_fld = first_field;

  /* then, if first is a B055F03, get the stage sequence number (from first_line)
     and the input units (from the file), otherwise, get the input units (from first_line) */

  if (check_fld == 3)
  {
    if ((status = parse_int_field (log, first_line, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if ((status = find_line (log, seed, ":", blkt_read, check_fld++, line)))
    {
      return status;
    }
  }
  else
  {
    strncpy (line, first_line, MAXLINELEN);
    check_fld++;
  }

  if ((status = read_units_first_line_known (log, options, seed, blkt_read, &check_fld,
                                             line, channel,
                                             &stage_ptr->input_units, &stage_ptr->output_units,
                                             &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* the number of responses */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &nresp)))
  {
    return status;
  }
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
    if ((status = find_line (log, &lookahead, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
    number_of_fields (log, line, &format);
    format -= 5;

    /*format == 0 if no number of responses in the file */
    /*format == 1 if number of responses in the sec column*/

    if (format > 1) /*Wrong format */
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s",
                    "Unknown format for B055F07-11");
      return EVALRESP_PAR;
    }

    for (i = 0; i < nresp; i++)
    {
      if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
      {
        return status;
      }
      if ((status = get_field_to_parse (log, line, format, field))) /* Frequency */
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if ((status = get_field_to_parse (log, line, 1 + format, field))) /* the amplitude of the Fourier transform */
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if ((status = get_field_to_parse (log, line, 3 + format, field))) /* Phase of the transform */
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }
  else
  { /* This is blockette 45 - leave at as in McSweeny's version */
    for (i = 0; i < nresp; i++)
    {
      if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
      {
        return status;
      }
      if ((status = get_field_to_parse (log, line, 0, field)))
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if ((status = get_field_to_parse (log, line, 1, field)))
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if ((status = get_field_to_parse (log, line, 3, field)))
      {
        return status;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }

  return status;
}

// this was "parse_generic"
static int
read_generic (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
              evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, blkt_read, check_fld, ncorners;
  char field[MAXFLDLEN], line[MAXLINELEN];

  blkt_ptr->type = GENERIC;

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_generic; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 56 : 46;

  /* set the check-field counter to the next expected field */

  check_fld = first_field;

  /* then, if first is a B056F03, get the stage sequence number (from first_line)
     and the input units (from the file), otherwise, get the input units (from first_line) */

  if (check_fld == 3)
  {
    if ((status = parse_int_field (log, first_line, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if ((status = find_line (log, seed, ":", blkt_read, check_fld++, line)))
    {
      return status;
    }
  }
  else
  {
    strncpy (line, first_line, MAXLINELEN);
    check_fld++;
  }

  if ((status = read_units_first_line_known (log, options, seed, blkt_read, &check_fld,
                                             line, channel,
                                             &stage_ptr->input_units, &stage_ptr->output_units,
                                             &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* the number of responses */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &ncorners)))
  {
    return status;
  }
  blkt_ptr->blkt_info.generic.ncorners = ncorners;

  /* remember to allocate enough space for the number corner_frequency, corner_slope pairs
     that follow */

  blkt_ptr->blkt_info.generic.corner_freq = alloc_double (ncorners, log);
  blkt_ptr->blkt_info.generic.corner_slope = alloc_double (ncorners, log);

  /* then get the response information */

  for (i = 0; i < ncorners; i++)
  {
    if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
    if ((status = get_field_to_parse (log, line, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_generic: %s%s%s",
                    "corner_freqs must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.generic.corner_freq[i] = atof (field);
    if ((status = get_field_to_parse (log, line, 2, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_generic: %s%s%s",
                    "corner_slopes must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.generic.corner_slope[i] = atof (field);
  }

  return status;
}

// this was "parse_deci"
static int
read_deci (evalresp_logger *log, const char **seed, int first_field, char *first_line,
           evalresp_blkt *blkt_ptr, int *sequence_no)
{
  int status = EVALRESP_OK, blkt_read, check_fld;
  double srate;
  char field[MAXFLDLEN];

  blkt_ptr->type = DECIMATION;

  if (sequence_no)
  {
    *sequence_no = 0;
  }

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_deci; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 57 : 47;
  check_fld = first_field;

  /* then, if first is a B057F03, get the stage sequence number (from first_line)
     and the input units (from the file), otherwise, get the input units (from first_line) */

  if (check_fld == 3)
  {
    if (sequence_no && (status = parse_int_field (log, first_line, 0, sequence_no)))
    {
      return status;
    }
    check_fld++;
    if ((status = find_field (log, seed, ":", blkt_read, check_fld++, 0, field)))
    {
      return status;
    }
  }
  else
  {
    if ((status = get_field_to_parse (log, first_line, 0, field)))
    {
      return status;
    }
    check_fld++;
  }

  /* next (from the file) input sample rate, convert to input sample interval */
  if ((status = read_double (log, field, &srate)))
  {
    return status;
  }
  else if (srate)
  {
    blkt_ptr->blkt_info.decimation.sample_int = 1.0 / srate;
  }

  /* get the decimation factor and decimation offset */
  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0,
                                &blkt_ptr->blkt_info.decimation.deci_fact)))
  {
    return status;
  }
  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0,
                                &blkt_ptr->blkt_info.decimation.deci_offset)))
  {
    return status;
  }

  /* the estimated delay */

  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.decimation.estim_delay)))
  {
    return status;
  }

  /* and, finally, the applied correction.  Note:  the calculated delay is left undefined
     by this routine, although space does exist in this filter type for this parameter */

  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.decimation.applied_corr)))
  {
    return status;
  }

  return status;
}

// this was "parse_gain"
static int
read_gain (evalresp_logger *log, const char **seed, int first_field, char *first_line,
           evalresp_blkt *blkt_ptr, int *sequence_no)
{
  int status = EVALRESP_OK, i, blkt_read, check_fld, nhist = 0;
  char field[MAXFLDLEN], line[MAXLINELEN];

  blkt_ptr->type = GAIN;
  if (sequence_no)
  {
    *sequence_no = 0;
  }

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_gain; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 of F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 58 : 48;
  check_fld = first_field;

  /* then, if first is a B058F03, get the stage sequence number (from first_line)
     and the symmetry type (from the file), otherwise, get the symmetry (from first_line) */

  if (check_fld == 3)
  {
    if (sequence_no && (status = parse_int_field (log, first_line, 0, sequence_no)))
    {
      return status;
    }
    check_fld++;
    if ((status = find_field (log, seed, ":", blkt_read, check_fld++, 0, field)))
    {
      return status;
    }
  }
  else
  {
    if ((status = get_field_to_parse (log, first_line, 0, field)))
    {
      return status;
    }
    check_fld++;
  }

  /* then get the gain and frequency of gain (these correspond to sensitivity and frequency of
     sensitivity for stage 0 Sensitivity/Gain filters) */

  if ((status = read_double (log, field, &blkt_ptr->blkt_info.gain.gain)))
  {
    return status;
  }

  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.gain.gain_freq)))
  {
    return status;
  }

  /* if there is a history, skip it. First determine number of lines to skip */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &nhist)))
  {
    return status;
  }

  /* then skip them  */

  for (i = 0; i < nhist; i++)
  {
    if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
  }

  /* return the sequence number of the stage for verification */

  return status;
}

// this was "parse_fir"
static int
read_fir (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
          evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, blkt_read, check_fld, ncoeffs;
  char field[MAXFLDLEN];

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_fir; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 61 : 41;
  check_fld = first_field;

  /* then, if first is a B061F03, get the stage sequence number (from first_line)
     and the symmetry type (from the file), otherwise, get the symmetry (from first_line) */

  if (check_fld == 3)
  {
    if ((status = parse_int_field (log, first_line, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
    check_fld += 2;
    if ((status = find_field (log, seed, ":", blkt_read, check_fld++, 0, field)))
    {
      return status;
    }
  }
  else
  {
    if ((status = get_field_to_parse (log, first_line, 0, field)))
    {
      return status;
    }
    check_fld++;
  }

  /* then get the symmetry type */

  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_fir; parsing (FIR), illegal symmetry type ('%s')",
                  field);
    return EVALRESP_PAR;
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
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_fir; parsing (FIR), unexpected symmetry type ('%c')",
                  *field);
    return EVALRESP_PAR;
  }

  if ((status = read_units (log, options, seed, blkt_read, &check_fld, channel,
                            &stage_ptr->input_units, &stage_ptr->output_units,
                            &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* the number of coefficients */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &ncoeffs)))
  {
    return status;
  }
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if ((status = find_field (log, seed, " ", blkt_read, check_fld, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_fir: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }

  return status;
}

// this was "parse_ref"
static int
read_ref (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
          evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, this_blkt_no = 60, blkt_no, fld_no, i, j, prev_blkt_no = 60;
  int nstages, stage_num, nresps, lcl_nstages;
  char field[MAXFLDLEN];
  evalresp_blkt *last_blkt;
  evalresp_stage *last_stage, *this_stage;

  blkt_ptr->type = REFERENCE;
  this_stage = stage_ptr;

  /* first get the number of stages (from the input line) */

  if (first_field != 3)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_ref; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }
  if ((status = get_field_to_parse (log, first_line, 0, field)))
  {
    return status;
  }
  if (!is_int (field, log))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_ref; value '%s' %s", field,
                  " cannot be converted to the number of stages");
    return EVALRESP_PAR;
  }
  nstages = atoi (field);
  blkt_ptr->blkt_info.reference.num_stages = nstages;

  /* then (from the file) read all of the stages in sequence */

  for (i = 0; i < nstages; i++)
  {

    /* determine the stage number in the sequence of stages */
    if ((status = find_field (log, seed, ":", this_blkt_no, 4, 0, field)))
    {
      return status;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the stage sequence number");
      return EVALRESP_PAR;
    }
    stage_num = atoi (field);
    blkt_ptr->blkt_info.reference.stage_num = stage_num;

    /* set the stage sequence number and the pointer to the first blockette */

    this_stage->sequence_no = stage_num;
    //curr_seq_no = this_stage->sequence_no;

    /* then the number of responses in this stage */

    if ((status = find_field (log, seed, ":", this_blkt_no, 5, 0, field)))
    {
      return status;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the number of responses");
      return EVALRESP_PAR;
    }
    nresps = atoi (field);
    blkt_ptr->blkt_info.reference.num_responses = nresps;

    /* then, for each of the responses in this stage, get the first line of the next
         response to determine the type of response to read */

    for (j = 0; j < nresps; j++)
    {
      if (!(status = read_line (log, seed, ":", &blkt_no, &fld_no, first_line)))
      {
        last_blkt = blkt_ptr;
        switch (blkt_no)
        {
        case 43:
          blkt_ptr = alloc_pz (log);
          status = read_pz (log, options, seed, fld_no, first_line, channel, blkt_ptr, this_stage);
          break;
        case 44:
          blkt_ptr = alloc_fir (log);
          status = read_coeff (log, options, seed, fld_no, first_line, channel, blkt_ptr, this_stage);
          break;
        case 45:
          blkt_ptr = alloc_list (log);
          status = read_list (log, options, seed, fld_no, first_line, channel, blkt_ptr, this_stage);
          break;
        case 46:
          blkt_ptr = alloc_generic (log);
          status = read_generic (log, options, seed, fld_no, first_line, channel, blkt_ptr, this_stage);
          break;
        case 47:
          blkt_ptr = alloc_deci (log);
          status = read_deci (log, seed, fld_no, first_line, blkt_ptr, NULL);
          break;
        case 48:
          blkt_ptr = alloc_gain (log);
          status = read_gain (log, seed, fld_no, first_line, blkt_ptr, NULL);
          break;
        case 41:
          blkt_ptr = alloc_fir (log);
          status = read_fir (log, options, seed, fld_no, first_line, channel, blkt_ptr, this_stage);
          break;
        case 60:
          evalresp_log (log, EV_ERROR, EV_ERROR,
                        "parse_ref; unexpected end of stage (at blockette [%3.3d])",
                        prev_blkt_no);
          status = EVALRESP_PAR;
          break;
        default:
          /* code to ignore unexected blockette/field lines might be useful here, but need
                   example code to test it - SBH. 2004.079
                   If we want it, replace this error_return and break with a "continue;"
                   */
          evalresp_log (log, EV_ERROR, EV_ERROR,
                        "parse_ref; unexpected filter type (blockette [%3.3d])",
                        blkt_no);
          status = EVALRESP_PAR;
        }
      }
      if (status)
      {
        return status;
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

      if ((status = find_field (log, seed, ":", this_blkt_no, 3, 0, field)))
      {
        return status;
      }
      if (!is_int (field, log))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "parse_ref; value '%s' %s", field,
                      " cannot be converted to the new stage sequence number");
        return EVALRESP_PAR;
      }

      lcl_nstages = atoi (field);
      if (lcl_nstages != nstages)
      {
        evalresp_log (log, EV_ERROR, EV_ERROR,
                      "parse_ref; internal RESP format error, %s%d%s%d",
                      "\n\tstage expected = ", nstages, ", stage found = ",
                      lcl_nstages);
        return EVALRESP_PAR;
      }
      blkt_ptr->blkt_info.reference.num_stages = nstages;
    }
  }

  return status;
}

// this was "parse_polynomial"
static int
read_polynomial (evalresp_logger *log, evalresp_options const *const options, const char **seed, int first_field, char *first_line,
                 evalresp_channel *channel, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr)
{
  int status = EVALRESP_OK, i, blkt_read, check_fld, ncoeffs;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the stage sequence number (from the input line) */

  if (first_field != 3 && first_field != 5)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "parse_polynomial; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", first_field);
    return EVALRESP_PAR;
  }

  blkt_read = first_field == 3 ? 62 : 42;

  if ((status = get_field_to_parse (log, first_line, 0, field)))
  {
    return status;
  }
  if (!strcmp (field, "P"))
  {
    blkt_ptr->type = POLYNOMIAL;
  }
  else
  {
    evalresp_log (log, EV_ERROR, EV_ERROR,
                  "parse_polynomial; parsing (Polynomial), unexpected filter type ('%s')",
                  field);
    return EVALRESP_PAR;
  }

  check_fld = first_field + 1;

  if (check_fld == 4)
  {
    if ((status = find_int_field (log, seed, ":", blkt_read, check_fld++, 0, &stage_ptr->sequence_no)))
    {
      return status;
    }
    //curr_seq_no = stage_ptr->sequence_no;
  }

  if ((status = read_units (log, options, seed, blkt_read, &check_fld, channel,
                            &stage_ptr->input_units, &stage_ptr->output_units,
                            &stage_ptr->input_units_str, &stage_ptr->output_units_str)))
  {
    return status;
  }

  /* Polynomial Approximation Type */
  if ((status = find_field (log, seed, ":", blkt_read, check_fld++, 0, field)))
  {
    return status;
  }
  blkt_ptr->blkt_info.polynomial.approximation_type = field[0];

  /* Valid Frequency Units */
  if ((status = find_field (log, seed, ":", blkt_read, check_fld++, 0, field)))
  {
    return status;
  }
  blkt_ptr->blkt_info.polynomial.frequency_units = field[0];

  /* Lower Valid Frequency Bound */
  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.polynomial.lower_freq_bound)))
  {
    return status;
  }

  /* Upper Valid Frequency Bound */
  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.polynomial.upper_freq_bound)))
  {
    return status;
  }

  /* Lower Bound of Approximation */
  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.polynomial.lower_approx_bound)))
  {
    return status;
  }

  /* Upper Bound of Approximation */
  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.polynomial.upper_approx_bound)))
  {
    return status;
  }

  /* Maximum Absolute Error */
  if ((status = find_double_field (log, seed, ":", blkt_read, check_fld++, 0,
                                   &blkt_ptr->blkt_info.polynomial.max_abs_error)))
  {
    return status;
  }

  /* the number of coefficients */

  if ((status = find_int_field (log, seed, ":", blkt_read, check_fld, 0, &ncoeffs)))
  {
    return status;
  }
  blkt_ptr->blkt_info.polynomial.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coeffs */

  blkt_ptr->blkt_info.polynomial.coeffs = calloc (ncoeffs, sizeof (double));
  blkt_ptr->blkt_info.polynomial.coeffs_err = calloc (ncoeffs, sizeof (double));

  check_fld += 1;

  /* get the coefficients and their errors */

  for (i = 0; i < ncoeffs; i++)
  {
    if ((status = find_line (log, seed, " ", blkt_read, check_fld, line)))
    {
      return status;
    }
    if ((status = get_field_to_parse (log, line, 1, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "polynomial: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.polynomial.coeffs[i] = atof (field);
    if ((status = get_field_to_parse (log, line, 2, field)))
    {
      return status;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "polynomial: %s%s%s",
                    "coeffs errors must be real numbers (found '", field, "')");
      return EVALRESP_PAR;
    }
    blkt_ptr->blkt_info.polynomial.coeffs_err[i] = atof (field);
  }

  return status;
}

// this was "parse_channel"
static int
read_channel_data (evalresp_logger *log, evalresp_options const *const options, const char **seed, char *first_line,
                   evalresp_channel *channel)
{

  int status = EVALRESP_OK, blkt_no, read_blkt = 0, no_units = 0;
  int curr_seq_no, last_seq_no;
  evalresp_blkt *blkt_ptr, *last_blkt = NULL;
  evalresp_stage *this_stage, *last_stage, *tmp_stage, *tmp_stage2 = NULL;
  int first_field;

  /* initialize the channel's sequence of stages */

  last_stage = (evalresp_stage *)NULL;
  curr_seq_no = last_seq_no = 0;
  this_stage = alloc_stage (log);
  channel->first_stage = this_stage;
  channel->nstages++;
  tmp_stage = alloc_stage (log);

  /* start processing the response information */

  while (!(status = read_line (log, seed, ":", &blkt_no, &first_field, first_line)) && blkt_no != 50)
  {
    tmp_stage->input_units_str = NULL;
    tmp_stage->output_units_str = NULL;

    switch (blkt_no)
    {
    case 53:
      blkt_ptr = alloc_pz (log);
      status = read_pz (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 54:
      /* IGD : as we add an IIR case to this blockette, we cannot simply assume that blockette 54 is FIR */
      /*The field 10 should be distinguish between the IIR and FIR */
      if (is_iir_coeffs (seed))
      {
        blkt_ptr = alloc_coeff (log);
        status = read_iir_coeff (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      }
      else
      {
        blkt_ptr = alloc_fir (log);
        status = read_coeff (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      }
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 55:
      blkt_ptr = alloc_list (log);
      status = read_list (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 56:
      blkt_ptr = alloc_generic (log);
      status = read_generic (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 57:
      blkt_ptr = alloc_deci (log);
      status = read_deci (log, seed, first_field, first_line, blkt_ptr, &curr_seq_no);
      break;
    case 58:
      blkt_ptr = alloc_gain (log);
      status = read_gain (log, seed, first_field, first_line, blkt_ptr, &curr_seq_no);
      break;
    case 60: /* never see a blockette [41], [43]-[48] without a [60], parse_ref handles these */
      blkt_ptr = alloc_ref (log);
      tmp_stage2 = alloc_stage (log);
      status = read_ref (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage2);
      curr_seq_no = tmp_stage2->sequence_no;
      tmp_stage2->first_blkt = blkt_ptr;
      break;
    case 61:
      blkt_ptr = alloc_fir (log);
      status = read_fir (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 62:
      blkt_ptr = alloc_polynomial (log);
      status = read_polynomial (log, options, seed, first_field, first_line, channel, blkt_ptr, tmp_stage);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    default:
      /* 2004.079 - SBH changed to allow code to skip unrecognized lines in RESP file. Just continue
       * to the next line. */
      continue;
    }

    if (status && status != EVALRESP_EOF)
    {
      return status;
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
        channel->nstages++;
        last_stage = this_stage;
        this_stage = alloc_stage (log);
        this_stage->sequence_no = curr_seq_no;
        last_stage->next_stage = this_stage;
        this_stage->first_blkt = blkt_ptr;
        last_stage = this_stage;
        no_units = 1;
      }
      else
      {
        last_blkt->next_blkt = blkt_ptr;
      }

      if (no_units && blkt_no != 57 && blkt_no != 58)
      {
        this_stage->input_units = tmp_stage->input_units;
        this_stage->output_units = tmp_stage->output_units;
        this_stage->input_units_str = tmp_stage->input_units_str;
        this_stage->output_units_str = tmp_stage->output_units_str;
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
        free_stages (channel->first_stage);
        channel->first_stage = this_stage;
      }
      else if (last_seq_no != curr_seq_no)
      {
        this_stage = tmp_stage2;
        last_stage->next_stage = this_stage;
        channel->nstages++;
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
        channel->nstages++;
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

  return (status && status != EVALRESP_EOF) ? status : (first_field ? EVALRESP_OK : EVALRESP_PAR);
}

// non-static only for testing
int
open_file (evalresp_logger *log, const char *filename, FILE **in)
{
  int status = EVALRESP_OK;
  if (!(*in = fopen (filename, "r")))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot open %s", filename);
    status = EVALRESP_IO;
  }
  return status;
}

// non-static only for testing
int
file_to_char (evalresp_logger *log, FILE *in, char **seed)
{
  size_t buffer_len = 1024, used_len = 0, space;
  int status = EVALRESP_OK;

  *seed = NULL;
  if (!(*seed = malloc (buffer_len)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate buffer memory");
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
          evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot reallocate buffer memory");
          status = EVALRESP_MEM;
        }
        space = buffer_len - used_len;
      }
      if (!status)
      {
        used_len += fread (*seed + used_len, 1, space, in);
        if (ferror (in))
        {
          evalresp_log (log, EV_ERROR, EV_ERROR, "Error reading input");
          status = EVALRESP_IO;
        }
      }
    }
  }
  // TODO - avoid this extra byte alloc by reading less above?
  if (!status && used_len == buffer_len)
  {
    if (!(*seed = realloc (*seed, ++buffer_len)))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot reallocate buffer memory");
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

static int
add_channel (evalresp_logger *log, evalresp_channel *channel, evalresp_channels *channels)
{
  int status = EVALRESP_OK;
  channels->nchannels++;
  if (!(channels->channels = realloc (channels->channels,
                                      sizeof (evalresp_channel *) * channels->nchannels)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot reallocate channels memory");
    status = EVALRESP_MEM;
  }
  else
  {
    channels->channels[channels->nchannels - 1] = channel;
  }
  return status;
}

// was part of in_epoch
static void
parse_datetime (const char *str, evalresp_datetime *datetime)
{
  char *start_pos, temp_str[DATIMLEN];
  int len;

  // TODO - this could probably be rewritten better (like, error handling, what's that?)
  datetime->hour = datetime->min = 0;
  datetime->sec = 0.0;
  strncpy (temp_str, str, DATIMLEN);
  start_pos = temp_str;
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  datetime->year = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  datetime->jday = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  if (strlen (start_pos))
  {
    len = strcspn (start_pos, ":");
    *(start_pos + len) = '\0';
    datetime->hour = atoi (start_pos);
    start_pos += (strlen (start_pos) + 1);
    if (strlen (start_pos))
    {
      len = strcspn (start_pos, ":");
      *(start_pos + len) = '\0';
      datetime->min = atoi (start_pos);
      start_pos += (strlen (start_pos) + 1);
      if (strlen (start_pos))
      {
        datetime->sec = atof (start_pos);
      }
    }
  }
}

// non-static only for tests
int
timecmp (evalresp_datetime *dt1, evalresp_datetime *dt2)
{

  /* check year */
  if (dt1->year < dt2->year)
    return (-1);
  if (dt1->year > dt2->year)
    return (1);

  /* check day */
  if (dt1->jday < dt2->jday)
    return (-1);
  if (dt1->jday > dt2->jday)
    return (1);

  /* check hour */
  if (dt1->hour < dt2->hour)
    return (-1);
  if (dt1->hour > dt2->hour)
    return (1);

  /* check minute */
  if (dt1->min < dt2->min)
    return (-1);
  if (dt1->min > dt2->min)
    return (1);

  /* check second */
  if (dt1->sec < dt2->sec)
    return (-1);
  if (dt1->sec > dt2->sec)
    return (1);

  /* if I got this far, times are equal */
  return (0);
}

#define NO_ENDING_TIME "No Ending Time"

static int
earlier (evalresp_channel *a, evalresp_channel *b)
{
  int open_a, open_b;
  evalresp_datetime a_time, b_time;
  open_a = !strncasecmp (a->end_t, NO_ENDING_TIME, (sizeof(NO_ENDING_TIME)-1));
  open_b = !strncasecmp (b->end_t, NO_ENDING_TIME, (sizeof(NO_ENDING_TIME)-1));

  if (open_a || open_b)
  {
    if (!open_b)
    {
      return 0; // if a is open, but b is closed, a is later
    }
    else if (!open_a)
    {
      return 1; //  if b is open, but a is closed, a is earlier
    }
    else
    {
      // otherwise, both open so compare start times
      parse_datetime (a->beg_t, &a_time);
      parse_datetime (b->beg_t, &b_time);
      return timecmp (&a_time, &b_time) <= 0;
    }
  }
  else
  {
    // both closed, so compare end times
    parse_datetime (a->end_t, &a_time);
    parse_datetime (b->end_t, &b_time);
    return timecmp (&a_time, &b_time) <= 0;
  }
}

time_t
to_epoch (evalresp_datetime *datetime)
{
  struct tm time = {0};
  time_t epoch, delta;
  /* find epoch of start of year */
  time.tm_year = datetime->year - 1900;
  time.tm_mday = 1;
  time.tm_mon = 0;
  epoch = timegm (&time);
  /* then add the rest - note correction for julian day being 1-indexed */
  delta = datetime->jday - 1;
  delta = datetime->hour + delta * 24;
  delta = datetime->min + delta * 60;
  delta = datetime->sec + delta * 60;
  return epoch + delta;
}

#define INDEFINITE INT_MAX

static int
duration (evalresp_channel *channel)
{
  evalresp_datetime begin, end;

  if (!strncasecmp (channel->end_t, NO_ENDING_TIME, (sizeof(NO_ENDING_TIME)-1)))
  {
    return INDEFINITE;
  }
  else
  {
    parse_datetime (channel->beg_t, &begin);
    parse_datetime (channel->end_t, &end);
    return (int)(to_epoch (&end) - to_epoch (&begin));
  }
}

static int
in_epoch (evalresp_datetime *requirement, const char *beg_t, const char *end_t)
{
  evalresp_datetime start_time, end_time;

  parse_datetime (beg_t, &start_time);
  if (strncasecmp (end_t, NO_ENDING_TIME, (sizeof(NO_ENDING_TIME)-1)))
  {
    parse_datetime (end_t, &end_time);
    return ((timecmp (&start_time, requirement) <= 0 && timecmp (&end_time, requirement) > 0));
  }
  else
  {
    return ((timecmp (&start_time, requirement) <= 0));
  }
}

static int
channel_matches (evalresp_logger *log, const evalresp_filter *filter, evalresp_channel *channel)
{
  int i;
  if (filter->datetime && filter->datetime->year)
  {
    if (!in_epoch (filter->datetime, channel->beg_t, channel->end_t))
    {
      return 0;
    }
  }
  if (filter->sncls->nscn)
  {
    for (i = 0; i < filter->sncls->nscn; ++i)
    {
      evalresp_sncl *sncl = filter->sncls->scn_vec[i];
      if (reg_string_match (log, channel->staname, sncl->station, "-g") &&
          ((!strlen (sncl->network) && !strlen (channel->network)) ||
           reg_string_match (log, channel->network, sncl->network, "-g")) &&
          reg_string_match (log, channel->locid, sncl->locid, "-g") &&
          reg_string_match (log, channel->chaname, sncl->channel, "-g"))
      {
        sncl->found++;
        return 1;
      }
    }
    return 0;
  }
  else
  {
    return 1;
  }
}

int
collect_channels (evalresp_logger *log, const char *seed_or_xml,
                  evalresp_options const *const options, evalresp_channels **channels)
{
  const char *read_ptr = seed_or_xml;
  evalresp_channel *channel;
  int status = EVALRESP_OK;
  // TODO - first_line and first_field are lookaheads that can be eliminated since
  // we are reading from char and can easily backstep
  char first_line[MAXLINELEN] = "";

  *channels = NULL;
  if (!(status = evalresp_alloc_channels (log, channels)))
  {
    while (!status && !end_of_string (&read_ptr))
    {
      if (!(channel = calloc (1, sizeof (*channel))))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate memory for channel");
        status = EVALRESP_MEM;
      }
      else
      {
        if (!(status = read_channel_header (log, &read_ptr, first_line, channel)))
        {
          if (!(status = read_channel_data (log, options, &read_ptr, first_line, channel)))
          {
            if (!(status = add_channel (log, channel, *channels)))
            {
              channel = NULL; // don't free below because added above
            }
          }
        }
        evalresp_free_channel (&channel);
      }
    }
  }

  if (status)
  {
    evalresp_free_channels (channels);
  }

  return status;
}

static int
same_channel (evalresp_channel *a, evalresp_channel *b)
{
  return !strcmp (a->network, b->network) && !strcmp (a->staname, b->staname) && !strcmp (a->locid, b->locid) && !strcmp (a->chaname, b->chaname);
}

/* WARNING - for efficiency this mutates channels_in (deleting channels) */
int
filter_channels (evalresp_logger *log, const evalresp_filter *filter,
                 evalresp_channels *channels_in, evalresp_channels **channels_out)
{
  int status = EVALRESP_OK, i, j, best_match, warn_user = 0;
  evalresp_channel *candidate, *other;

  if (!(status = evalresp_alloc_channels (log, channels_out)))
  {
    for (i = 0; i < channels_in->nchannels && !status; ++i)
    {
      if ((candidate = channels_in->channels[i])) /* this may be null */
      {
        /* basic functionality: only ever consider candidates that match the filter */
        if (!filter || channel_matches (log, filter, candidate))
        {
          /* run through remaining channels removing any that this "beats".
             if it "loses" then stop, but delete this. */
          best_match = 1;
          for (j = i + 1; j < channels_in->nchannels && best_match; ++j)
          {
            if ((other = channels_in->channels[j])) /* this may be null */
            {
              /* other must match the filter too.  we do this in two parts.
                first the channel must be the same */
              if (same_channel (candidate, other))
              {
                /* second, either there's no date filter or that matches too */
                if (!(filter && filter->datetime && filter->datetime->year))
                {
                  /* when no date filter is specified, we go with the latest data */
                  if (earlier (candidate, other))
                  {
                    best_match = 0;
                  }
                }
                else
                {
                  /* note that if other is not in_epoch then it's automatically a loser
                     and can be deleted.  it's only kept if it matches AND is shorter */
                  if (in_epoch (filter->datetime, other->beg_t, other->end_t) &&
                      duration (candidate) >= duration (other))
                  {
                    warn_user = 1;
                    best_match = 0;
                  }
                }
                if (best_match)
                {
                  /* candidate beat this, so discard it */
                  evalresp_free_channel (&channels_in->channels[j]);
                }
              }
            }
          }
          if (best_match) /* candidate won */
          {
            /* only check channels that we will output */
            if (!(status = check_channel (log, candidate)))
            {
              if (!(status = add_channel (log, candidate, *channels_out)))
              {
                channels_in->channels[i] = NULL; /* don't free with channels_in */
              }
            }
          }
          else /* candidate lost */
          {
            evalresp_free_channel (&channels_in->channels[i]);
          }
        }
      }
    }
  }

  if (!status && warn_user)
  {
    evalresp_log (log, EV_WARN, EV_WARN,
                  "Two or more entries match the same SNCL and date; the shortest was used");
  }

  return status;
}

int
evalresp_char_to_channels (evalresp_logger *log, const char *seed_or_xml,
                           evalresp_options const *const options,
                           const evalresp_filter *filter, evalresp_channels **channels)
{
  int status = EVALRESP_OK;
  evalresp_channels *all_channels = NULL;

  *channels = NULL;
  if (!(status = collect_channels (log, seed_or_xml, options, &all_channels)))
  {
    status = filter_channels (log, filter, all_channels, channels);
  }

  evalresp_free_channels (&all_channels);
  if (status)
  {
    evalresp_free_channels (channels);
  }

  return status;
}

int
evalresp_file_to_channels (evalresp_logger *log, FILE *file,
                           evalresp_options const *const options,
                           const evalresp_filter *filter, evalresp_channels **channels)
{
  char *seed = NULL;
  int status = EVALRESP_OK;
  if (!(status = file_to_char (log, file, &seed)))
  {
    status = evalresp_char_to_channels (log, seed, options, filter, channels);
  }
  free (seed);
  return status;
}

/* Detection of FDSN StationXML by searching the first 255 bytes of the
 * file for "<FDSNStationXML".
 *
 * Return 1 if Station, 0 if not and -1 on error. */
int
evalresp_file_detect_stationxml (evalresp_logger *log, FILE *file)
{
  char buffer[255];
  int status = -1;

  if (fread (buffer, sizeof (buffer), 1, file) == 1)
  {
    buffer[sizeof (buffer) - 1] = '\0';

    if (strstr (buffer, "<FDSNStationXML"))
      status = 1;
    else
      status = 0;
  }

  if (fseek (file, 0L, SEEK_SET))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot set file position back to 0: %s", strerror (errno));
  }

  return status;
}

int
evalresp_filename_to_channels (evalresp_logger *log, const char *filename, evalresp_options const *const options,
                               const evalresp_filter *filter, evalresp_channels **channels)
{
  FILE *file = NULL;
  int status = EVALRESP_OK;
  int station_xml = options != NULL ? options->station_xml : 0;

  if (!(status = open_file (log, filename, &file)))
  {
    FILE *temp_file = NULL;

    /* Attempt to detect StationXML if not forced */
    if (options != NULL && options->station_xml == 0)
    {
      station_xml = evalresp_file_detect_stationxml (log, file);

      /* Assume RESP on detection error */
      if (station_xml == -1)
        station_xml = 0;
    }

    if (options != NULL && station_xml)
    {
      if (EVALRESP_OK == (status = evalresp_xml_stream_to_resp_file (log, 1, file, NULL, &temp_file)))
      {
        if (temp_file != NULL)
        {
          fclose (file);
          file = temp_file;
          temp_file = NULL;
        }
      }
    }
    if (EVALRESP_OK == status)
    {
      status = evalresp_file_to_channels (log, file, options, filter, channels);
    }
  }
  if (file)
  {
    fclose (file);
  }
  return status;
}

int
evalresp_new_filter (evalresp_logger *log, evalresp_filter **filter)
{
  int status = EVALRESP_OK;
  if (!(*filter = calloc (1, sizeof (**filter))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate filter");
    status = EVALRESP_MEM;
  }
  else
  {
    if (!((*filter)->datetime = calloc (1, sizeof (*((*filter)->datetime)))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate datetime");
      status = EVALRESP_MEM;
    }
    else
    {
      if (!((*filter)->sncls = calloc (1, sizeof (*(*filter)->sncls))))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate SNCLS");
        status = EVALRESP_MEM;
      }
    }
  }
  return status;
}

int
evalresp_set_year (evalresp_logger *log, evalresp_filter *filter, const char *year)
{
  return parse_int (log, "year", year, &filter->datetime->year);
}

int
evalresp_set_julian_day (evalresp_logger *log, evalresp_filter *filter, const char *julian_day)
{
  return parse_int (log, "Julian day", julian_day, &filter->datetime->jday);
}

// time format is hh[:mm[:ss[.sss]]]
int
evalresp_set_time (evalresp_logger *log, evalresp_filter *filter, const char *time)
{
  int status = EVALRESP_OK;
  const char *str = time;
  char *end;
  filter->datetime->hour = (int)strtol (str, &end, 10);
  if (*end)
  {
    if (*end != ':')
    {
      status = EVALRESP_INP;
    }
    else
    {
      str = end + 1;
      filter->datetime->min = (int)strtol (str, &end, 10);
      if (*end)
      {
        if (*end != ':')
        {
          status = EVALRESP_INP;
        }
        else
        {
          str = end + 1;
          filter->datetime->sec = (float)strtod (str, &end);
          if (*end)
          {
            status = EVALRESP_INP;
          }
        }
      }
    }
  }
  if (status)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot parse '%s' as a time", time);
  }
  return status;
}

int
evalresp_add_sncl_text (evalresp_logger *log, evalresp_filter *filter,
                        const char *net, const char *sta, const char *locid, const char *chan)
{
  int status = EVALRESP_OK;
  evalresp_sncls *sncls = filter->sncls;
  sncls->nscn++;
  if (!(sncls->scn_vec = realloc (sncls->scn_vec, sncls->nscn * sizeof (*sncls->scn_vec))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot reallocate sncls");
    status = EVALRESP_MEM;
  }
  else
  {
    if (!(sncls->scn_vec[sncls->nscn - 1] = calloc (1, sizeof (*(sncls->scn_vec[sncls->nscn - 1])))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate sncl");
      status = EVALRESP_MEM;
    }
    else
    {
      evalresp_sncl *sncl = sncls->scn_vec[sncls->nscn - 1];
      sncl->station = strdup (sta ? sta : "*");
      sncl->network = strdup (net ? net : "*");
      sncl->channel = strdup (chan ? chan : "*");
      // various special cases for location
      if (locid && strlen (locid) == strspn (locid, "?"))
      {
        sncl->locid = strdup ("*");
      }
      else if (locid && strlen (locid) == strspn (locid, " "))
      {
        sncl->locid = strdup ("");
      }
      else
      {
        sncl->locid = strdup (locid ? locid : "*");
      }
    }
  }
  return status;
}

static void
shift_string_left (char *string)
{
  char *c;
  for (c = string + 1; *c; c++)
  {
    *(c - 1) = *c;
  }
  *(c - 1) = '\0';
}

static void
trim_string_inplace (char *trimmed)
{
  char *c;

  while (*trimmed == ' ')
  {
    shift_string_left (trimmed);
  }

  c = trimmed + strlen (trimmed) - 1;
  while (c >= trimmed && *c == ' ')
  {
    *(c--) = '\0';
  }
}

static int
copy_and_trim_string (const char *string, char **trimmed)
{
  if (!(*trimmed = strdup (string ? string : "")))
  {
    return EVALRESP_MEM;
  }
  else
  {
    trim_string_inplace (*trimmed);
    return EVALRESP_OK;
  }
}

static void
coallesce_spaces (char *string)
{
  char *c;

  for (c = string; *c; c++)
  {
    if (*c == ' ')
    {
      while (*(c + 1) == ' ')
      {
        shift_string_left (c + 1);
      }
    }
  }
}

static void
replace_comma_with_space (char *str)
{
  while (*str++)
  {
    if (*str == ',')
    {
      *str = ' ';
    }
  }
}

static int
append_string (struct string_array *array, char *string, int len)
{
  int status = EVALRESP_OK;

  array->nstrings++;
  if (!(array->strings = realloc (array->strings, array->nstrings * sizeof (char *))))
  {
    status = EVALRESP_MEM;
  }
  else
  {
    /* len+1 to include terminating NULL */
    if (!(array->strings[array->nstrings - 1] = calloc (len + 1, 1)))
    {
      status = EVALRESP_MEM;
    }
    else
    {
      strncpy (array->strings[array->nstrings - 1], string, len);
      trim_string_inplace (array->strings[array->nstrings - 1]);
    }
  }

  return status;
}

static int
split_on (evalresp_logger *log, char *string, char *delim, struct string_array **array)
{
  int status = EVALRESP_OK;
  char *start = string, *end;

  if (!(*array = calloc (1, sizeof (**array))))
  {
    status = EVALRESP_MEM;
  }
  else
  {
    while (!status && *start)
    {
      if (!(end = strstr (start, delim)))
      {
        end = start + strlen (start);
      }
      status = append_string (*array, start, end - start);
      start = end;
      if (*start)
        start++; /* drop delimiter */
    }
  }

  if (!status && !(*array)->nstrings)
  {
    /* add in an single, empty value, since this is for user options */
    status = append_string (*array, "", 0);
  }

  return status;
}

static int
split_on_space (evalresp_logger *log, const char *name, const char *str, struct string_array **array)
{
  int status = EVALRESP_OK;
  char *trimmed = NULL;

  if (!(status = copy_and_trim_string (str, &trimmed)))
  {
    replace_comma_with_space (trimmed);
    coallesce_spaces (trimmed);
    status = split_on (log, trimmed, " ", array);
  }

  free (trimmed);
  return status;
}

static int
split_on_comma (evalresp_logger *log, const char *name, const char *str, struct string_array **array)
{
  int status = EVALRESP_OK;
  char *trimmed = NULL;

  if (!(status = copy_and_trim_string (str, &trimmed)))
  {
    status = split_on (log, trimmed, ",", array);
  }

  free (trimmed);
  return status;
}

int
evalresp_add_sncl_all (evalresp_logger *log, evalresp_filter *filter,
                       const char *net, const char *sta, const char *locid, const char *chan)
{
  int status = EVALRESP_OK, i, j, k;
  struct string_array *stations = NULL, *channels = NULL, *locations = NULL;

  if (!(status = split_on_space (log, "stations", sta, &stations)))
  {
    if (!(status = split_on_space (log, "channels", chan, &channels)))
    {
      if (!(status = split_on_comma (log, "locations", locid, &locations)))
      {
        for (i = 0; !status && i < stations->nstrings; i++)
        {
          for (j = 0; !status && j < locations->nstrings; j++)
          {
            for (k = 0; !status && k < channels->nstrings; k++)
            {
              status = evalresp_add_sncl_text (log, filter,
                                               net, stations->strings[i], locations->strings[j], channels->strings[k]);
            }
          }
        }
      }
    }
  }
  free_string_array (stations);
  free_string_array (channels);
  free_string_array (locations);
  return status;
}

void
evalresp_free_filter (evalresp_filter **filter)
{
  if (*filter)
  {
    free ((*filter)->datetime);
    evalresp_free_sncls ((*filter)->sncls);
    free (*filter);
    *filter = NULL;
  }
}

int
parse_int (evalresp_logger *log, const char *name, const char *str, int *value)
{
  int status = EVALRESP_OK;
  char *end;
  *value = (int)strtol (str, &end, 10);
  while (isspace (*end))
    ++end;
  if (*end)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot parse '%s' as an integer for %s", str, name);
    status = EVALRESP_INP;
  }
  return status;
}

int
parse_double (evalresp_logger *log, const char *name, const char *str, double *value)
{
  int status = EVALRESP_OK;
  char *end;
  *value = strtod (str, &end);
  while (isspace (*end))
    ++end;
  if (*end)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot parse '%s' as a double for %s", str, name);
    status = EVALRESP_INP;
  }
  return status;
}

int
is_int (const char *test, evalresp_logger *log)
{
  char ipattern[MAXLINELEN];

  /* first check to see if is an integer prefixed by a plus or minus.  If not
     then check to see if is simply an integer */

  strncpy (ipattern, "^[-+]?[0-9]+$", MAXLINELEN);
  return (reg_string_match (log, test, ipattern, "-r"));
}

int
is_real (const char *test, evalresp_logger *log)
{
  char fpattern[MAXLINELEN];
  strncpy (fpattern, "^[-+]?[0-9]+\\.?[0-9]*[Ee][-+]?[0-9]+$", MAXLINELEN);
  strcat (fpattern, "|^[-+]?[0-9]*\\.[0-9]+[Ee][-+]?[0-9]+$");
  strcat (fpattern, "|^[-+]?[0-9]+\\.?[0-9]*$");
  strcat (fpattern, "|^[-+]?[0-9]*\\.[0-9]+$");
  return (reg_string_match (log, test, fpattern, "-r"));
}
