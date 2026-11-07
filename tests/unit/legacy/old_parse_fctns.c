/* parse_fctns.c */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
    8/28/2001 -- [ET]  Added "!= 0" to several conditionals to squelch
                       "possibly incorrect assignment" warnings.
   10/19/2005 -- [ET]  Modified to handle case where file contains
                      "B052F03 Location:" and nothing after it on line.
 */

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "evalresp/public_channels.h"
#include "legacy.h"
#include "old_fctns.h"
#include <evalresp/private.h>

/* define a pointer to a channel structure to use in determining the input and
 output units if using "default" units and for use in error output*/
evalresp_channel *GblChanPtr;
char FirstLine[MAXLINELEN];
int FirstField;
int curr_seq_no;

double atof ();

int
parse_pref (int *blkt_no, int *fld_no, char *line, evalresp_logger *log)
{
  char fldstr[FLDSTRLEN], blktstr[BLKTSTRLEN];

  strncpy (fldstr, "", FLDSTRLEN);
  strncpy (blktstr, "", BLKTSTRLEN);
  if (*line != 'B' || strlen (line) < 7)
    return (0);

  strncpy (blktstr, (line + 1), 3);
  strncpy (fldstr, (line + 5), 2);
  *(blktstr + 3) = '\0';
  *(fldstr + 2) = '\0';

  if (!is_int (blktstr, log))
  {
    evalresp_log (log, EV_ERROR, 0, "parse_pref; prefix '%s' cannot be %s",
                  blktstr, "converted to a blockette number");
    return 0;
  }
  *blkt_no = atoi (blktstr);
  if (!is_int (fldstr, log))
  {
    evalresp_log (log, EV_ERROR, 0, "parse_pref; prefix '%s' cannot be %s",
                  fldstr, "converted to a blockette number");
    return 0;
  }
  *fld_no = atoi (fldstr);
  return (1);
}

void
parse_pz (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ, check_fld;
  int blkt_read;
  int npoles, nzeros;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [53] the first field expected is a F03, if from a blockette [43], the
     first field should be a F05 */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_pz; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
    blkt_read = 53;
  else
    blkt_read = 43;

  if (0 > parse_field (FirstLine, 0, field, log))
  {
    return;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "parse_pz; parsing (Poles & Zeros), illegal filter type ('%s')",
                  field);
    return;
  }
  blkt_typ = *field;
  switch (blkt_typ)
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
    evalresp_log (log, EV_ERROR, 0,
                  "parse_pz; parsing (Poles & Zeros), unexpected filter type ('%c')",
                  *field);
    return;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField + 1;

  /* then, if is a B053F04, get the stage sequence number (from the file) */

  if (check_fld == 4)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* then the A0 normalization factor */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.pole_zero.a0 = get_double (field, log);

  /* the A0 normalization frequency */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.pole_zero.a0_freq = get_double (field, log);

  /* the number of zeros */

  if (0 > get_field (fptr, field, blkt_read, check_fld, ":", 0, log))
  {
    return;
  }
  nzeros = get_int (field, log);
  blkt_ptr->blkt_info.pole_zero.nzeros = nzeros;

  /* remember to allocate enough space for the number of zeros to follow */

  blkt_ptr->blkt_info.pole_zero.zeros = alloc_complex (nzeros, log);

  /* set the expected field to the current value (9 or 10 for [53] or [43])
     to the current value + 5 (14 or 15 for [53] or [43] respectively) */

  check_fld += 5;

  /* the number of poles */

  if (0 > get_field (fptr, field, blkt_read, check_fld, ":", 0, log))
  {
    return;
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
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return;
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].real = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_pz: %s%s%s",
                    "zeros must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.pole_zero.zeros[i].imag = atof (field);
  }

  /* set the expected field to the current value (10 or 11 for [53] or [43])
     to the current value + 5 (15 or 16 for [53] or [43] respectively) */

  check_fld += 5;

  /* and then get the poles */

  for (i = 0; i < npoles; i++)
  {
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return;
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].real = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_pz: %s%s%s",
                    "poles must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.pole_zero.poles[i].imag = atof (field);
  }
}

