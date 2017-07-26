/* string_fctns.c */

/*
   02/12/2005 -- [IGD] Moved parse_line() to ev_parse_line() to avoid name
                       conflict    with external libraries
   10/21/2005 -- [ET]  Modified so as not to require characters after
                       'units' specifiers like "M" and "COUNTS";
                       improved error message generated when no
                       data fields found on line; added test of 'strstr()'
                       result in 'count_fields()' and 'parse_field()'
                       functions to prevent possible program crashes
                       due to null pointer (tended to be caused by
                       response files with Windows-type "CR/LF" line
                       ends); modified 'get/next/check_line()' functions
                       to make them strip trailing CR and LF characters
                       (instead of just LF character).
    1/18/2006 -- [ET]  Renamed 'regexp' functions to prevent name clashes
                       with other libraries.
     4/4/2006 -- [ET]  Modified 'parse_line()' and 'parse_delim_line()'
                       functions to return allocated string array with
                       empty entry (instead of NULL) if no fields found.
    8/21/2006 -- [IGD] Version 3.2.36: Added support for TESLA units
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <string.h>

#include "./legacy.h"
#include "./old_fctns.h"
#include <evalresp/evresp.h>
#include <evalresp/private.h>
#include <evalresp/public_channels.h>
#include <evalresp/regexp.h>

char myLabel[20];

struct string_array *
ev_parse_line (char *line, evalresp_logger *log)
{
  char *lcl_line, field[MAXFLDLEN];
  int nfields, fld_len, i = 0;
  struct string_array *lcl_strings;

  lcl_line = line;
  nfields = count_fields (lcl_line);
  if (nfields > 0)
  {
    if (!(lcl_strings = alloc_string_array (nfields, log)))
    {
      return NULL;
    }
    for (i = 0; i < nfields; i++)
    {
      if (0 > parse_field (line, i, field, log))
      {
        break;
      }
      fld_len = strlen (field) + 1;
      if ((lcl_strings->strings[i] = (char *)malloc (
               fld_len * sizeof (char))) == (char *)NULL)
      {
        evalresp_log (log, EV_ERROR, 0,
                      "ev_parse_line; malloc() failed for (char) vector");
        break;
      }
      strncpy (lcl_strings->strings[i], "", fld_len);
      strncpy (lcl_strings->strings[i], field, fld_len - 1);
    }
    if (i < nfields)
    {
      for (i--; i > 0; i--)
      {
        free (lcl_strings->strings[i]);
        lcl_strings->strings[i] = NULL;
      }
      free (lcl_strings->strings);
      free (lcl_strings);
      return NULL;
    }
  }
  else
  { /* if no fields then alloc string array with empty entry */
    if (!(lcl_strings = alloc_string_array (1, log)))
    {
      return NULL;
    }
    if ((lcl_strings->strings[0] = (char *)malloc (sizeof (char))) == (char *)NULL)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "ev_parse_line; malloc() failed for (char) vector");
      free (lcl_strings->strings);
      free (lcl_strings);
      return NULL;
    }
    strncpy (lcl_strings->strings[0], "", 1);
  }
  return (lcl_strings);
}

