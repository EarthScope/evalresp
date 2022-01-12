
/* NEEDED for M_PI on windows */
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "evalresp/public_api.h"

int
evalresp_response_to_char (evalresp_logger *log, const evalresp_response *response,
                           int unwrap, evalresp_file_format format, char **output)
{
  int status = EVALRESP_OK;
  int num_of_points = 0;
  int i, length, offset;
  double added_value = 0, prev_phase = 0;
  double *amp_arr = NULL;
  double *pha_arr = NULL;
  double *freq_arr = NULL;

  if (*output)
  {
    /* don't want to get bad memory location to write to
       or we don't want to zombify stuff */
    evalresp_log (log, EV_ERROR, EV_ERROR, "cannot out put to an already allocated output");
    return EVALRESP_ERR;
  }

  if (!response)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot Process Empty Response");
    return EVALRESP_ERR;
  }

  num_of_points = response->nfreqs;
  freq_arr = response->freqs;

  switch (format)
  {
  case evalresp_fap_file_format:
    if (!(status = calloc_doubles (log, "amplitude", num_of_points, &amp_arr)))
    {
      status = calloc_doubles (log, "phase", num_of_points, &pha_arr);
    }
    break;
  case evalresp_amplitude_file_format:
    status = calloc_doubles (log, "amplitude", num_of_points, &amp_arr);
    break;
  case evalresp_phase_file_format:
    status = calloc_doubles (log, "phase", num_of_points, &pha_arr);
    break;
  case evalresp_complex_file_format:
    break;
  default:
    evalresp_log (log, EV_ERROR, EV_ERROR, "Invalid format sent to evalresp_response_to_char");
    status = EVALRESP_ERR;
  }

  if (!status)
  {
    for (i = 0; i < num_of_points; i++)
    {
      if (amp_arr)
      {
        amp_arr[i] = sqrt (response->rvec[i].real * response->rvec[i].real + response->rvec[i].imag * response->rvec[i].imag);
      }
      if (pha_arr)
      {
        pha_arr[i] = atan2 (response->rvec[i].imag, response->rvec[i].real + 1.e-200) * 180.0 / M_PI;
        if (unwrap)
        {
          if (i == 0) /* force initial phase to [0,360) */
          {
            while (pha_arr[0] + added_value < 0)
              added_value += 360;
            while (pha_arr[0] + added_value >= 360)
              added_value -= 360;
            prev_phase = pha_arr[0] + added_value;
          }
          pha_arr[i] = unwrap_phase (pha_arr[i], prev_phase, 360, &added_value);
          prev_phase = pha_arr[i];
        }
      }
    }
    /* Get Size of output string */
    for (i = 0, length = 0; i < num_of_points; i++)
    {
      switch (format)
      {
      case evalresp_fap_file_format:
        length += _evalresp_snprintf (NULL, 0, "%.6E %.6E %.6E\n", freq_arr[i], amp_arr[i], pha_arr[i]);
        break;
      case evalresp_amplitude_file_format:
        length += _evalresp_snprintf (NULL, 0, "%.6E %.6E\n", freq_arr[i], amp_arr[i]);
        break;
      case evalresp_phase_file_format:
        length += _evalresp_snprintf (NULL, 0, "%.6E %.6E\n", freq_arr[i], pha_arr[i]);
        break;
      case evalresp_complex_file_format:
        length += _evalresp_snprintf (NULL, 0, "%.6E  %.6E  %.6E\n", freq_arr[i], response->rvec[i].real, response->rvec[i].imag);
        break;
      }
    }
    length++; /* Adding one for NULL termination */
  }

  if (!status)
  {
    if (!(*output = (char *)calloc (length, sizeof (char))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate output");
      status = EVALRESP_MEM;
    }
  }

  if (!status)
  {
    for (i = 0, offset = 0; i < num_of_points; i++)
    {
      switch (format)
      {
      case evalresp_fap_file_format:
        offset += _evalresp_snprintf (*output + offset, length - offset, "%.6E %.6E %.6E\n", freq_arr[i], amp_arr[i], pha_arr[i]);
        break;
      case evalresp_amplitude_file_format:
        offset += _evalresp_snprintf (*output + offset, length - offset, "%.6E %.6E\n", freq_arr[i], amp_arr[i]);
        break;
      case evalresp_phase_file_format:
        offset += _evalresp_snprintf (*output + offset, length - offset, "%.6E %.6E\n", freq_arr[i], pha_arr[i]);
        break;
      case evalresp_complex_file_format:
        offset += _evalresp_snprintf (*output + offset, length - offset, "%.6E  %.6E  %.6E\n", freq_arr[i], response->rvec[i].real, response->rvec[i].imag);
        break;
      }
    }
  }

  free (amp_arr);
  free (pha_arr);

  return status;
}

