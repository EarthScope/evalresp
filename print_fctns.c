#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "./evresp.h"
#include <string.h>
/* print_chan:  prints a summary of the channel's response information to stderr */

void
	evresp_adjust_phase(double *pha, int len, double min, double max);

int
	evresp_vector_minmax(double *pha, int len, double *min, double *max);

void print_chan(struct channel *chan, int start_stage, int stop_stage, int stdio_flag) {
  struct stage *this_stage, *last_stage, *first_stage;
  struct blkt *this_blkt;
  char tmp_str[TMPSTRLEN], out_str[OUTPUTLEN];
  int in_units = 0;
  int out_units = 0;
  int first_blkt;

  /* determine what the input units of the first stage and output units
     of the last stage are */

  this_stage = first_stage = chan->first_stage;
  while(this_stage) {
    if(start_stage >= 0 && stop_stage && (this_stage->sequence_no < start_stage ||
       this_stage->sequence_no > stop_stage)) {
      this_stage = this_stage->next_stage;
      continue;
    }
    else if(start_stage >= 0 && !stop_stage && this_stage->sequence_no != start_stage) {
      this_stage = this_stage->next_stage;
      continue;
    }
    last_stage = this_stage;
    if(!in_units) in_units = this_stage->input_units;
    if(last_stage->output_units)
      out_units = last_stage->output_units;
    this_stage = last_stage->next_stage;
    if(this_stage != (struct stage *)NULL)
      this_blkt = this_stage->first_blkt;
  }

  /* inform the user of which file has been evaluated */

  fprintf(stderr, "--------------------------------------------------\n");
  if(!stdio_flag) {
    fprintf(stderr, "  %s\n", curr_file);
  }
  else {
    if(strlen(chan->network)) {
      fprintf(stderr, "  RESP.%s.%s.%s.%s (from stdin)\n",chan->network,
                      chan->staname,chan->locid,chan->chaname);
    }
    else {
      fprintf(stderr, "  RESP..%s.%s.%s (from stdin)\n",chan->staname,
                      chan->locid,chan->chaname);
    }
  }
  fprintf(stderr, "--------------------------------------------------\n");
  fprintf(stderr, "  %s %s %s %s ", (strlen(chan->network) ? chan->network : "??"),
          chan->staname, (strlen(chan->locid) ? chan->locid : "??"), chan->chaname);
  if(!def_units_flag)
    fprintf(stderr, "%s %s\n   Seed units: %s(in)->%s(out)\n",
            chan->beg_t, chan->end_t,SEEDUNITS[in_units],
            SEEDUNITS[out_units]);
  else
    fprintf(stderr, "%s %s\n   Seed units: %s(in)->%s(out)\n",
            chan->beg_t, chan->end_t,chan->first_units,
            chan->last_units);

  fprintf(stderr, "   computed sens=%.5E (reported=%.5E) @ %.5E Hz\n",
          chan->calc_sensit, chan->sensit, chan->sensfreq);
  fprintf(stderr, "   calc_del=%.5E  corr_app=%.5E  est_delay=%.5E  final_sint=%.3g(sec/sample)\n",
          chan->calc_delay, chan->applied_corr, chan->estim_delay, chan->sint);
#ifdef USE_DELAY
  fprintf(stderr, "      NOTE: Estimated delay was used in computation of PHASE\n");
#endif
  /* then print the parameters for each stage (stage number, type of stage, number
     of coefficients [or number of poles and zeros], gain, and input sample interval
     if it is defined for that stage */

  this_stage = first_stage;
  while(this_stage) {
    if(start_stage >= 0 && stop_stage && (this_stage->sequence_no < start_stage ||
       this_stage->sequence_no > stop_stage)) {
      this_stage = this_stage->next_stage;
      continue;
    }
    else if(start_stage >= 0 && !stop_stage && this_stage->sequence_no != start_stage) {
      this_stage = this_stage->next_stage;
      continue;
    }
    this_blkt = this_stage->first_blkt;
    if(this_stage->sequence_no) {
      strncpy(tmp_str,"",TMPSTRLEN);
      sprintf(tmp_str,"     stage %2d:",this_stage->sequence_no);
      strcpy(out_str,tmp_str);
    }
    first_blkt = 1;
    while(this_blkt) {
      strncpy(tmp_str,"",TMPSTRLEN);
      switch (this_blkt->type) {
      case LAPLACE_PZ:
        sprintf(tmp_str," LAPLACE     A0=%E NZeros= %2d NPoles= %2d",
                  this_blkt->blkt_info.pole_zero.a0,
                  this_blkt->blkt_info.pole_zero.nzeros,
                  this_blkt->blkt_info.pole_zero.npoles);
        break;
      case ANALOG_PZ:
        sprintf(tmp_str," ANALOG      A0=%E NZeros= %2d NPoles= %2d",
                          this_blkt->blkt_info.pole_zero.a0,
                          this_blkt->blkt_info.pole_zero.nzeros,
                          this_blkt->blkt_info.pole_zero.npoles);
        break;
      case FIR_SYM_1:
        sprintf(tmp_str," FIR_SYM_1   H0=%E Ncoeff=%3d",
                          this_blkt->blkt_info.fir.h0,
                          this_blkt->blkt_info.fir.ncoeffs*2-1);
        break;
     case FIR_SYM_2:
        sprintf(tmp_str," FIR_SYM_2   H0=%E Ncoeff=%3d",
                          this_blkt->blkt_info.fir.h0,
                          this_blkt->blkt_info.fir.ncoeffs*2);
        strcat(out_str,tmp_str);
        strncpy(tmp_str,"",TMPSTRLEN);
        break;
      case FIR_ASYM:
        sprintf(tmp_str," FIR_ASYM    H0=%E Ncoeff=%3d",
                          this_blkt->blkt_info.fir.h0,
                          this_blkt->blkt_info.fir.ncoeffs);
        break;
      case IIR_PZ:
        sprintf(tmp_str," IIR_PZ      A0=%E NZeros= %2d NPoles= %2d",
                          this_blkt->blkt_info.pole_zero.a0,
                          this_blkt->blkt_info.pole_zero.nzeros,
                          this_blkt->blkt_info.pole_zero.npoles);
	break;
      case IIR_COEFFS:
	sprintf(tmp_str, "IIR_COEFFS   H0=%E NNumers=%2d NDenums= %2d",
			  this_blkt->blkt_info.coeff.h0,
                          this_blkt->blkt_info.coeff.nnumer,
                          this_blkt->blkt_info.coeff.ndenom);
        break;
      case GAIN:
        if(first_blkt && this_stage->sequence_no)
          sprintf(tmp_str," GAIN        Sd=%E",this_blkt->blkt_info.gain.gain);
        else if(this_stage->sequence_no)
          sprintf(tmp_str," Sd=%E",this_blkt->blkt_info.gain.gain);
        break;
      case DECIMATION:
        sprintf(tmp_str," SamInt=%E",this_blkt->blkt_info.decimation.sample_int);
        break;
      case GENERIC:
        sprintf(tmp_str," Generic blockette is ignored; ");
        break;

      case FIR_COEFFS:
      case LIST:
      case REFERENCE:
        break;
      default:
        fprintf(stderr, ".........");
      }
      strcat(out_str,tmp_str);
      if(first_blkt)
        first_blkt = 0;
      this_blkt = this_blkt->next_blkt;
    }
    if(this_stage->sequence_no)
      fprintf(stderr,"%s\n",out_str);
    this_stage = this_stage->next_stage;
  }
  fprintf(stderr, "--------------------------------------------------\n");
  /* IGD : here we print a notice about blockette 55: evalresp v. 2.3.17+*/
  if ( chan->first_stage->first_blkt->type == LIST)	{
	fprintf(stderr, "++++++++ WARNING ++++++++++++++++++++++++++++\n");
	fprintf(stderr, "Response information  is extracted from  the blockette 55\n");
	fprintf(stderr, "which 'is not an acceptable response description' [SEED manual]\n");
	fprintf (stderr, "The response is computed ONLY for those %d frequencies from the",
				chan->first_stage->first_blkt->blkt_info.list.nresp); 
	fprintf (stderr, " input file\n");
	fprintf(stderr, "+++++++++++++++++++++++++++++++++++++++++++++\n");


  }
  fflush(stderr);
}

