/* This file is modified by I.Dricker , ISTI, NY 06/21/00 */

/*
 *  8/28/2001 -- [ET]  Added quotes to two 'blockette 55' error messages
 *                     to avoid having an open string traverse into the
 *                     next line.
 */

#include <stdlib.h>
#include "evresp.h"

/* merge_lists:

   a routine that merges two lists filters (blockettes 55).
   The frequencies, amplitudes and phases from the
   second filter are copied into the first filter and the number of
   coefficients in the first filter is adjusted to reflect the new
   filter size.  Then the next_blkt pointer for the first filter is
   reset to the value of the next_blkt pointer of the second filter
   and the space associated with the second filter is free'd.  Finally,
   the second filter pointer is reset to the next_blkt pointer value
   of the first filter

	Modified from merge_coeffs() by Ilya Dricker IGD
	(i.dricker@isti.com)  07/07/00
 */

void merge_lists(struct blkt *first_blkt, struct blkt **second_blkt) {
  int new_ncoeffs, ncoeffs1, ncoeffs2, i, j;
  double *amp1, *amp2, *phase1, *phase2, *freq1, *freq2;
  struct blkt *tmp_blkt;

  tmp_blkt = *second_blkt;
  switch(first_blkt->type) {
  case LIST:
 	   break;
  default:
    error_return(MERGE_ERROR, "merge_lists; filter types must be LIST");
  }

  if(first_blkt->type != tmp_blkt->type)
    error_return(MERGE_ERROR, "merge_lists; both filters must have the same type");

  /* set up some local pointers and values */

  ncoeffs1 = first_blkt->blkt_info.list.nresp;
  
  amp1 = first_blkt->blkt_info.list.amp;
  phase1 = first_blkt->blkt_info.list.phase;
  freq1 = first_blkt->blkt_info.list.freq;

  ncoeffs2 = tmp_blkt->blkt_info.list.nresp;
  amp2 = tmp_blkt->blkt_info.list.amp;
  phase2 = tmp_blkt->blkt_info.list.phase;
  freq2 = tmp_blkt->blkt_info.list.freq;

  new_ncoeffs = ncoeffs1 + ncoeffs2;

  /* attempt to reallocate space for the new (combined) coefficients vector */

  if((amp1 = (double *)realloc(amp1,new_ncoeffs*sizeof(double))) == (double *)NULL)
    error_exit(OUT_OF_MEMORY, "merge_lists; insufficient memory for combined amplitudes");

  if((phase1 = (double *)realloc(phase1,new_ncoeffs*sizeof(double))) == (double *)NULL)
    error_exit(OUT_OF_MEMORY, "merge_lists; insufficient memory for combined phases");

  if((freq1 = (double *)realloc(freq1,new_ncoeffs*sizeof(double))) == (double *)NULL)
    error_exit(OUT_OF_MEMORY, "merge_lists; insufficient memory for combined frequencies");
;

  /* copy the coeff values to the new space */

  for(i = 0, j = (ncoeffs1); i < ncoeffs2; i++, j++) {
    amp1[j] = amp2[i];
    phase1[j]=phase2[i];
    freq1[j]=freq2[i];
  }

  /* set the new values for the combined filter, free second_blkt and reset to the next_blkt
     value for first_blkt (i.e. to the next filter in the sequence) */

  first_blkt->blkt_info.list.nresp = new_ncoeffs;
  first_blkt->blkt_info.list.amp = amp1;
  first_blkt->blkt_info.list.freq = freq1;
  first_blkt->blkt_info.list.phase = phase1;
  first_blkt->next_blkt = tmp_blkt->next_blkt;
  free_fir(tmp_blkt);
  *second_blkt = first_blkt->next_blkt;

}


/* merge_coeffs:

   a routine that merges two fir filters.  The coefficients from the
   second filter are copied into the first filter and the number of
   coefficients in the first filter is adjusted to reflect the new
   filter size.  Then the next_blkt pointer for the first filter is
   reset to the value of the next_blkt pointer of the second filter
   and the space associated with the second filter is free'd.  Finally,
   the second filter pointer is reset to the next_blkt pointer value
   of the first filter */