int
evalresp_response_to_stream (evalresp_logger *log, const evalresp_response *response,
                             int unwrap, evalresp_file_format format, FILE *const file)
{
  char *resp_string = NULL;
  int status = EVALRESP_OK, len;

  /* need to check for valid FILE subsequent calls handle other error checks */
  if (!file)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "the stream is not open");
    status = EVALRESP_ERR;
  }
  else
  {
    /* get the output string */
    if (!(status = evalresp_response_to_char (log, response, unwrap, format, &resp_string)))
    {
      /* print to the FILE * */
      len = fprintf (file, "%s", resp_string);
      if (len != strlen (resp_string))
      {
        evalresp_log (log, EV_ERROR, EV_ERROR, "Failed to write to file");
        status = EVALRESP_IO;
      }
    }
  }

  /* clean up the string */
  free (resp_string);
  return status;
}

int
evalresp_response_to_file (evalresp_logger *log, const evalresp_response *response,
                           int unwrap, evalresp_file_format format, const char *filename)
{
  int status = EVALRESP_OK;
  FILE *file;

  /* check that a valid filename is sent values are checked in called functions */
  if (!filename)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Empty file name");
    status = EVALRESP_ERR;
  }
  else
  {
    /* open file and check that it did open */
    if (!(file = fopen (filename, "w")))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "could not open output file %s", filename);
      status = EVALRESP_IO;
    }
    else
    {
      status = evalresp_response_to_stream (log, response, unwrap, format, file);
      /* clean up NOTE: file should not be NULL at this point */
      fclose (file);
    }
  }

  return status;
}