/* print_resp:  prints the response information in the fashion that the
                user requested it.  The response is either in the form of
                a complex spectra (freq, real_resp, imag_resp) to the
                file SPECTRA.NETID.STANAME.CHANAME (if rtype = "cs")
                or in the form of seperate amplitude and phase files
                (if rtype = "ap") with names like AMP.NETID.STANAME.CHANAME
                and PHASE.NETID.STANAME.CHANAME.  In all cases, the pointer to
                the channel is used to obtain the NETID, STANAME, and CHANAME
                values.  If the 'stdio_flag' is set to 1, then the response
                information will be output to stdout, prefixed by a header that
                includes the NETID, STANAME, and CHANAME, as well as whether
                the response given is in amplitude/phase or complex response
                (real/imaginary) values.  If either case, the output to stdout
                will be in the form of three columns of real numbers, in the
                former case they will be freq/amp/phase tuples, in the latter
                case freq/real/imaginary tuples. */

void print_resp(double *freqs, int nfreqs, struct response *first,
                char *rtype, int stdio_flag) {
  int i;
  double mod, pha;
  char filename[MAXLINELEN];
  FILE *fptr1, *fptr2;
  struct response *resp;
  struct complex *output;
#ifdef UNWRAP_PHASE
  double range = 360;
  double added_value = 0;
  double prev_phase = 0;
  double *phaArray;
  double *modArray;
#endif

  resp = first;
  while(resp != (struct response *)NULL) {
    output = resp->rvec;
    nfreqs = resp->nfreqs; /*IGD for v 3.2.17 of evalresp to support blockette 55 */
    freqs = resp->freqs;
    if(!strcmp(rtype,"AP")) {
      if(!stdio_flag) {
        sprintf(filename,"AMP.%s.%s.%s.%s",resp->network,resp->station,resp->locid,resp->channel);
        if((fptr1 = fopen(filename,"w")) == (FILE *)NULL) {
          error_exit(OPEN_FILE_ERROR,"print_resp; failed to open file %s", filename);
        }
        sprintf(filename,"PHASE.%s.%s.%s.%s",resp->network,resp->station,resp->locid,resp->channel);
        if((fptr2 = fopen(filename,"w")) == (FILE *)NULL) {
          error_exit(OPEN_FILE_ERROR,"print_resp; failed to open file %s", filename);
        }
#ifdef UNWRAP_PHASE
	phaArray = (double *) calloc(nfreqs, sizeof(double));
	modArray = (double *) calloc(nfreqs, sizeof(double));
#endif
        for(i = 0; i < nfreqs; i++) {
          mod = sqrt(output[i].real*output[i].real+output[i].imag*output[i].imag);
          pha = atan2(output[i].imag, output[i].real+1.e-200)*180./Pi;
#ifdef UNWRAP_PHASE
	  pha = unwrap_phase(pha, prev_phase, range, &added_value);
	  phaArray[i] = pha;
	  prev_phase = pha;
	  modArray[i] = mod;
#endif
#ifndef UNWRAP_PHASE
          fprintf(fptr1,"%.6E %.6E\n",freqs[i],mod);
          fprintf(fptr2,"%.6E %.6E\n",freqs[i],pha);
#endif
        }
#ifdef UNWRAP_PHASE
	/* Next function attempts to put phase withing -360:360 bounds
	 * this is requested by AFTAC
	*/
	(void) evresp_adjust_phase(phaArray, nfreqs, (double)-360, (double)360);
        for(i = 0; i < nfreqs; i++) {
          fprintf(fptr1,"%.6E %.6E\n",freqs[i],modArray[i]);
          fprintf(fptr2,"%.6E %.6E\n",freqs[i],phaArray[i]);
        }
	free(phaArray);
	free(modArray);
#endif
        fclose(fptr1);
        fclose(fptr2);
      }
      else {
        fprintf(stdout, "--------------------------------------------------\n");
        fprintf(stdout,"AMP/PHS.%s.%s.%s.%s\n",resp->network,resp->station,resp->locid,resp->channel);
        fprintf(stdout, "--------------------------------------------------\n");
        for(i = 0; i < nfreqs; i++) {
          mod = sqrt(output[i].real*output[i].real+output[i].imag*output[i].imag);
          pha = atan2(output[i].imag, output[i].real+1.e-200)*180./Pi;
          fprintf(stdout,"%.6E %.6E %.6E\n",freqs[i],mod,pha);
        }
        fprintf(stdout, "--------------------------------------------------\n");
      }
    }
    else {
      if(!stdio_flag) {
        sprintf(filename,"SPECTRA.%s.%s.%s.%s",resp->network,resp->station,resp->locid,resp->channel);
        if((fptr1 = fopen(filename,"w")) == (FILE *)NULL) {
          error_exit(OPEN_FILE_ERROR,"print_resp; failed to open file %s", filename);
        }
      }
      else {
        fptr1 = stdout;
        fprintf(stdout, "--------------------------------------------------\n");
        fprintf(stdout,"SPECTRA.%s.%s.%s.%s\n",resp->network,resp->station,resp->locid,resp->channel);
        fprintf(stdout, "--------------------------------------------------\n");
      }
      for(i = 0; i < nfreqs; i++)
        fprintf(fptr1,"%.6E %.6E %.6E\n",freqs[i],output[i].real,output[i].imag);
      if(!stdio_flag) {
        fclose(fptr1);
      }
    }
    resp = resp->next;
  }
}