void
parse_iir_coeff (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ, check_fld;
  int blkt_read;
  int ncoeffs, ndenom;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
    blkt_read = 54;
  else
    blkt_read = 44;

  if (0 > parse_field (FirstLine, 0, field, log))
  {
    return;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "parse_coeff; parsing (IIR_COEFFS), illegal filter type ('%s')",
                  field);
    return;
  }
  blkt_typ = *field;
  switch (blkt_typ)
  {
  case 'D':
    blkt_ptr->type = IIR_COEFFS;
    break;
  default:
    evalresp_log (log, EV_ERROR, 0,
                  "parse_coeff; parsing (IIR_COEFFS), unexpected filter type ('%c')",
                  *field);
    return;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */

  if (check_fld == 4)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* the number of coefficients */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.coeff.nnumer = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.coeff.numer = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */

  check_fld += 2;

  /* the number of denominators */

  if (0 > get_field (fptr, field, blkt_read, check_fld, ":", 0, log))
  {
    return;
  }
  ndenom = get_int (field, log);

  /* if the number of denominators is zero, then is not an IIR filter. */

  if (ndenom == 0)
  {
    evalresp_log (log, EV_ERROR, 0, "%s%s",
                  "parse_coeff; This is not IIR filter , because number of denominators is zero!\n",
                  "\tshould be represented as blockette [53] filters");
    return;
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
    if (0 > get_field (fptr, field, blkt_read, check_fld, " ", 1, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_coeff: %s%s%s",
                    "numerators must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.coeff.numer[i] = atof (field);
  }

  check_fld += 3; /*IGD : we need field 11 now */

  for (i = 0; i < ndenom; i++)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld, " ", 1, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_coeff: %s%s%s",
                    "denominators must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.coeff.denom[i] = atof (field);
  }
}

void
parse_coeff (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ, check_fld;
  int blkt_read;
  int ncoeffs, ndenom;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the response type (from the input line).  Note: if is being called from
     a blockette [54] the first field expected is a F03, if from a blockette [44], the
     first field should be a F05 */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_coeff; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
    blkt_read = 54;
  else
    blkt_read = 44;

  if (0 > parse_field (FirstLine, 0, field, log))
  {
    return;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "parse_coeff; parsing (FIR_ASYM), illegal filter type ('%s')",
                  field);
    return;
  }
  blkt_typ = *field;
  switch (blkt_typ)
  {
  case 'D':
    blkt_ptr->type = FIR_ASYM;
    break;
  default:
    evalresp_log (log, EV_ERROR, 0,
                  "parse_coeff; parsing (FIR_ASYM), unexpected filter type ('%c')",
                  *field);
    return;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField + 1;

  /* then, if is a B054F04, get the stage sequence number (from the file) */

  if (check_fld == 4)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->output_units)
  {
    return;
  }

  /* the number of coefficients */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* set the expected field to the current value (8 or 9 for [54] or [44])
     to the current value + 2 (10 or 11 for [54] or [44] respectively) */

  check_fld += 2;

  /* the number of denominators */

  if (0 > get_field (fptr, field, blkt_read, check_fld, ":", 0, log))
  {
    return;
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
    evalresp_log (log, EV_ERROR, 0, "%s%s",
                  "parse_coeff; Unsupported filter type, IIR and Analog filters\n",
                  "\tshould be represented as blockette [53] filters");
    return;
  }

  /* set the expected field to the current value (10 or 11 for [54] or [44])
     to the current value - 2 (8 or 9 for [54] or [44] respectively) */

  check_fld -= 2;

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld, " ", 1, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_coeff: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }
}

