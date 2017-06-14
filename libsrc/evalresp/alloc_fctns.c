/* This file is modified by I.Dricker I.dricker@isti.com for version 3.2.17 of evalresp*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "evalresp_log/log.h"
#include "evalresp/evalresp_private.h"
#include "evalresp/evalresp_public_seed.h"

struct evr_complex *
alloc_complex (int npts, evalresp_log_t *log)
{
  struct evr_complex *cptr;

  if (npts)
  {
    if ((cptr = (struct evr_complex *)malloc (npts * sizeof (struct evr_complex))) == (struct evr_complex *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_complex; malloc() failed for (complex) vector");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_complex; malloc() failed for (complex) vector"); */
    }
  }
  else
    cptr = (struct evr_complex *)NULL;

  return (cptr);
}

struct string_array *
alloc_string_array (int nstrings, evalresp_log_t *log)
{
  struct string_array *sl_ptr;
  int i;

  if (nstrings)
  {
    if ((sl_ptr = (struct string_array *)malloc (
             sizeof (struct string_array))) == (struct string_array *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_string_array; malloc() failed for (string_array)");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_string_array; malloc() failed for (string_array)"); */
    }
    if ((sl_ptr->strings = (char **)malloc (nstrings * sizeof (char *))) == (char **)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_string_array; malloc() failed for (char *) vector");
      free (sl_ptr);
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_string_array; malloc() failed for (char *) vector"); */
    }
    for (i = 0; i < nstrings; i++)
      sl_ptr->strings[i] = (char *)NULL;
    sl_ptr->nstrings = nstrings;
  }
  else
    sl_ptr = (struct string_array *)NULL;

  return (sl_ptr);
}

struct scn *
alloc_scn (evalresp_log_t *log)
{
  struct scn *scn_ptr;

  if ((scn_ptr = (struct scn *)malloc (sizeof (struct scn))) == (struct scn *)NULL)
  {
    evalresp_log (log, ERROR, 0, "alloc_scn; malloc() failed for (scn)");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY, "alloc_scn; malloc() failed for (scn)"); */
  }
  if ((scn_ptr->station = (char *)malloc (STALEN * sizeof (char))) == (char *)NULL)
  {
    evalresp_log (log, ERROR, 0, "alloc_scn; malloc() failed for (station)");
    free (scn_ptr);
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY, "alloc_scn; malloc() failed for (station)"); */
  }
  if ((scn_ptr->network = (char *)malloc (NETLEN * sizeof (char))) == (char *)NULL)
  {
    evalresp_log (log, ERROR, 0, "alloc_scn; malloc() failed for (station)");
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY, "alloc_scn; malloc() failed for (station)"); */
  }
  if ((scn_ptr->locid = (char *)malloc (LOCIDLEN * sizeof (char))) == (char *)NULL)
  {
    evalresp_log (log, ERROR, 0, "alloc_scn; malloc() failed for (channel)");
    free (scn_ptr->network);
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY, "alloc_scn; malloc() failed for (channel)"); */
  }
  if ((scn_ptr->channel = (char *)malloc (CHALEN * sizeof (char))) == (char *)NULL)
  {
    evalresp_log (log, ERROR, 0, "alloc_scn; malloc() failed for (channel)");
    free (scn_ptr->channel);
    free (scn_ptr->network);
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY, "alloc_scn; malloc() failed for (channel)"); */
  }

  strncpy (scn_ptr->station, "", STALEN);
  strncpy (scn_ptr->network, "", NETLEN);
  strncpy (scn_ptr->locid, "", LOCIDLEN);
  strncpy (scn_ptr->channel, "", CHALEN);
  scn_ptr->found = 0;

  return (scn_ptr);
}

struct response *
alloc_response (int npts, evalresp_log_t *log)
{
  struct response *rptr;
  struct evr_complex *cvec;
  int k;

  if (npts)
  {
    if ((rptr = (struct response *)malloc (sizeof (struct response))) == (struct response *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_response; malloc() failed for (response) vector");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_response; malloc() failed for (response) vector"); */
    }
    strncpy (rptr->station, "", STALEN);
    strncpy (rptr->locid, "", LOCIDLEN);
    strncpy (rptr->channel, "", NETLEN);
    strncpy (rptr->network, "", CHALEN);
    rptr->rvec = alloc_complex (npts, log);
    cvec = rptr->rvec;
    for (k = 0; k < npts; k++)
    {
      cvec[k].real = 0.0;
      cvec[k].imag = 0.0;
    }
    rptr->next = (struct response *)NULL;
    /*IGD add freqs to this structure to process blockette 55 */
    rptr->nfreqs = 0;
    rptr->freqs = (double *)NULL;
  }
  else
    rptr = (struct response *)NULL;

  return (rptr);
}