void
	evresp_adjust_phase(double *pha, int len, double min, double max)
	{
		double vector_min;
		double vector_max;
		int retVal;
		int cycles = 0;
		int i;
		if (!pha)
			return;

		retVal = evresp_vector_minmax(pha, len, &vector_min, &vector_max);
		if (0 == retVal)
			return;
		/* See if the data range is within the requested */
		if ((max - min) < (vector_max - vector_min))
			return; /* range is larger that requested */
		/* see if shifting down puts vector inside the requested range */
		if (vector_max > max)
		{
			cycles = (vector_max-max)/180 + 1;
		}
		/* see if shifting up puts vector inside the requested range */
		if (vector_min < min)
		{
			cycles = (vector_min-min)/180 - 1;
		}
		/* Do actual shift: note if we were within the ranged originally , cycles == 0; */
		for (i = 0; i < len; i++)
			pha[i] = pha[i] - cycles * 180;
		return;
	}
int
	evresp_vector_minmax(double *pha, int len, double *min, double *max)
	{
		int i;
		if (!pha)
			return 0;
		*min = pha[0];
		*max = pha[0];
		for (i = 0; i < len; i++)
		{
			if (pha[i] > *max)
				*max = pha[i];
			if (pha[i] < *min)
				*min = pha[i];
		}
		return 1;
	}