void
parse_list (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ = LIST;
  int blkt_read, check_fld;
  int nresp;
  char field[MAXFLDLEN], line[MAXLINELEN];
  int marker;
  int format = -1;
  /* set the filter type */

  blkt_ptr->type = blkt_typ;

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_list; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
    blkt_read = 55;
  else
    blkt_read = 45;

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B055F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
    {
      return;
    }
  }
  else
  {
    strncpy (line, FirstLine, MAXLINELEN);
    check_fld++;
  }

  /* the units (in first, then out) */

  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* the number of responses */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
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
    marker = ftell (fptr);
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return;
    }
    format = count_fields (line) - 5; /*format == 0 if no number of responses in the file */
    /*format == 1 if  number of responses in the sec column*/
    fseek (fptr, marker, SEEK_SET); /* rewind back to we we've been before the test */
    if (format != 0 && format != 1) /*Wrong format */
    {
      evalresp_log (log, EV_ERROR, 0, "parse_list: %s",
                    "Unknown format for B055F07-11");
      return;
    }

    for (i = 0; i < nresp; i++)
    {
      if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
      {
        return;
      }
      if (0 > parse_field (line, format, field, log)) /* Frequency */
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if (0 > parse_field (line, 1 + format, field, log)) /* the amplitude of the Fourier transform */
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if (0 > parse_field (line, 3 + format, field, log)) /* Phase of the transform */
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }
  else
  { /* This is blockette 45 - leave at as in McSweeny's version */
    for (i = 0; i < nresp; i++)
    {
      if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
      {
        return;
      }
      if (0 > parse_field (line, 0, field, log))
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "freq vals must be real numbers (found '", field, "')");
        return;
      }
      blkt_ptr->blkt_info.list.freq[i] = atof (field);
      if (0 > parse_field (line, 1, field, log))
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "amp vals must be real numbers (found '", field, "')");
        return;
      }
      blkt_ptr->blkt_info.list.amp[i] = atof (field);
      if (0 > parse_field (line, 3, field, log))
      {
        return;
      }
      if (!is_real (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_list: %s%s%s",
                      "phase vals must be real numbers (found '", field,
                      "')");
        return;
      }
      blkt_ptr->blkt_info.list.phase[i] = atof (field);
    }
  }
}

void
parse_generic (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ = GENERIC;
  int blkt_read, check_fld;
  int ncorners;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* set the filter type */

  blkt_ptr->type = blkt_typ;

  /* first get the stage sequence number (from the input line) */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_generic; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }
  if (FirstField == 3)
    blkt_read = 56;
  else
    blkt_read = 46;

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B056F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld++;
    if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
    {
      return;
    }
  }
  else
  {
    strncpy (line, FirstLine, MAXLINELEN);
    check_fld++;
  }

  /* next (from the file) should be the units (in first, then out) */

  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* the number of responses */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
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
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return;
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_generic: %s%s%s",
                    "corner_freqs must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.generic.corner_freq[i] = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_generic: %s%s%s",
                    "corner_slopes must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.generic.corner_slope[i] = atof (field);
  }
}

int
parse_deci (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_logger *log)
{
  int blkt_typ = DECIMATION;
  int blkt_read, check_fld;
  int sequence_no = 0;
  double srate;
  char field[MAXFLDLEN];

  /* set the filter type */

  blkt_ptr->type = blkt_typ;

  /* first get the stage sequence number (from the input line) */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_deci; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return PARSE_ERROR;
  }
  if (FirstField == 3)
  {
    blkt_read = 57;
  }
  else
  {
    blkt_read = 47;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B057F03, get the stage sequence number (from FirstLine)
     and the input units (from the file), otherwise, get the input units (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return PARSE_ERROR;
    }
    sequence_no = get_int (field, log);
    check_fld++;
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return PARSE_ERROR;
    }
  }
  else
  {
    if (0 > parse_field (FirstLine, 0, field, log))
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

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.deci_fact = get_int (field, log);
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.deci_offset = get_int (field, log);

  /* the estimated delay */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.estim_delay = get_double (field, log);

  /* and, finally, the applied correction.  Note:  the calculated delay is left undefined
     by this routine, although space does exist in this filter type for this parameter */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.decimation.applied_corr = get_double (field, log);

  /* return the sequence number of the stage for verification */

  return (sequence_no);
}