struct scn_list *
alloc_scn_list (int nscn, evalresp_log_t *log)
{
  struct scn_list *sc_ptr;
  int i;

  if (nscn)
  {
    if ((sc_ptr = (struct scn_list *)malloc (sizeof (struct scn_list))) == (struct scn_list *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_scn_list; malloc() failed for (scn_list)");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_scn_list; malloc() failed for (scn_list)"); */
    }
    if ((sc_ptr->scn_vec = (struct scn **)malloc (
             nscn * sizeof (struct scn *))) == (struct scn **)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_scn_list; malloc() failed for (scn_vec)");
      free (sc_ptr);
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_scn_list; malloc() failed for (scn_vec)"); */
    }
    for (i = 0; i < nscn; i++)
    {
      sc_ptr->scn_vec[i] = alloc_scn (log);
    }
    sc_ptr->nscn = nscn;
  }
  else
  {
    sc_ptr = (struct scn_list *)NULL;
  }

  return (sc_ptr);
}

struct file_list *
alloc_file_list (evalresp_log_t *log)
{
  struct file_list *flst_ptr;

  if ((flst_ptr = (struct file_list *)malloc (sizeof (struct file_list))) == (struct file_list *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_file_list; malloc() failed for (file_list)");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_file_list; malloc() failed for (file_list)"); */
  }
  flst_ptr->name = (char *)NULL;
  flst_ptr->next_file = (struct file_list *)NULL;

  return (flst_ptr);
}

struct matched_files *
alloc_matched_files (evalresp_log_t *log)
{
  struct matched_files *flst_ptr;

  if ((flst_ptr = (struct matched_files *)malloc (
           sizeof (struct matched_files))) == (struct matched_files *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_matched_files; malloc() failed for (matched_files)");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_matched_files; malloc() failed for (matched_files)"); */
  }
  flst_ptr->nfiles = 0;
  flst_ptr->first_list = (struct file_list *)NULL;
  flst_ptr->ptr_next = (struct matched_files *)NULL;

  return (flst_ptr);
}

double *
alloc_double (int npts, evalresp_log_t *log)
{
  double *dptr;

  if (npts)
  {
    if ((dptr = (double *)malloc (npts * sizeof (double))) == (double *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_double; malloc() failed for (double) vector");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_double; malloc() failed for (double) vector"); */
    }
  }
  else
    dptr = (double *)NULL;

  return (dptr);
}

char *
alloc_char (int len, evalresp_log_t *log)
{
  char *cptr;

  if (len)
  {
    if ((cptr = (char *)malloc (len * sizeof (char))) == (char *)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_char; malloc() failed for (char) vector");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_char; malloc() failed for (char) vector"); */
    }
  }
  else
    cptr = (char *)NULL;

  return (cptr);
}

char **
alloc_char_ptr (int len, evalresp_log_t *log)
{
  char **cptr;

  if (len)
  {
    if ((cptr = (char **)malloc (len * sizeof (char *))) == (char **)NULL)
    {
      evalresp_log (log, ERROR, 0,
                    "alloc_char_ptr; malloc() failed for (char *) vector");
      return NULL;
      /*XXX error_exit(OUT_OF_MEMORY,
                    "alloc_char_ptr; malloc() failed for (char *) vector"); */
    }
  }
  else
    cptr = (char **)NULL;

  return (cptr);
}

evalresp_blkt *
alloc_pz (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_pz; malloc() failed for (Poles & Zeros) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_pz; malloc() failed for (Poles & Zeros) blkt structure"); */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.pole_zero.zeros = (struct evr_complex *)NULL;
  blkt_ptr->blkt_info.pole_zero.poles = (struct evr_complex *)NULL;
  blkt_ptr->blkt_info.pole_zero.nzeros = 0;
  blkt_ptr->blkt_info.pole_zero.npoles = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_coeff (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_coeff; malloc() failed for (FIR) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_coeff; malloc() failed for (FIR) blkt structure"); */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.coeff.numer = (double *)NULL;
  blkt_ptr->blkt_info.coeff.denom = (double *)NULL;
  blkt_ptr->blkt_info.coeff.nnumer = 0;
  blkt_ptr->blkt_info.coeff.ndenom = 0;
  blkt_ptr->blkt_info.coeff.h0 = 1.0; /*IGD this field is new for v 3.2.17*/

  return (blkt_ptr);
}

evalresp_blkt *
alloc_polynomial (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)calloc (1, sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_polynomial; calloc() failed for polynomial blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_polynomial; calloc() failed for polynomial blkt structure");  */
  }

  return (blkt_ptr);
}