void merge_coeffs(struct blkt *first_blkt, struct blkt **second_blkt) {
  int new_ncoeffs, ncoeffs1, ncoeffs2, i, j;
  double *coeffs1, *coeffs2;
  struct blkt *tmp_blkt;

  tmp_blkt = *second_blkt;
  switch(first_blkt->type) {
  case FIR_SYM_1:
  case FIR_SYM_2:
  case FIR_ASYM:
    break;
  default:
    error_return(MERGE_ERROR, "merge_coeffs; filter types must be FIR");
  }

  if(first_blkt->type != tmp_blkt->type)
    error_return(MERGE_ERROR, "merge_coeffs; both filters must have the same type");

  /* set up some local pointers and values */

  ncoeffs1 = first_blkt->blkt_info.fir.ncoeffs;
  coeffs1 = first_blkt->blkt_info.fir.coeffs;

  ncoeffs2 = tmp_blkt->blkt_info.fir.ncoeffs;
  coeffs2 = tmp_blkt->blkt_info.fir.coeffs;

  new_ncoeffs = ncoeffs1 + ncoeffs2;

  /* attempt to reallocate space for the new (combined) coefficients vector */

  if((coeffs1 = (double *)realloc(coeffs1,new_ncoeffs*sizeof(double))) == (double *)NULL)
    error_exit(OUT_OF_MEMORY, "merge_coeffs; insufficient memory for combined coeffs");

  /* copy the coeff values to the new space */

  for(i = 0, j = (ncoeffs1); i < ncoeffs2; i++, j++) {
    coeffs1[j] = coeffs2[i];
  }

  /* set the new values for the combined filter, free second_blkt and reset to the next_blkt
     value for first_blkt (i.e. to the next filter in the sequence) */

  first_blkt->blkt_info.fir.ncoeffs = new_ncoeffs;
  first_blkt->blkt_info.fir.coeffs = coeffs1;
  first_blkt->next_blkt = tmp_blkt->next_blkt;
  free_fir(tmp_blkt);
  *second_blkt = first_blkt->next_blkt;

}

/*  check_channel: a routine that checks a channel's filter stages to

    (1) run a sanity check on the filter sequence.
        and that LAPLACE_PZ and ANALOG_PZ filters will be followed
        by a GAIN blockette only.
        REFERENCE blockettes are ignored (since they contain no response information)
    (2) As the routine moves through the stages in the filter sequence,
        several other checks are made.  First, the output units of this
        stage are compared with the input units of the next on to ensure
        that no stages have been skipped.  Second, the filter type of this
        blockette is compared with the filter type of the next. If they
        are the same and have the same stage-sequence number, then they
        are merged to form one filter.  At the present time this is only
        implemented for the FIR type filters (since those are the only filter
        types that typically are continued to a second filter blockette).
        The new filter will have the combined number of coefficients and a
        new vector containing the coefficients for both filters, one after
        the other.
    (3) the expected delay from the FIR filter stages in the channel's
        filter sequence is calculated and stored in the filter structure.

*/