struct string_array *
parse_delim_line (char *line, char *delim, evalresp_logger *log)
{
  char *lcl_line, field[MAXFLDLEN];
  int nfields, fld_len, i = 0;
  struct string_array *lcl_strings;

  lcl_line = line;
  nfields = count_delim_fields (lcl_line, delim);
  if (nfields > 0)
  {
    if (!(lcl_strings = alloc_string_array (nfields, log)))
    {
      return NULL;
    }
    for (i = 0; i < nfields; i++)
    {
      memset (field, 0, MAXFLDLEN);
      if (0 > parse_delim_field (line, i, delim, field, log))
      {
        break;
      }
      fld_len = strlen (field) + 1;
      if ((lcl_strings->strings[i] = (char *)malloc (
               fld_len * sizeof (char))) == (char *)NULL)
      {
        evalresp_log (log, EV_ERROR, 0,
                      "parse_delim_line; malloc() failed for (char) vector");
        break;
      }
      strncpy (lcl_strings->strings[i], "", fld_len);
      strncpy (lcl_strings->strings[i], field, fld_len - 1);
    }
    if (i < nfields)
    {
      for (i--; i > 0; i--)
      {
        free (lcl_strings->strings[i]);
        lcl_strings->strings[i] = NULL;
      }
      free (lcl_strings->strings);
      free (lcl_strings);
      return NULL;
    }
  }
  else
  { /* if no fields then alloc string array with empty entry */
    if (!(lcl_strings = alloc_string_array (1, log)))
    {
      return NULL;
    }
    if ((lcl_strings->strings[0] = (char *)malloc (sizeof (char))) == (char *)NULL)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "parse_delim_line; malloc() failed for (char) vector");
      free (lcl_strings->strings);
      free (lcl_strings);
      return NULL;
    }
    strncpy (lcl_strings->strings[0], "", 1);
  }
  return (lcl_strings);
}

int
get_field (FILE *fptr, char *return_field, int blkt_no, int fld_no,
           char *sep, int fld_wanted, evalresp_logger *log)
{
  char line[MAXLINELEN];

  /* first get the next non-comment line */

  get_line (fptr, line, blkt_no, fld_no, sep, log);

  /* then parse the field that the user wanted from the line get_line returned
   * and return the length of the field */
  return (parse_field (line, fld_wanted, return_field, log));
}

int
test_field (FILE *fptr, char *return_field, int *blkt_no, int *fld_no,
            char *sep, int fld_wanted, evalresp_logger *log)
{
  char line[MAXLINELEN];

  /* first get the next non-comment line */

  if (0 >= next_line (fptr, line, blkt_no, fld_no, sep, log))
  {
    return 0;
  }

  /* then parse the field that the user wanted from the line get_line returned
   * and return the length of the field */
  return (parse_field (line, fld_wanted, return_field, log));
}

int
get_line (FILE *fptr, char *return_line, int blkt_no, int fld_no, char *sep, evalresp_logger *log)
{
  char *lcl_ptr, line[MAXLINELEN];
  int lcl_blkt, lcl_fld, test;
  int tmpint;
  char tmpstr[200];
  int i;

  test = fgetc (fptr);

  while (test != EOF && test == '#')
  {
    strncpy (line, "", MAXLINELEN - 1);
    (void)fgets (line, MAXLINELEN, fptr);
    test = fgetc (fptr);
  }

  if (test == EOF)
  {
    return (0);
  }
  else
  {
    ungetc (test, fptr);
    (void)fgets (line, MAXLINELEN, fptr);

    for (i = 0; i < strlen (line); i++)
    {
      if ('\t' == line[i])
        line[i] = ' ';
    }

    /* check for blank line */
    tmpint = sscanf (line, "%s", tmpstr);

    if (tmpint == EOF)
    {
      return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
    }

    tmpint = strlen (line); /* strip any trailing CR or LF chars */
    while (tmpint > 0 && line[tmpint - 1] < ' ')
      line[--tmpint] = '\0';
  }

  /*if(!line)
     error_return(UNEXPECTED_EOF, "get_line; no more non-comment lines found in file");*/

  test = parse_pref (&lcl_blkt, &lcl_fld, line, log);
  if (!test)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "get_line; unrecogn. prefix on the following line:\n\t  '%s'",
                  line);
    return UNDEF_PREFIX;
  }

  /* check the blockette and field numbers found on the line versus the expected values */

  if (blkt_no != lcl_blkt)
  {
    /* try to parse the next line */
    return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
    /*
         removed by SBH 2004.079
         if(fld_no != lcl_fld) {
         error_return(PARSE_ERROR,"get_line; %s%s%3.3d%s%3.3d%s%2.2d%s%2.2d","blkt",
         " and fld numbers do not match expected values\n\tblkt_xpt=B",
         blkt_no, ", blkt_found=B", lcl_blkt, "; fld_xpt=F", fld_no,
         ", fld_found=F", lcl_fld);
         }
         */
  }
  else if (fld_no != lcl_fld)
  {
    /* try to parse the next line */
    return get_line (fptr, return_line, blkt_no, fld_no, sep, log);
    /*
         removed by SBH 2004.079
         error_return(PARSE_ERROR,"get_line (parsing blockette [%3.3d]); %s%2.2d%s%2.2d",
         lcl_blkt, "unexpected fld number\n\tfld_xpt=F", fld_no,
         ", fld_found=F", lcl_fld, lcl_blkt);
         */
  }

  if ((lcl_ptr = strstr (line, sep)) == (char *)NULL)
  {
    evalresp_log (log, EV_ERROR, 0, "get_line; seperator string not found");
    return UNDEF_SEPSTR;
  }
  else if ((lcl_ptr - line) > (int)(strlen (line) - 1))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "get_line; nothing to parse after seperator string");
    return UNDEF_SEPSTR;
  }

  lcl_ptr++;
  while (*lcl_ptr && isspace (*lcl_ptr))
  {
    lcl_ptr++;
  }

  if ((lcl_ptr - line) > (int)strlen (line))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "get_line; no non-white space after seperator string");
    return UNDEF_SEPSTR;
  }

  strncpy (return_line, lcl_ptr, MAXLINELEN);
  return (strlen (return_line));
}