int
evalresp_channel_to_log (evalresp_logger *log, evalresp_options const *const options, evalresp_channel *const channel)
{
  evalresp_stage *this_stage, *last_stage, *first_stage;
  evalresp_blkt *this_blkt;
  char tmp_str[TMPSTRLEN], out_str[OUTPUTLEN];
  int in_units = 0;
  char *in_units_str = 0;
  char *out_units_str = 0;
  int first_blkt;

  /* determine what the input units of the first stage and output units
     of the last stage are */

  this_stage = first_stage = channel->first_stage;
  while (this_stage)
  {
    if (options->start_stage >= 0 && options->stop_stage && (this_stage->sequence_no < options->start_stage || this_stage->sequence_no > options->stop_stage))
    {
      this_stage = this_stage->next_stage;
      continue;
    }
    else if (options->start_stage >= 0 && !options->stop_stage && this_stage->sequence_no != options->start_stage)
    {
      this_stage = this_stage->next_stage;
      continue;
    }
    last_stage = this_stage;
    if (!in_units || !in_units_str)
    {
      in_units = this_stage->input_units;
      in_units_str = this_stage->input_units_str;
    }
    if (last_stage->output_units || last_stage->output_units_str)
    {
      out_units_str = last_stage->output_units_str;
    }
    this_stage = last_stage->next_stage;
    if (this_stage != (evalresp_stage *)NULL)
      this_blkt = this_stage->first_blkt;
  }

  /* inform the user of which file has been evaluated */

  evalresp_log (log, EV_INFO, 0, " --------------------------------------------------");
  if (!options->use_stdio)
  {
    evalresp_log (log, EV_INFO, 0, "  %s", options->filename);
  }
  else
  {
    if (strlen (channel->network))
    {
      evalresp_log (log, EV_INFO, 0, "  RESP.%s.%s.%s.%s (from stdin)",
                    channel->network, channel->staname, channel->locid, channel->chaname);
    }
    else
    {
      evalresp_log (log, EV_INFO, 0, "  RESP..%s.%s.%s (from stdin)",
                    channel->staname, channel->locid, channel->chaname);
    }
  }
  evalresp_log (log, EV_INFO, 0, " --------------------------------------------------");
  evalresp_log (log, EV_INFO, 0, "  %s %s %s %s ",
                (strlen (channel->network) ? channel->network : "??"), channel->staname,
                (strlen (channel->locid) ? channel->locid : "??"), channel->chaname);
  evalresp_log (log, EV_INFO, 0, "  %s %s", channel->beg_t, channel->end_t);

  evalresp_log (log, EV_INFO, 0, "   documented input units: %s", (in_units_str) ? in_units_str : "None");
  evalresp_log (log, EV_INFO, 0, "   documented output units: %s", (out_units_str) ? out_units_str : "None");
  evalresp_log (log, EV_INFO, 0, "   requested units: %s", evalresp_unit_string(options->unit));

  evalresp_log (log, EV_INFO, 0, "   computed sens=%.5E (reported=%.5E) @ %.5E Hz",
                channel->calc_sensit, channel->sensit, channel->sensfreq);
  evalresp_log (log, EV_INFO, 0,
                "   calc_del=%.5E  corr_app=%.5E  est_delay=%.5E  final_sint=%.3g(sec/sample)",
                channel->calc_delay, channel->applied_corr, channel->estim_delay,
                channel->sint);
  if (1 == options->use_total_sensitivity)
  {
    evalresp_log (log, EV_INFO, 0,
                  "   (reported sensitivity was used to compute response (-ts option enabled))");
  }

  /* then print the parameters for each stage (stage number, type of stage, number
     of coefficients [or number of poles and zeros], gain, and input sample interval
     if it is defined for that stage */

  this_stage = first_stage;
  while (this_stage)
  {
    if (options->start_stage >= 0 && options->stop_stage && (this_stage->sequence_no < options->start_stage || this_stage->sequence_no > options->stop_stage))
    {
      this_stage = this_stage->next_stage;
      continue;
    }
    else if (options->start_stage >= 0 && !options->stop_stage && this_stage->sequence_no != options->start_stage)
    {
      this_stage = this_stage->next_stage;
      continue;
    }
    this_blkt = this_stage->first_blkt;
    if (this_stage->sequence_no)
    {
      strncpy (tmp_str, "", TMPSTRLEN);
      sprintf (tmp_str, "     stage %2d:", this_stage->sequence_no);
      strcpy (out_str, tmp_str);
    }
    first_blkt = 1;
    while (this_blkt)
    {
      strncpy (tmp_str, "", TMPSTRLEN);
      switch (this_blkt->type)
      {
      case POLYNOMIAL:
        sprintf (tmp_str, " POLYNOMIAL     MacLaurin Coeffs= %2d",
                 this_blkt->blkt_info.polynomial.ncoeffs);
        break;
      case LAPLACE_PZ:
        sprintf (tmp_str, " LAPLACE     A0=%E NZeros= %2d NPoles= %2d",
                 this_blkt->blkt_info.pole_zero.a0,
                 this_blkt->blkt_info.pole_zero.nzeros,
                 this_blkt->blkt_info.pole_zero.npoles);
        break;
      case ANALOG_PZ:
        sprintf (tmp_str, " ANALOG      A0=%E NZeros= %2d NPoles= %2d",
                 this_blkt->blkt_info.pole_zero.a0,
                 this_blkt->blkt_info.pole_zero.nzeros,
                 this_blkt->blkt_info.pole_zero.npoles);
        break;
      case FIR_SYM_1:
        sprintf (tmp_str, " FIR_SYM_1   H0=%E Ncoeff=%3d",
                 this_blkt->blkt_info.fir.h0,
                 this_blkt->blkt_info.fir.ncoeffs * 2 - 1);
        break;
      case FIR_SYM_2:
        sprintf (tmp_str, " FIR_SYM_2   H0=%E Ncoeff=%3d",
                 this_blkt->blkt_info.fir.h0,
                 this_blkt->blkt_info.fir.ncoeffs * 2);
        strcat (out_str, tmp_str);
        strncpy (tmp_str, "", TMPSTRLEN);
        break;
      case FIR_ASYM:
        sprintf (tmp_str, " FIR_ASYM    H0=%E Ncoeff=%3d",
                 this_blkt->blkt_info.fir.h0,
                 this_blkt->blkt_info.fir.ncoeffs);
        break;
      case IIR_PZ:
        sprintf (tmp_str, " IIR_PZ      A0=%E NZeros= %2d NPoles= %2d",
                 this_blkt->blkt_info.pole_zero.a0,
                 this_blkt->blkt_info.pole_zero.nzeros,
                 this_blkt->blkt_info.pole_zero.npoles);
        break;
      case IIR_COEFFS:
        sprintf (tmp_str, "IIR_COEFFS   H0=%E NNumers=%2d NDenums= %2d",
                 this_blkt->blkt_info.coeff.h0,
                 this_blkt->blkt_info.coeff.nnumer,
                 this_blkt->blkt_info.coeff.ndenom);
        break;
      case GAIN:
        if (first_blkt && this_stage->sequence_no)
          sprintf (tmp_str, " GAIN        Sd=%E",
                   this_blkt->blkt_info.gain.gain);
        else if (this_stage->sequence_no)
          sprintf (tmp_str, " Sd=%E", this_blkt->blkt_info.gain.gain);
        break;
      case DECIMATION:
        sprintf (tmp_str, " SamInt=%E",
                 this_blkt->blkt_info.decimation.sample_int);
        if (this_blkt->blkt_info.decimation.applied_corr < 0)
        {
          evalresp_log (log, EV_WARN, 0,
                        " WARNING Stage %d: Negative correction_applied=%.5E is likely to be incorrect\n",
                        this_stage->sequence_no,
                        this_blkt->blkt_info.decimation.applied_corr);
        }
        if (this_blkt->blkt_info.decimation.estim_delay < 0)
        {
          evalresp_log (log, EV_WARN, 0,
                        " WARNING Stage %d: Negative estimated_delay=%.5E is likely to be incorrect",
                        this_stage->sequence_no,
                        this_blkt->blkt_info.decimation.estim_delay);
        }
        break;
      case GENERIC:
        sprintf (tmp_str, " Generic blockette is ignored; ");
        break;

      case FIR_COEFFS:
      case LIST:
      case REFERENCE:
        break;
      default:
        evalresp_log (log, EV_INFO, 0, " .........");
      }
      strcat (out_str, tmp_str);
      if (first_blkt)
      {
        first_blkt = 0;
      }
      this_blkt = this_blkt->next_blkt;
    }
    if (this_stage->sequence_no)
    {
      evalresp_log (log, EV_INFO, 0, " %s", out_str);
    }
    this_stage = this_stage->next_stage;
  }
  evalresp_log (log, EV_INFO, 0, "--------------------------------------------------");
  /* IGD : here we print a notice about blockette 55: evalresp v. 2.3.17+*/
  /* ET:  Notice modified, with different notice if freqs interpolated */
  if (channel->first_stage->first_blkt->type == LIST)
  {
    if (options->b55_interpolate)
    {
      evalresp_log (log, EV_INFO, 0,
                    " Note:  The input has been interpolated from the response List stage");
      evalresp_log (log, EV_INFO, 0,
                    " (blockette 55) to generate output for the %d frequencies requested",
                    channel->first_stage->first_blkt->blkt_info.list.nresp);
    }
    else if (options->b55_interpolate)
    {
      evalresp_log (log, EV_INFO, 0,
                    " Note:  The output has been interpolated from the %d frequencies",
                    channel->first_stage->first_blkt->blkt_info.list.nresp);
      evalresp_log (log, EV_INFO, 0,
                    " defined in the response List stage (blockette 55)");
    }
    else
    {
      evalresp_log (log, EV_WARN, 0,
                    " Response contains a List stage (blockette 55)--the output has");
      evalresp_log (log, EV_WARN, 0,
                    " been generated for those %d frequencies defined in the blockette",
                    channel->first_stage->first_blkt->blkt_info.list.nresp);
    }
  }

  return EVALRESP_OK;
}