int
parse_gain (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_logger *log)
{
  int i, blkt_typ = GAIN;
  int blkt_read, check_fld;
  int sequence_no = 0;
  int nhist = 0;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* set the filter type */

  blkt_ptr->type = blkt_typ;

  /* first get the stage sequence number (from the input line) */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_gain; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 of F05",
                  ", fld_found=F", FirstField);
    return PARSE_ERROR;
  }

  if (FirstField == 3)
  {
    blkt_read = 58;
  }
  else
  {
    blkt_read = 48;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B058F03, get the stage sequence number (from FirstLine)
     and the symmetry type (from the file), otherwise, get the symmetry (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return PARSE_ERROR;
    }
    sequence_no = get_int (field, log);
    check_fld++;
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return PARSE_ERROR;
    }
  }
  else
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return PARSE_ERROR;
    }
    check_fld++;
  }

  /* then get the gain and frequency of gain (these correspond to sensitivity and frequency of
     sensitivity for stage 0 Sensitivity/Gain filters) */

  blkt_ptr->blkt_info.gain.gain = get_double (field, log);
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  blkt_ptr->blkt_info.gain.gain_freq = get_double (field, log);

  /* if there is a history, skip it. First determine number of lines to skip */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return PARSE_ERROR;
  }
  nhist = get_int (field, log);

  /* then skip them  */

  for (i = 0; i < nhist; i++)
  {
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return PARSE_ERROR;
    }
  }

  /* return the sequence number of the stage for verification */

  return (sequence_no);
}

void
parse_polynomial (FILE *fptr, evalresp_blkt *blkt_ptr,
                  evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ;
  int blkt_read, check_fld;
  int ncoeffs;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the stage sequence number (from the input line) */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_polynomial; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
  {
    blkt_read = 62;
  }
  else
  {
    blkt_read = 42;
  }

  if (0 > parse_field (FirstLine, 0, field, log))
  {
    return;
  }
  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "parse_polynomial; parsing (Polynomial), illegal filter type ('%s')",
                  field);
    return;
  }
  blkt_typ = *field;
  switch (blkt_typ)
  {
  case 'P':
    blkt_ptr->type = POLYNOMIAL;
    break;
  default:
    evalresp_log (log, EV_ERROR, 0,
                  "parse_polynomial; parsing (Polynomial), unexpected filter type ('%c')",
                  *field);
    return;
  }

  check_fld = FirstField + 1;

  if (check_fld == 4)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
  }

  /* next should be the units (in first, then out) */

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* Polynomial Approximation Type */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.approximation_type = field[0];

  /* Valid Frequency Units */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.frequency_units = field[0];

  /* Lower Valid Frequency Bound */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.lower_freq_bound = get_double (field, log);

  /* Upper Valid Frequency Bound */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.upper_freq_bound = get_double (field, log);

  /* Lower Bound of Approximation */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.lower_approx_bound = get_double (field, log);

  /* Upper Bound of Approximation */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.upper_approx_bound = get_double (field, log);

  /* Maximum Absolute Error */
  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  blkt_ptr->blkt_info.polynomial.max_abs_error = get_double (field, log);

  /* the number of coefficients */

  if (0 > get_field (fptr, field, blkt_read, check_fld, ":", 0, log))
  {
    return;
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
    if (0 > get_line (fptr, line, blkt_read, check_fld, " ", log))
    {
      return;
    }
    if (0 > parse_field (line, 1, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "polynomial: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.polynomial.coeffs[i] = atof (field);
    if (0 > parse_field (line, 2, field, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "polynomial: %s%s%s",
                    "coeffs errors must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.polynomial.coeffs_err[i] = atof (field);
  }
}

void
parse_fir (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int i, blkt_typ;
  int blkt_read, check_fld;
  int ncoeffs;
  char field[MAXFLDLEN], line[MAXLINELEN];

  /* first get the stage sequence number (from the input line) */

  if (FirstField != 3 && FirstField != 5)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_fir; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03 or F05",
                  ", fld_found=F", FirstField);
    return;
  }

  if (FirstField == 3)
  {
    blkt_read = 61;
  }
  else
  {
    blkt_read = 41;
  }

  /* set the check-field counter to the next expected field */

  check_fld = FirstField;

  /* then, if first is a B061F03, get the stage sequence number (from FirstLine)
     and the symmetry type (from the file), otherwise, get the symmetry (from FirstLine) */

  if (check_fld == 3)
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return;
    }
    stage_ptr->sequence_no = get_int (field, log);
    curr_seq_no = stage_ptr->sequence_no;
    check_fld += 2;
    if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
    {
      return;
    }
  }
  else
  {
    if (0 > parse_field (FirstLine, 0, field, log))
    {
      return;
    }
    check_fld++;
  }

  /* then get the symmetry type */

  if (strlen (field) != 1)
  {
    evalresp_log (log, EV_ERROR, 0,
                  "parse_fir; parsing (FIR), illegal symmetry type ('%s')",
                  field);
    return;
  }
  blkt_typ = *field;
  switch (blkt_typ)
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
    evalresp_log (log, EV_ERROR, 0,
                  "parse_fir; parsing (FIR), unexpected symmetry type ('%c')",
                  *field);
    return;
  }
  /* next should be the units (in first, then out) */

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->input_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  if (0 > get_line (fptr, line, blkt_read, check_fld++, ":", log))
  {
    return;
  }
  stage_ptr->output_units = check_units (GblChanPtr, line, log);
  if (UNDEF_UNITS == stage_ptr->input_units)
  {
    return;
  }

  /* the number of coefficients */

  if (0 > get_field (fptr, field, blkt_read, check_fld++, ":", 0, log))
  {
    return;
  }
  ncoeffs = get_int (field, log);
  blkt_ptr->blkt_info.fir.ncoeffs = ncoeffs;

  /* remember to allocate enough space for the number of coefficients to follow */

  blkt_ptr->blkt_info.fir.coeffs = alloc_double (ncoeffs, log);

  /* the coefficients */

  for (i = 0; i < ncoeffs; i++)
  {
    if (0 > get_field (fptr, field, blkt_read, check_fld, " ", 1, log))
    {
      return;
    }
    if (!is_real (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_fir: %s%s%s",
                    "coeffs must be real numbers (found '", field, "')");
      return;
    }
    blkt_ptr->blkt_info.fir.coeffs[i] = atof (field);
  }
}