int
next_line (FILE *fptr, char *return_line, int *blkt_no, int *fld_no,
           char *sep, evalresp_logger *log)
{
  char *lcl_ptr, line[MAXLINELEN];
  int test;
  int tmpint;
  char tmpstr[200];

  test = fgetc (fptr);

  while (test != EOF && test == '#')
  {
    (void)fgets (line, MAXLINELEN, fptr);
    test = fgetc (fptr);
  }

  if (test == EOF)
  {
    return (0);
  }
  else
  {
    ungetc (test, fptr);
    (void)fgets (line, MAXLINELEN, fptr);
    tmpint = strlen (line); /* strip any trailing CR or LF chars */
    while (tmpint > 0 && line[tmpint - 1] < ' ')
      line[--tmpint] = '\0';
  }

  /* check for blank line */

  tmpint = sscanf (line, "%s", tmpstr);

  if (tmpint == EOF)
  {
    return next_line (fptr, return_line, blkt_no, fld_no, sep, log);
  }

  test = parse_pref (blkt_no, fld_no, line, log);
  if (!test)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "get_field; unrecogn. prefix on the following line:\n\t  '%s'",
                  line);
    return 0;
  }

  if ((lcl_ptr = strstr (line, sep)) == (char *)NULL)
  {
    evalresp_log (log, EV_ERROR, 0, "get_field; seperator string not found");
    return 0;
  }
  else if ((lcl_ptr - line) > (int)(strlen (line) - 1))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "get_field; nothing to parse after seperator string");
    return 0;
  }

  lcl_ptr++;
  while (*lcl_ptr && isspace (*lcl_ptr))
  {
    lcl_ptr++;
  }
  strncpy (return_line, lcl_ptr, MAXLINELEN);

  return (*fld_no);
}

int
count_fields (char *line)
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
  return (nfields);
}

int
count_delim_fields (char *line, char *delim)
{
  const char *lcl_ptr, *tmp_ptr;
  int nfields = 0;
  int line_len = 0;

  lcl_ptr = (const char *)line;
  while (*lcl_ptr && (tmp_ptr = strstr ((lcl_ptr + line_len), delim)) != (char *)NULL)
  {
    line_len = (tmp_ptr - lcl_ptr + 1);
    nfields += 1;
  }
  if (strlen ((lcl_ptr + line_len)))
  {
    nfields++;
  }
  else if (!strcmp ((lcl_ptr + line_len - 1), ","))
  {
    nfields++;
  }

  return (nfields);
}