void check_channel(struct channel *chan) {
  struct stage *stage_ptr, *next_stage, *prev_stage;
  struct blkt *blkt_ptr, *next_blkt;
  struct blkt *filt_blkt, *deci_blkt, *gain_blkt, *ref_blkt;
  int stage_type;
  int  gain_flag, deci_flag, ref_flag;
  int i, j, nc, nblkts;

  /* first run a 'sanity-check' of the filter sequence, making sure
     that the units match and that the proper blockettes are found
     where they are 'expected'.  At the same time, continuation filters
     from the FIR filters are merged and out of order filters are
     moved to ensure that the order for blockettes in a stage will be

        FILT_TYPE   ( ->  DECIMATION  )->  GAIN

     where FILT_TYPE is one of ANALOG_PZ, LAPLACE_PZ (which are both called
     PZ_TYPE here), IIR_PZ (called IIR_TYPE here), FIR_ASYM, FIR_SYM1 or
     FIR_SYM2 (which are all called FIR_TYPE in this routine).
     This sequence will order will be established, regardless of the order
     in which the blockettes appeared in the RESP file.  One other 'type' of
     stage is allowed (in addition to the PZ_TYPE and FIR_TYPE):  A 'gain only'
     stage.  This stage type consists of a gain blockette only (with no other
     blockettes).  Use of this type of filter is discouraged, since there is no
     information about units into and out of the stage in a gain blockette, but
     evalresp will allow this type of stage to be used by assuming that the
     units into and out of this stage are identical (i.e. that the units are
     continous between the stage preceeding the 'GAIN_TYPE' stage and the stage
     following it (essentially, this means that 'Units Checking' is skipped
     when this type of stage is encountered).

     Note:  The Decimation stages in any 'type' of filter are optional, except
            for FIR filters, where they are required if there are a non-zero
            number of FIR coefficients of filter or IIR filters with a non-zero
            number of poles or zeros */

  stage_ptr = chan->first_stage;
  prev_stage = (struct stage *)NULL;
  for(i = 0; i < chan->nstages; i++) {
    j = 0;
    next_stage = stage_ptr->next_stage;
    stage_type = gain_flag = deci_flag = ref_flag = 0;
    blkt_ptr = stage_ptr->first_blkt;
    curr_seq_no = stage_ptr->sequence_no;
    while(blkt_ptr) {
      j++;
      next_blkt = blkt_ptr->next_blkt;
      switch (blkt_ptr->type) {
      case ANALOG_PZ:
      case LAPLACE_PZ:
        if(stage_type && stage_type != GAIN_TYPE)
          error_return(ILLEGAL_RESP_FORMAT, "check_channel; %s in stage %d",
                       "more than one filter type",i);
        stage_type = PZ_TYPE;
        filt_blkt = blkt_ptr;
        break;
      case IIR_PZ:
        if(stage_type && stage_type != GAIN_TYPE)
          error_return(ILLEGAL_RESP_FORMAT, "check_channel; %s in stage %d",
                       "more than one filter type",i);
        stage_type = IIR_TYPE;
        filt_blkt = blkt_ptr;
        break;
      case FIR_COEFFS:
        error_return(UNSUPPORT_FILTYPE, "check_channel; unsupported filter type");
      case LIST:
/* IGD         error_return(UNSUPPORT_FILTYPE, "check_channel; unsupported filter type"); */
/* We are going to support this blockette starting with version 3.2.17 IGD */
/* Nevertheless, this support is limited . Clearly, if we allow to mix the blockette 55 with */
/* only selected number of known frequencies with the other blockettes with arbitrary frequency */
/* sampling, there will be a mess */
/* In this version of evalresp (2.3.17), we allow to get a response for the blockette 55 if it is */
/* the only one filter blockette in the response file . To simplify , we allow the program to process */
/* the LIST response if and only if the blkt_ptr->next_blkt == NULL; */

/* First we merge blocketes if the list spans for more than a single blockette */

  	while(next_blkt != (struct blkt *)NULL && next_blkt->type == blkt_ptr->type)
        	merge_lists(blkt_ptr,&next_blkt);
	if (stage_ptr->next_stage != NULL || prev_stage != NULL)
	{
				        error_return(UNSUPPORT_FILTYPE,
						 "blockette 55 cannot be mixed with other filter blockettes\n");
	}
	else
	{
		/* There are still situations which we want to avoid */
		/* In particular, the next stage can be hidden in */
		/* chan->first_stage->next_stage->first_blkt->type */
		/* If it is GAIN, we will continue. If it is not gain, we generate error */

		if ( chan->first_stage->next_stage != NULL)
		{
			if (chan->first_stage->next_stage->first_blkt != NULL)
			{
				if (chan->first_stage->next_stage->first_blkt->type != GAIN)
				        error_return(UNSUPPORT_FILTYPE,
						"blockette 55 cannot be mixed with other filter blockettes\n");
			}
		}
	}
	stage_type = LIST_TYPE;
	filt_blkt = blkt_ptr;
	break;
      case GENERIC:
//        error_return(UNSUPPORT_FILTYPE, "check_channel; unsupported filter type");
	/* IGD 05/16/02 Added support for Generic type */
	if(stage_type && stage_type != GAIN_TYPE)
	        error_return(ILLEGAL_RESP_FORMAT, "check_channel; %s in stage %d",
	               	       "more than one filter type",i+1);
        /* check to see if next blockette(s) is(are) a continuation of this one.
        If so, merge them into one blockette */
	if(next_blkt != (struct blkt *)NULL && next_blkt->type == blkt_ptr->type)
                error_return(ILLEGAL_RESP_FORMAT,
                        "check_channel; multiple 55 blockettes in GENERIC stages are not supported yet");
         stage_type = GENERIC_TYPE;
//	nc = 1; /*for calc_delay to be 0 in decimation blockette */
	fprintf(stdout, "WARNING: Generic blockette is detected in stage %d; content is ignored\n", i+1);
	fflush(stdout);
	filt_blkt = blkt_ptr;
        break;

      case FIR_SYM_1:
      case FIR_SYM_2:
      case FIR_ASYM:
        if(stage_type && stage_type != GAIN_TYPE)
          error_return(ILLEGAL_RESP_FORMAT, "check_channel; %s in stage %d",
                       "more than one filter type",i);

        /* check to see if next blockette(s) is(are) a continuation of this one.
           If so, merge them into one blockette */
        while(next_blkt != (struct blkt *)NULL && next_blkt->type == blkt_ptr->type)
          merge_coeffs(blkt_ptr,&next_blkt);

        /* set the stage type to be FIR_TYPE */
        stage_type = FIR_TYPE;

        /* make FIR filters symmetric if possible */
        if(blkt_ptr->type == FIR_ASYM)
          check_sym(blkt_ptr, chan);

        /* increment the channel delay for this stage using the number of coefficients
           and the filter type */
        if(blkt_ptr->type == FIR_SYM_1)
          nc = (double) blkt_ptr->blkt_info.fir.ncoeffs*2 - 1;
        else if(blkt_ptr->type == FIR_SYM_2)
          nc = (double) blkt_ptr->blkt_info.fir.ncoeffs*2;
        else if(blkt_ptr->type == FIR_ASYM)
          nc = (double) blkt_ptr->blkt_info.fir.ncoeffs;
        filt_blkt = blkt_ptr;
        break;
      case IIR_COEFFS:  /* IGD New type evalresp supports in 3.2.17 */
 	if(stage_type && stage_type != GAIN_TYPE)
	        error_return(ILLEGAL_RESP_FORMAT, "check_channel; %s in stage %d",
	               	       "more than one filter type",i);
        /* check to see if next blockette(s) is(are) a continuation of this one.
        If so, merge them into one blockette */
	if(next_blkt != (struct blkt *)NULL && next_blkt->type == blkt_ptr->type)
                error_return(ILLEGAL_RESP_FORMAT,
                        "check_channel; multiple 55 blockettes in IIR stages are not supported yet");
        /* merge_coeffs(blkt_ptr,&next_blkt);  */ /* Leave it alone for now ! */
        /* set the stage type to be FIR_TYPE */
        stage_type = IIR_COEFFS_TYPE;
	nc = 1; /*for calc_delay to be 0 in decimation blockette */
        filt_blkt = blkt_ptr;
        break;
      case GAIN:
        if(!stage_ptr->sequence_no) {
          chan->sensit = blkt_ptr->blkt_info.gain.gain;
          chan->sensfreq = blkt_ptr->blkt_info.gain.gain_freq;
        }
        if(!stage_type)
          stage_type = GAIN_TYPE;
        gain_flag = j;
        gain_blkt = blkt_ptr;
        break;
      case DECIMATION:
        /* if stage is a FIR filter, increment the estimated delay and applied
           correction for the channel */
        if(stage_type) {
          if(blkt_ptr->type != IIR_PZ) {
	/*    if (stage_type == IIR_COEFFS_TYPE)  ;IGD commented out offending statement 07/17/01*/
            chan->calc_delay += ((nc-1)/2.0) * blkt_ptr->blkt_info.decimation.sample_int;
            chan->estim_delay += (double) blkt_ptr->blkt_info.decimation.estim_delay;
            chan->applied_corr += (double) blkt_ptr->blkt_info.decimation.applied_corr;
          }
          chan->sint = blkt_ptr->blkt_info.decimation.sample_int*
            (double)blkt_ptr->blkt_info.decimation.deci_fact;  /* channel's sint is the last stage's */
        }
        else
          error_return(ILLEGAL_RESP_FORMAT,
                     "check_channel; decimation blockette with no associated filter");
        deci_blkt = blkt_ptr;
        deci_flag = j;
        break;
      case REFERENCE:
        ref_flag = j;
        ref_blkt = blkt_ptr;
        break;
      default:
        error_return(UNSUPPORT_FILTYPE, "check_channel; unrecognized blkt type (type=%d)",
                     blkt_ptr->type);
      }
      blkt_ptr = next_blkt;
    }
    nblkts = j;

    /* If not a 'gain-only' stage, run a 'sanity check' for this stage's filter
       sequence.  Do this by reorganizing the filters (regardless of input
       order) so that they point to each other in the following manner within
       a particular stage:

        STAGE_FIRST_BLKT -> (REF_BLKT) -> FILT_BLKT -> (DECI_BLKT) -> GAIN_BLKT

       where the blockette types are defined above (blockettes given in
       parentheses are optional), the STAGE_FIRST_BLKT indicates the pointer
       to the first blockette for this stage and the REF_BLKT indicates the
       pointer to the REFERENCE blockette, if there is one */
    if(stage_type != GAIN_TYPE) {
      if(ref_flag && deci_flag) {
        stage_ptr->first_blkt = ref_blkt;
        ref_blkt->next_blkt = filt_blkt;
        filt_blkt->next_blkt = deci_blkt;
        deci_blkt->next_blkt = gain_blkt;
        gain_blkt->next_blkt = (struct blkt *)NULL;
      }
      else if(deci_flag) {
        stage_ptr->first_blkt = filt_blkt;
        filt_blkt->next_blkt = deci_blkt;
        deci_blkt->next_blkt = gain_blkt;
        gain_blkt->next_blkt = (struct blkt *)NULL;
      }
      else if(ref_flag) {
        stage_ptr->first_blkt = ref_blkt;
        ref_blkt->next_blkt = filt_blkt;
        filt_blkt->next_blkt = gain_blkt;
        gain_blkt->next_blkt = (struct blkt *)NULL;
      }
      else if(gain_flag) {
        stage_ptr->first_blkt = filt_blkt;
        filt_blkt->next_blkt = gain_blkt;
        gain_blkt->next_blkt = (struct blkt *)NULL;
      }
    }

    /* check for mismatch in units between this stage and the previous one
       (or in the case where this is a gain-only stage, between the next stage
       and the previous stage).  If it is a gain-only stage, this check will be
       skipped and the 'prev_stage' pointer will be left in it's existing
       position (essentially, this 'disables' the units check for 'gain-only'
       stages */
	/* IGD in version 3.2.17, there are two new stage types are in the next check */
    if(stage_type == PZ_TYPE || stage_type == FIR_TYPE ||
	stage_type == IIR_TYPE || stage_type == IIR_COEFFS_TYPE || stage_type == LIST_TYPE) {
      if(prev_stage != (struct stage *)NULL && prev_stage->output_units !=
         stage_ptr->input_units)
        error_return(ILLEGAL_RESP_FORMAT, "check_channel; units mismatch between stages");
    }

    /* if the main filter type is either FIR with a non-zero number of coefficients
       or IIR with a non-zero number of poles or zeros, then it is a fatal error
       if the following blockette for this stage is not a decimation blockette
       (i.e. if the deci_flag has not been set) */

    /* IGD : new stage type IIR_COEFFS_TYPE is processed in v 3.2.17 */
    if((stage_type == IIR_TYPE || stage_type == FIR_TYPE || stage_type == IIR_COEFFS_TYPE) && !deci_flag)
        error_return(ILLEGAL_RESP_FORMAT, "check_channel; required decimation blockette for IIR or FIR filter missing");

    /* if this wasn't a gain-only stage, save it for comparison of units between
       stages.  Otherwise, keep the previous stage from this iteration and move
       on to the next stage */
    if(stage_type != GAIN_TYPE) {
      if(stage_ptr->sequence_no)
        prev_stage = stage_ptr;
    }
    stage_ptr = next_stage;
  }
}