void
parse_ref (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log)
{
  int this_blkt_no = 60, blkt_no, fld_no, i, j, prev_blkt_no = 60;
  int nstages, stage_num, nresps, lcl_nstages;
  char field[MAXFLDLEN];
  evalresp_blkt *last_blkt;
  evalresp_stage *last_stage, *this_stage;

  /* set the filter type for, then start parsing the stages */

  blkt_ptr->type = REFERENCE;

  /* set up a local variable for the input 'stage pointer' */

  this_stage = stage_ptr;

  /* first get the number of stages (from the input line) */

  if (FirstField != 3)
  {
    evalresp_log (log, EV_ERROR, 0, "parse_ref; %s%s%s%2.2d",
                  "(return_field) fld ",
                  "number does not match expected value\n\tfld_xpt=F03",
                  ", fld_found=F", FirstField);
    return;
  }
  if (0 > parse_field (FirstLine, 0, field, log))
  {
    return;
  }
  if (!is_int (field, log))
  {
    evalresp_log (log, EV_ERROR, 0, "parse_ref; value '%s' %s", field,
                  " cannot be converted to the number of stages");
    return;
  }
  nstages = atoi (field);
  blkt_ptr->blkt_info.reference.num_stages = nstages;

  /* then (from the file) read all of the stages in sequence */

  for (i = 0; i < nstages; i++)
  {

    /* determine the stage number in the sequence of stages */

    if (0 > get_field (fptr, field, this_blkt_no, 4, ":", 0, log))
    {
      return;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the stage sequence number");
      return;
    }
    stage_num = atoi (field);
    blkt_ptr->blkt_info.reference.stage_num = stage_num;

    /* set the stage sequence number and the pointer to the first blockette */

    this_stage->sequence_no = stage_num;
    curr_seq_no = this_stage->sequence_no;

    /* then the number of responses in this stage */

    if (0 > get_field (fptr, field, this_blkt_no, 5, ":", 0, log))
    {
      return;
    }
    if (!is_int (field, log))
    {
      evalresp_log (log, EV_ERROR, 0, "parse_ref; value '%s' %s", field,
                    " cannot be converted to the number of responses");
      return;
    }
    nresps = atoi (field);
    blkt_ptr->blkt_info.reference.num_responses = nresps;

    /* then, for each of the responses in this stage, get the first line of the next
         response to determine the type of response to read */

    for (j = 0; j < nresps; j++)
    {
      FirstField = next_line (fptr, FirstLine, &blkt_no, &fld_no, ":", log);
      last_blkt = blkt_ptr;
      switch (blkt_no)
      {
      case 43:
        blkt_ptr = alloc_pz (log);
        parse_pz (fptr, blkt_ptr, this_stage, log);
        break;
      case 44:
        blkt_ptr = alloc_fir (log);
        parse_coeff (fptr, blkt_ptr, this_stage, log);
        break;
      case 45:
        blkt_ptr = alloc_list (log);
        parse_list (fptr, blkt_ptr, this_stage, log);
        break;
      case 46:
        blkt_ptr = alloc_generic (log);
        parse_generic (fptr, blkt_ptr, this_stage, log);
        break;
      case 47:
        blkt_ptr = alloc_deci (log);
        parse_deci (fptr, blkt_ptr, log);
        break;
      case 48:
        blkt_ptr = alloc_gain (log);
        parse_gain (fptr, blkt_ptr, log);
        break;
      case 41:
        blkt_ptr = alloc_fir (log);
        parse_fir (fptr, blkt_ptr, this_stage, log);
        break;
      case 60:
        evalresp_log (log, EV_ERROR, 0,
                      "parse_ref; unexpected end of stage (at blockette [%3.3d])",
                      prev_blkt_no);
        return;
        break;
      default:
        /* code to ignore unexected blockette/field lines might be useful here, but need
                 example code to test it - SBH. 2004.079
                 If we want it, replace this error_return and break with a "continue;"
                 */
        evalresp_log (log, EV_ERROR, 0,
                      "parse_ref; unexpected filter type (blockette [%3.3d])",
                      blkt_no);
        return;
        break;
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

      if (0 > get_field (fptr, field, this_blkt_no, 3, ":", 0, log))
      {
        return;
      }
      if (!is_int (field, log))
      {
        evalresp_log (log, EV_ERROR, 0, "parse_ref; value '%s' %s", field,
                      " cannot be converted to the new stage sequence number");
        return;
      }

      lcl_nstages = atoi (field);
      if (lcl_nstages != nstages)
      {
        evalresp_log (log, EV_ERROR, 0,
                      "parse_ref; internal RESP format error, %s%d%s%d",
                      "\n\tstage expected = ", nstages, ", stage found = ",
                      lcl_nstages);
        return;
      }
      blkt_ptr->blkt_info.reference.num_stages = nstages;
    }
  }
}

int
parse_channel (FILE *fptr, evalresp_channel *chan, evalresp_logger *log)
{

  int blkt_no, read_blkt = 0, fld_no, no_units = 0;
  int curr_seq_no, last_seq_no;
  evalresp_blkt *blkt_ptr, *last_blkt = NULL;
  evalresp_stage *this_stage, *last_stage, *tmp_stage, *tmp_stage2 = NULL;

  /* initialize the channel's sequence of stages */

  last_stage = (evalresp_stage *)NULL;
  curr_seq_no = last_seq_no = 0;
  this_stage = alloc_stage (log);
  chan->first_stage = this_stage;
  chan->nstages++;
  tmp_stage = alloc_stage (log);

  /* start processing the response information */

  while ((FirstField = next_line (fptr, FirstLine, &blkt_no, &fld_no, ":", log)) != 0 && blkt_no != 50)
  {
    switch (blkt_no)
    {
    case 53:
      blkt_ptr = alloc_pz (log);
      parse_pz (fptr, blkt_ptr, tmp_stage, log);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 54:
      /* IGD : as we add an IIR case to this blockette, we cannot simply assume that blockette 54 is FIR */
      /*The field 10 should be distinguish between the IIR and FIR */
      if (is_IIR_coeffs (fptr, ftell (fptr)))
      { /*IGD New IIR case */
        blkt_ptr = alloc_coeff (log);
        parse_iir_coeff (fptr, blkt_ptr, tmp_stage, log);
      }
      else
      { /*IGD this is the original case here */
        blkt_ptr = alloc_fir (log);
        parse_coeff (fptr, blkt_ptr, tmp_stage, log);
      }
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 55:
      blkt_ptr = alloc_list (log);
      parse_list (fptr, blkt_ptr, tmp_stage, log);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 56:
      blkt_ptr = alloc_generic (log);
      parse_generic (fptr, blkt_ptr, tmp_stage, log);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 57:
      blkt_ptr = alloc_deci (log);
      curr_seq_no = parse_deci (fptr, blkt_ptr, log);
      break;
    case 58:
      blkt_ptr = alloc_gain (log);
      curr_seq_no = parse_gain (fptr, blkt_ptr, log);
      break;
    case 60: /* never see a blockette [41], [43]-[48] without a [60], parse_ref handles these */
      blkt_ptr = alloc_ref (log);
      tmp_stage2 = alloc_stage (log);
      parse_ref (fptr, blkt_ptr, tmp_stage2, log);
      curr_seq_no = tmp_stage2->sequence_no;
      tmp_stage2->first_blkt = blkt_ptr;
      break;
    case 61:
      blkt_ptr = alloc_fir (log);
      parse_fir (fptr, blkt_ptr, tmp_stage, log);
      curr_seq_no = tmp_stage->sequence_no;
      break;
    case 62:
      blkt_ptr = alloc_polynomial (log);
      parse_polynomial (fptr, blkt_ptr, tmp_stage, log);
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

/* in_epoch:  determines if an input date-time lies within the response epoch
 defined by the strings "beg_t" and "end_t" */

int
in_epoch (const char *datime, const char *beg_t, const char *end_t)
{
  char *start_pos;
  char temp_str[DATIMLEN];
  int len;
  evalresp_datetime start_time, end_time, this_time;

  /* parse the "datime" argument */

  this_time.hour = this_time.min = 0;
  this_time.sec = 0.0;
  strncpy (temp_str, datime, DATIMLEN);
  start_pos = temp_str;
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  this_time.year = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  this_time.jday = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  len = strcspn (start_pos, ":");
  *(start_pos + len) = '\0';
  this_time.hour = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  len = strcspn (start_pos, ":");
  *(start_pos + len) = '\0';
  this_time.min = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  this_time.sec = atof (start_pos);

  /* parse the "beg_t" argument */

  start_time.hour = start_time.min = 0;
  start_time.sec = 0.0;
  strncpy (temp_str, beg_t, DATIMLEN);
  start_pos = temp_str;
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  start_time.year = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  len = strcspn (start_pos, ",");
  *(start_pos + len) = '\0';
  start_time.jday = atoi (start_pos);
  start_pos += (strlen (start_pos) + 1);
  if (strlen (start_pos))
  {
    len = strcspn (start_pos, ":");
    *(start_pos + len) = '\0';
    start_time.hour = atoi (start_pos);
    start_pos += (strlen (start_pos) + 1);
    if (strlen (start_pos))
    {
      len = strcspn (start_pos, ":");
      *(start_pos + len) = '\0';
      start_time.min = atoi (start_pos);
      start_pos += (strlen (start_pos) + 1);
      if (strlen (start_pos))
      {
        start_time.sec = atof (start_pos);
      }
    }
  }

  /* parse the "end_t" argument.  If there is no ending time, only check to see if
     the start time is before this time */

  if (0 != strncmp (end_t, "No Ending Time", 14))
  { /* Bad bug fixed 06/26/08 */
    end_time.hour = end_time.min = 0;
    end_time.sec = 0.0;
    strncpy (temp_str, end_t, DATIMLEN);
    start_pos = temp_str;
    len = strcspn (start_pos, ",");
    *(start_pos + len) = '\0';
    end_time.year = atoi (start_pos);
    start_pos += (strlen (start_pos) + 1);
    len = strcspn (start_pos, ",");
    *(start_pos + len) = '\0';
    end_time.jday = atoi (start_pos);
    start_pos += (strlen (start_pos) + 1);
    if (strlen (start_pos))
    {
      len = strcspn (start_pos, ":");
      *(start_pos + len) = '\0';
      end_time.hour = atoi (start_pos);
      start_pos += (strlen (start_pos) + 1);
      if (strlen (start_pos))
      {
        len = strcspn (start_pos, ":");
        *(start_pos + len) = '\0';
        end_time.min = atoi (start_pos);
        start_pos += (strlen (start_pos) + 1);
        if (strlen (start_pos))
        {
          end_time.sec = atof (start_pos);
        }
      }
    }
    return ((timecmp (&start_time, &this_time) <= 0 && timecmp (&end_time, &this_time) > 0));
  }
  else
  {
    return ((timecmp (&start_time, &this_time) <= 0));
  }
}