int
parse_field (char *line, int fld_no, char *return_field, evalresp_logger *log)
{
  char *lcl_ptr, *new_ptr;
  char lcl_field[MAXFLDLEN];
  int nfields, i;

  nfields = count_fields (line);
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

int
parse_delim_field (char *line, int fld_no, char *delim, char *return_field, evalresp_logger *log)
{

  char *lcl_ptr, *tmp_ptr = NULL;
  int nfields, i;

  nfields = count_delim_fields (line, delim);
  if (fld_no >= nfields)
  {
    if (nfields > 0)
    {
      evalresp_log (log, EV_ERROR, 0, "%s%d%s%d%s",
                    "parse_delim_field; Input field number (", fld_no,
                    ") exceeds number of fields on line(", nfields, ")");
      return PARSE_ERROR;
    }
    else
    {
      evalresp_log (log, EV_ERROR, 0, "%s",
                    "parse_delim_field; Data fields not found on line");
      return PARSE_ERROR;
    }
  }

  lcl_ptr = line;
  for (i = 0; i <= fld_no; i++)
  {
    tmp_ptr = strstr (lcl_ptr, delim);
    if (tmp_ptr && i < fld_no)
      lcl_ptr = tmp_ptr + 1;
  }

  // AC - removed incorrect memset here.  was already compensated for in caller.
  if (tmp_ptr)
    strncpy (return_field, lcl_ptr, (tmp_ptr - lcl_ptr));
  else
    strncpy (return_field, lcl_ptr, strlen (lcl_ptr));

  return (strlen (return_field));
}

int
check_line (FILE *fptr, int *blkt_no, int *fld_no, char *in_line, evalresp_logger *log)
{
  char line[MAXLINELEN];
  int test;
  char tmpstr[200];
  int tmpint;

  test = fgetc (fptr);
  while (test != EOF && test == '#')
  {
    (void)fgets (line, MAXLINELEN, fptr);
    test = fgetc (fptr);
  }

  /*
     while(test != EOF && (test == 10) {
     fgets(line, MAXLINELEN, fptr);
     test = fgetc(fptr);
     }
     */

  if (test == EOF)
  {
    return (0);
  }
  else
  {
    ungetc (test, fptr);
    (void)fgets (line, MAXLINELEN, fptr);

    /* check for blank line */
    tmpint = sscanf (line, "%s", tmpstr);

    if (tmpint == EOF)
    {
      return check_line (fptr, blkt_no, fld_no, in_line, log);
    }

    tmpint = strlen (line); /* strip any trailing CR or LF chars */
    while (tmpint > 0 && line[tmpint - 1] < ' ')
      line[--tmpint] = '\0';
  }

  test = parse_pref (blkt_no, fld_no, line, log);
  if (!test)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "check_line; unrecogn. prefix on the following line:\n\t  '%s'",
                  line);
    return 0;
  }

  strncpy (in_line, line, MAXLINELEN);
  return (1);
}

int
get_int (char *in_line, evalresp_logger *log)
{
  int value;

  if (!is_int (in_line, log))
  {
    evalresp_log (log, EV_ERROR, 0, "get_int; '%s' is not an integer",
                  in_line);
  }
  value = atoi (in_line);
  return (value);
}

double
get_double (char *in_line, evalresp_logger *log)
{
  double lcl_val;

  if (!is_real (in_line, log))
  {
    evalresp_log (log, EV_ERROR, 0, "get_double; '%s' is not a real number",
                  in_line);
  }
  lcl_val = atof (in_line);
  return (lcl_val);
}