evalresp_blkt *
alloc_fir (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_fir; malloc() failed for (FIR) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_fir; malloc() failed for (FIR) blkt structure"); */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.fir.coeffs = (double *)NULL;
  blkt_ptr->blkt_info.fir.ncoeffs = 0;
  blkt_ptr->blkt_info.fir.h0 = 1.0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_ref (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_ref; malloc() failed for (Resp. Ref.) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_ref; malloc() failed for (Resp. Ref.) blkt structure"); */
  }

  blkt_ptr->type = REFERENCE;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.reference.num_stages = 0;
  blkt_ptr->blkt_info.reference.stage_num = 0;
  blkt_ptr->blkt_info.reference.num_responses = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_gain (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_gain; malloc() failed for (Gain) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_gain; malloc() failed for (Gain) blkt structure"); */
  }

  blkt_ptr->type = GAIN;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.gain.gain = 0;
  blkt_ptr->blkt_info.gain.gain_freq = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_list (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_list; malloc() failed for (List) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_list; malloc() failed for (List) blkt structure"); */
  }

  blkt_ptr->type = LIST;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.list.freq = (double *)NULL;
  blkt_ptr->blkt_info.list.amp = (double *)NULL;
  blkt_ptr->blkt_info.list.phase = (double *)NULL;
  blkt_ptr->blkt_info.list.nresp = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_generic (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_generic; malloc() failed for (Generic) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_generic; malloc() failed for (Generic) blkt structure"); */
  }

  blkt_ptr->type = GENERIC;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.generic.corner_slope = (double *)NULL;
  blkt_ptr->blkt_info.generic.corner_freq = (double *)NULL;
  blkt_ptr->blkt_info.generic.ncorners = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_deci (evalresp_log_t *log)
{
  evalresp_blkt *blkt_ptr;

  if ((blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))) == (evalresp_blkt *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_deci; malloc() failed for (Decimation) blkt structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_deci; malloc() failed for (Decimation) blkt structure"); */
  }

  blkt_ptr->type = DECIMATION;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.decimation.sample_int = 0;
  blkt_ptr->blkt_info.decimation.deci_fact = 0;
  blkt_ptr->blkt_info.decimation.deci_offset = 0;
  blkt_ptr->blkt_info.decimation.estim_delay = 0;
  blkt_ptr->blkt_info.decimation.applied_corr = 0;

  return (blkt_ptr);
}

evalresp_stage *
alloc_stage (evalresp_log_t *log)
{
  evalresp_stage *stage_ptr;

  if ((stage_ptr = (evalresp_stage *)malloc (sizeof (evalresp_stage))) == (evalresp_stage *)NULL)
  {
    evalresp_log (log, ERROR, 0,
                  "alloc_stage; malloc() failed for stage structure");
    return NULL;
    /*XXX error_exit(OUT_OF_MEMORY,
                "alloc_stage; malloc() failed for stage structure"); */
  }

  stage_ptr->sequence_no = 0;
  stage_ptr->output_units = 0;
  stage_ptr->input_units = 0;
  stage_ptr->first_blkt = (evalresp_blkt *)NULL;
  stage_ptr->next_stage = (evalresp_stage *)NULL;

  return (stage_ptr);
}

void
free_string_array (struct string_array *lst)
{
  int i;

  if (lst)
  {
    if (lst->strings)
    {
      for (i = 0; i < lst->nstrings; i++)
      {
        if (lst->strings[i])
        {
          free (lst->strings[i]);
          lst->strings[i] = NULL;
        }
      }
      free (lst->strings);
      lst->strings = NULL;
    }
    free (lst);
  }
}

void
free_scn (struct scn *ptr)
{

  free (ptr->station);
  free (ptr->network);
  free (ptr->locid);
  free (ptr->channel);
}

void
free_scn_list (struct scn_list *lst)
{
  int i;

  for (i = 0; i < lst->nscn; i++)
  {
    free_scn (lst->scn_vec[i]);
    free (lst->scn_vec[i]);
  }
  free (lst->scn_vec);
  free (lst);
}

void
free_matched_files (struct matched_files *lst)
{
  if (lst != (struct matched_files *)NULL)
  {
    free_matched_files (lst->ptr_next);
    if (lst->nfiles)
    {
      free_file_list (lst->first_list);
      free (lst->first_list);
    }
    free (lst);
    lst = (struct matched_files *)NULL;
  }
}