/* check_sym: checks to see if a FIR filter can be converted to a symmetric FIR
              filter, if so, the conversion is made and the filter type is redefined */

void check_sym(struct blkt *f, struct channel *chan) {
  int nc, n0, k;
  double sum = 0.0;

  nc = f->blkt_info.fir.ncoeffs;

  /* CHECK IF IF FILTER IS NORMALIZED TO 1 AT FREQ 0 */

  for (k=0; k<nc; k++) sum += f->blkt_info.fir.coeffs[k];
  if (nc && (sum < (1.0-FIR_NORM_TOL) || sum > (1.0+FIR_NORM_TOL))) {
    fprintf(stderr,"WARNING: FIR normalized: sum[coef]=%E; ", sum);
    fprintf(stderr,"%s %s %s %s\n", chan->network, chan->staname, chan->locid, chan->chaname);
    fflush(stderr);
    for (k=0; k<nc; k++) f->blkt_info.fir.coeffs[k] /= sum;
  }

  if (f->type != FIR_ASYM) return;

  /* CHECK IF FILTER IS SYMETRICAL WITH EVEN NUM OF WEIGHTS */

  if ((nc%2) == 0) {
    n0 = nc / 2;
    for (k=0; k < n0; k++) {
      if (f->blkt_info.fir.coeffs[n0+k] != f->blkt_info.fir.coeffs[n0-k-1]) return;
    }
    f->type = FIR_SYM_2;
    f->blkt_info.fir.ncoeffs = n0;
  }

  /* CHECK IF FILTER IS SYMETRICAL WITH ODD NUM OF WEIGHTS */

  else {
    n0 = (nc - 1) / 2;
    for (k=1; k<nc-n0; k++) {
      if (f->blkt_info.fir.coeffs[n0+k] != f->blkt_info.fir.coeffs[n0-k]) return;
    }
    f->type = FIR_SYM_1;
    f->blkt_info.fir.ncoeffs =  nc-n0;
  }
}