int
check_units (evalresp_channel *channel, char *line, evalresp_logger *log)
{
  int i, first_flag = 0;

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

  if (def_units_flag)
  {
    return DEFAULT;
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
    return PRESSURE;

  /* IGD 08/21/06 Added support for TESLA */
  if (strncasecmp (line, "T -", 3) == 0)
    return TESLA;

  /* IHD 10/03/13 Adding DEGREES CENTIGRADE */
  if (strncasecmp (line, "C -", 3) == 0)
    return CENTIGRADE;

  if (string_match (line, "^[CNM]?M/S\\*\\*2|^[CNM]?M/SEC\\*\\*2", "-r", log))
  {
    if (first_flag && !strncmp ("NM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp ("MM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp ("CM", line, (size_t)2))
      channel->unit_scale_fact = 1.0e2;
    return ACC;
  }
  else if (string_match (line, "^[CNM]?M/S|^[CNM]?M/SEC", "-r", log))
  {
    if (first_flag && !strncmp (line, "NM", 2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp (line, "MM", 2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp (line, "CM", 2))
      channel->unit_scale_fact = 1.0e2;
    return VEL;
  }
  else if (string_match (line, "^[CNM]?M[^A-Z/]?", "-r", log))
  {
    if (first_flag && !strncmp (line, "NM", 2))
      channel->unit_scale_fact = 1.0e9;
    else if (first_flag && !strncmp (line, "MM", 2))
      channel->unit_scale_fact = 1.0e3;
    else if (first_flag && !strncmp (line, "CM", 2))
      channel->unit_scale_fact = 1.0e2;
    return DIS;
  }
  else if (string_match (line, "^COUNTS?[^A-Z]?", "-r", log) || string_match (line, "^DIGITAL[^A-Z]?", "-r", log))
  {
    return COUNTS;
  }
  else if (string_match (line, "^V[^A-Z]?", "-r", log) || string_match (line, "^VOLTS[^A-Z]?", "-r", log))
  {
    return VOLTS;
  }
  evalresp_log (log, EV_WARN, EV_WARN,
                "check_units; units found ('%s') are not supported", line);
  return UNDEF_UNITS;
}

int
string_match (const char *string, char *expr, char *type_flag, evalresp_logger *log)
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
    evalresp_log (log, EV_ERROR, 0, "%s string_match; improper pattern type (%s)\n",
                  myLabel, type_flag);
    return 0;
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
    return 0;
  }
  lcl_ptr = lcl_string;
  test = evr_regexec (prog, lcl_ptr, log);

  free (prog);
  return (test);
}

int
is_time (const char *test, evalresp_logger *log)
{
  char fpattern[MAXLINELEN];

  /* time strings must be in the format 'hh:mm:ss[.#####]', so more than 14
     characters is an error (too many digits) */

  if (is_int (test, log) && atoi (test) < 24)
    return (1);

  /* if gets this far, just check without the decimal, then with the decimal */

  strncpy (fpattern, "^[0-9][0-9]?:[0-9][0-9]$", MAXLINELEN);
  strcat (fpattern, "|^[0-9][0-9]?:[0-9][0-9]:[0-9][0-9]$");
  strcat (fpattern, "|^[0-9][0-9]?:[0-9][0-9]:[0-9][0-9]\\.[0-9]*$");
  return (string_match (test, fpattern, "-r", log));
}

int
add_null (char *s, int len, char where)
{
  int len_save;
  switch (where)
  {
  case 'a': /* remove extra spaces from end of string */
    len_save = len;
    for (; len >= 0; len--)
    { /* test in reverse order */
      if (!isspace (*(s + len)))
      {
        if (*(s + len) == '\0')
        {
          return (len);
        }
        else
        {
          if (len != len_save)
            len += 1;
          *(s + len) = '\0';
          return (len);
        }
      }
    }
    break;
  case 'e': /* add null character to end of string */
    if (len > 0)
    {
      *(s + len) = '\0';
      return (len);
    }
    break;
  }
  *s = '\0';
  return (0);
}

int
is_IIR_coeffs (FILE *fp, int position)
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
  char line[500];
  int i, denoms, result;
  for (i = 0; i < 80; i++)
  { /* enough to make sure we are getting field 10; not to much to get to the next blockette */
    (void)fscanf (fp, "%s", line);
    if (strncmp (line, "B054F10", 7) == 0)
      break;
  }
  if (strncmp (line, "B054F10", 7) == 0)
  {
    for (i = 0; i < 4; i++)
      (void)fscanf (fp, "%s", line);
    denoms = atoi (line);
    if (denoms == 0)
      result = 0;
    else
      result = 1;
  }
  else
    result = 0;
  fseek (fp, position, SEEK_SET);
  return (result);
}