void
free_file_list (struct file_list *lst)
{

  if (lst != (struct file_list *)NULL)
  {
    free_file_list (lst->next_file);
    if (lst->name != (char *)NULL)
      free (lst->name);
    if (lst->next_file != (struct file_list *)NULL)
      free (lst->next_file);
  }
}

void
free_pz (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.pole_zero.zeros != (struct evr_complex *)NULL)
      free (blkt_ptr->blkt_info.pole_zero.zeros);
    if (blkt_ptr->blkt_info.pole_zero.poles != (struct evr_complex *)NULL)
      free (blkt_ptr->blkt_info.pole_zero.poles);
    free (blkt_ptr);
  }
}

void
free_coeff (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.coeff.numer != (double *)NULL)
      free (blkt_ptr->blkt_info.coeff.numer);
    if (blkt_ptr->blkt_info.coeff.denom != (double *)NULL)
      free (blkt_ptr->blkt_info.coeff.denom);
    free (blkt_ptr);
  }
}

void
free_fir (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.fir.coeffs != (double *)NULL)
      free (blkt_ptr->blkt_info.fir.coeffs);
    free (blkt_ptr);
  }
}

void
free_list (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.list.freq != (double *)NULL)
      free (blkt_ptr->blkt_info.list.freq);
    if (blkt_ptr->blkt_info.list.amp != (double *)NULL)
      free (blkt_ptr->blkt_info.list.amp);
    if (blkt_ptr->blkt_info.list.phase != (double *)NULL)
      free (blkt_ptr->blkt_info.list.phase);
    free (blkt_ptr);
  }
}

void
free_generic (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.generic.corner_slope != (double *)NULL)
      free (blkt_ptr->blkt_info.generic.corner_slope);
    if (blkt_ptr->blkt_info.generic.corner_freq != (double *)NULL)
      free (blkt_ptr->blkt_info.generic.corner_freq);
    free (blkt_ptr);
  }
}

void
free_gain (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    free (blkt_ptr);
  }
}

void
free_deci (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    free (blkt_ptr);
  }
}

void
free_ref (evalresp_blkt *blkt_ptr)
{

  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    free (blkt_ptr);
  }
}

void
free_stages (evalresp_stage *stage_ptr)
{
  evalresp_blkt *this_blkt, *next_blkt;

  if (stage_ptr != (evalresp_stage *)NULL)
  {
    free_stages (stage_ptr->next_stage);
    this_blkt = stage_ptr->first_blkt;
    while (this_blkt != (evalresp_blkt *)NULL)
    {
      next_blkt = this_blkt->next_blkt;
      switch (this_blkt->type)
      {
      case LAPLACE_PZ:
      case ANALOG_PZ:
      case IIR_PZ:
        free_pz (this_blkt);
        break;
      case FIR_SYM_1:
      case FIR_SYM_2:
      case FIR_ASYM:
        free_fir (this_blkt);
        break;
      case FIR_COEFFS:
        free_coeff (this_blkt);
        break;
      case LIST:
        free_list (this_blkt);
        break;
      case GENERIC:
        free_generic (this_blkt);
        break;
      case DECIMATION:
        free_deci (this_blkt);
        break;
      case GAIN:
        free_gain (this_blkt);
        break;
      case REFERENCE:
        free_ref (this_blkt);
        break;
      default:
        break;
      }
      this_blkt = next_blkt;
    }
    free (stage_ptr);
  }
}

void
free_channel (evalresp_channel *chan_ptr)
{

  free_stages (chan_ptr->first_stage);
  strncpy (chan_ptr->staname, "", STALEN);
  strncpy (chan_ptr->network, "", NETLEN);
  strncpy (chan_ptr->locid, "", LOCIDLEN);
  strncpy (chan_ptr->chaname, "", CHALEN);
  strncpy (chan_ptr->beg_t, "", DATIMLEN);
  strncpy (chan_ptr->end_t, "", DATIMLEN);
  strncpy (chan_ptr->first_units, "", MAXLINELEN);
  strncpy (chan_ptr->last_units, "", MAXLINELEN);
}

void
free_response (struct response *resp_ptr)
{
  struct response *this_resp, *next_resp;

  this_resp = resp_ptr;
  while (this_resp != (struct response *)NULL)
  {
    next_resp = this_resp->next;
    free (this_resp->rvec);
    free (this_resp->freqs); /*IGD for v 3.2.17 */
    free (this_resp);
    this_resp = next_resp;
  }
}