/* IGD 08/03/02 Added support for B55 interpolation */

/*********** function free for freeing of double arrays *******************************************/

void sscdns_free_double (double *array)
{
  if (array != NULL)
  {
    free (array);
    array = NULL;
  }
}

#ifdef B55_INTRPL
/* functions for reading and interpolation of 55 blockette  */
/**************************************************************************************************/
double do_interpolation (double value, double x1, double x2,
double y1, double y2)
{
  double a, c;
  double b;
  a = (y1 - y2)/(x1-x2);
  b = (x1*y2 - y1*x2)/(x1 - x2);
  c = a*value+b;
  return c;
}
/**********************************************************************************************************/
/* This functions reads in the data from frequency/amplitude/spectra and
 * the parameters of resampling. It then does a linear interpolation of the
 * three input arrays using the new sampling paramters and saves the output
 * into the input array (reallocating memory if needed).
 * Functions returns the number of samples in the newly created array or
 * negative value indicating the error.
 */
/*************************************************************************************************/
int interpolate_spectra(double **frequency_ptr,
                               double **amplitude_ptr,
			       double **phase_ptr,
                               int number_responses,
			       double frequency_start,
			       double frequency_step,
			       double frequency_end)
{
  int         frequencyCounter;
  int         i;
  int         i_start = 0;
  int         number_samples;
  double      frequency_value;
  double      *spare_frequency;
  double      *spare_amplitude;
  double      *spare_phase;
  double      *frequency;
  double      *amplitude;
  double      *phase;

  frequency = *frequency_ptr;
  amplitude = *amplitude_ptr;
  phase = *phase_ptr;
  number_samples = (frequency_end-frequency_start)/frequency_step+1;
  spare_frequency = (double *) calloc (1, (number_samples) * sizeof (double));
  spare_amplitude = (double *) calloc (1, (number_samples) * sizeof (double));
  spare_phase = (double *) calloc (1, (number_samples) * sizeof (double));

  for (frequencyCounter = 0; frequencyCounter <= number_samples-1; frequencyCounter++)
  {
    frequency_value = frequency_start + frequencyCounter * frequency_step;
    if (frequency_value < frequency[0])
    {
     spare_amplitude [frequencyCounter] = 0.00;
     spare_phase [frequencyCounter] = 1.00;
     spare_frequency[frequencyCounter] = frequency_value;
    }
    else
    {
      for (i = i_start; i <= number_responses-1; i++)
      {
       if (frequency_value>= frequency [i] && frequency_value <= frequency [i+1])
       {
          spare_amplitude [frequencyCounter] = do_interpolation (frequency_value, frequency[i+1], frequency[i],
          amplitude [i+1], amplitude [i]);
          spare_phase [frequencyCounter]=do_interpolation (frequency_value, frequency[i+1], frequency[i],
          phase [i+1], phase [i]);
          spare_frequency[frequencyCounter] = frequency_value;

          i_start = i;
          break;
        }
        else
        {
          if (frequency_value >= frequency[number_responses-1])
          {
            spare_amplitude [frequencyCounter]= 0.00;
            spare_frequency[frequencyCounter] = frequency_value;
            spare_phase [frequencyCounter] = phase[number_responses-1];
            break;
           }
         }
        }
       }
    }
    sscdns_free_double (frequency);
    sscdns_free_double (amplitude);
    sscdns_free_double (phase);

    frequency = (double *) calloc (number_samples, sizeof (double));
    amplitude = (double *) calloc(number_samples, sizeof (double));
    phase = (double *) calloc(number_samples, sizeof (double));

    *frequency_ptr = frequency;
    *amplitude_ptr = amplitude;
    *phase_ptr = phase;

    memcpy(frequency, spare_frequency, number_samples * sizeof(double));
    memcpy(amplitude, spare_amplitude, number_samples * sizeof(double));
    memcpy(phase, spare_phase, number_samples * sizeof(double));

    sscdns_free_double (spare_frequency);
    sscdns_free_double (spare_amplitude);
    sscdns_free_double (spare_phase);
    return number_samples;
}
/**********************************************************************************************************/
#endif
