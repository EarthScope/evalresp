
/* This file is modified by I.Dricker I.dricker@isti.com for version 3.2.17 of evalresp*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "./private.h"
#include "evalresp/public_api.h"
#include "evalresp_log/log.h"

evalresp_complex *
alloc_complex (int npts, evalresp_logger *log)
{
  evalresp_complex *cptr;

  if (npts)
  {
    if (!(cptr = malloc (npts * sizeof (*cptr))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_complex; malloc() failed for (complex) vector");
      return NULL; // TODO - return EVALRESP_MEM
    }
  }
  else
    cptr = NULL;

  return (cptr);
}

struct string_array *
alloc_string_array (int nstrings, evalresp_logger *log)
{
  struct string_array *sl_ptr;
  int i;

  if (nstrings)
  {
    if (!(sl_ptr = (struct string_array *)malloc (sizeof (struct string_array))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_string_array; malloc() failed for (string_array)");
      return NULL; // TODO - return EVALRESP_MEM
    }
    if ((sl_ptr->strings = (char **)malloc (nstrings * sizeof (char *))) == (char **)NULL)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_string_array; malloc() failed for (char *) vector");
      free (sl_ptr);
      return NULL; /* OUT_OF_MEMORY */
    }
    for (i = 0; i < nstrings; i++)
      sl_ptr->strings[i] = (char *)NULL;
    sl_ptr->nstrings = nstrings;
  }
  else
    sl_ptr = (struct string_array *)NULL;

  return (sl_ptr);
}

evalresp_sncl *
alloc_scn (evalresp_logger *log)
{
  evalresp_sncl *scn_ptr;

  if (!(scn_ptr = (evalresp_sncl *)malloc (sizeof (evalresp_sncl))))
  {
    evalresp_log (log, EV_ERROR, 0, "alloc_scn; malloc() failed for (scn)");
    return NULL; /* OUT_OF_MEMORY */
  }
  if (!(scn_ptr->station = (char *)malloc (STALEN * sizeof (char))))
  {
    evalresp_log (log, EV_ERROR, 0, "alloc_scn; malloc() failed for (station)");
    free (scn_ptr);
    return NULL; /* OUT_OF_MEMORY */
  }
  if (!(scn_ptr->network = (char *)malloc (NETLEN * sizeof (char))))
  {
    evalresp_log (log, EV_ERROR, 0, "alloc_scn; malloc() failed for (station)");
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL; /* OUT_OF_MEMORY */
  }
  if (!(scn_ptr->locid = (char *)malloc (LOCIDLEN * sizeof (char))))
  {
    evalresp_log (log, EV_ERROR, 0, "alloc_scn; malloc() failed for (channel)");
    free (scn_ptr->network);
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL; /* OUT_OF_MEMORY */
  }
  if (!(scn_ptr->channel = (char *)malloc (CHALEN * sizeof (char))))
  {
    evalresp_log (log, EV_ERROR, 0, "alloc_scn; malloc() failed for (channel)");
    free (scn_ptr->channel);
    free (scn_ptr->network);
    free (scn_ptr->station);
    free (scn_ptr);
    return NULL; /* OUT_OF_MEMORY */
  }

  strncpy (scn_ptr->station, "", STALEN);
  strncpy (scn_ptr->network, "", NETLEN);
  strncpy (scn_ptr->locid, "", LOCIDLEN);
  strncpy (scn_ptr->channel, "", CHALEN);
  scn_ptr->found = 0;

  return (scn_ptr);
}

evalresp_response *
alloc_response (int npts, evalresp_logger *log)
{
  evalresp_response *rptr;
  evalresp_complex *cvec;
  int k;

  if (npts)
  {
    if (!(rptr = (evalresp_response *)malloc (sizeof (evalresp_response))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_response; malloc() failed for (response) vector");
      return NULL; /* OUT_OF_MEMORY */
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
    rptr->next = (evalresp_response *)NULL;
    /*IGD add freqs to this structure to process blockette 55 */
    rptr->nfreqs = 0;
    rptr->freqs = (double *)NULL;
  }
  else
    rptr = (evalresp_response *)NULL;

  return (rptr);
}

evalresp_sncls *
alloc_scn_list (int nscn, evalresp_logger *log)
{
  evalresp_sncls *sc_ptr;
  int i;

  if (nscn)
  {
    if (!(sc_ptr = (evalresp_sncls *)malloc (sizeof (evalresp_sncls))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_scn_list; malloc() failed for (scn_list)");
      return NULL; /* OUT_OF_MEMORY */
    }
    if ((sc_ptr->scn_vec = (evalresp_sncl **)malloc (
             nscn * sizeof (evalresp_sncl *))) == (evalresp_sncl **)NULL)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_scn_list; malloc() failed for (scn_vec)");
      free (sc_ptr);
      return NULL; /* OUT_OF_MEMORY */
    }
    for (i = 0; i < nscn; i++)
    {
      sc_ptr->scn_vec[i] = alloc_scn (log);
    }
    sc_ptr->nscn = nscn;
  }
  else
  {
    sc_ptr = (evalresp_sncls *)NULL;
  }

  return (sc_ptr);
}

struct file_list *
alloc_file_list (evalresp_logger *log)
{
  struct file_list *flst_ptr;

  if (!(flst_ptr = (struct file_list *)malloc (sizeof (struct file_list))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_file_list; malloc() failed for (file_list)");
    return NULL; /* OUT_OF_MEMORY */
  }
  flst_ptr->name = (char *)NULL;
  flst_ptr->next_file = (struct file_list *)NULL;

  return (flst_ptr);
}

struct matched_files *
alloc_matched_files (evalresp_logger *log)
{
  struct matched_files *flst_ptr;

  if (!(flst_ptr = (struct matched_files *)malloc (sizeof (struct matched_files))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_matched_files; malloc() failed for (matched_files)");
    return NULL; /* OUT_OF_MEMORY */
  }
  flst_ptr->nfiles = 0;
  flst_ptr->first_list = (struct file_list *)NULL;
  flst_ptr->ptr_next = (struct matched_files *)NULL;

  return (flst_ptr);
}

// TODO - replace with calloc_doubles below?
double *
alloc_double (int npts, evalresp_logger *log)
{
  double *dptr;

  if (npts)
  {
    if (!(dptr = (double *)malloc (npts * sizeof (double))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_double; malloc() failed for (double) vector");
      return NULL; /* OUT_OF_MEMORY */
    }
  }
  else
    dptr = (double *)NULL;

  return (dptr);
}

char *
alloc_char (int len, evalresp_logger *log)
{
  char *cptr;

  if (len)
  {
    if (!(cptr = (char *)malloc (len * sizeof (char))))
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_char; malloc() failed for (char) vector");
      return NULL; /* OUT_OF_MEMORY */
    }
  }
  else
    cptr = (char *)NULL;

  return (cptr);
}

char **
alloc_char_ptr (int len, evalresp_logger *log)
{
  char **cptr;

  if (len)
  {
    if ((cptr = (char **)malloc (len * sizeof (char *))) == (char **)NULL)
    {
      evalresp_log (log, EV_ERROR, 0,
                    "alloc_char_ptr; malloc() failed for (char *) vector");
      return NULL; /* OUT_OF_MEMORY */
    }
  }
  else
    cptr = (char **)NULL;

  return (cptr);
}

evalresp_blkt *
alloc_pz (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_pz; malloc() failed for (Poles & Zeros) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = NULL;
  blkt_ptr->blkt_info.pole_zero.zeros = NULL;
  blkt_ptr->blkt_info.pole_zero.poles = NULL;
  blkt_ptr->blkt_info.pole_zero.nzeros = 0;
  blkt_ptr->blkt_info.pole_zero.npoles = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_coeff (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (*blkt_ptr))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_coeff; malloc() failed for (FIR) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = NULL;
  blkt_ptr->blkt_info.coeff.numer = NULL;
  blkt_ptr->blkt_info.coeff.denom = NULL;
  blkt_ptr->blkt_info.coeff.nnumer = 0;
  blkt_ptr->blkt_info.coeff.ndenom = 0;
  blkt_ptr->blkt_info.coeff.h0 = 1.0; /*IGD this field is new for v 3.2.17*/

  return (blkt_ptr);
}

evalresp_blkt *
alloc_polynomial (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)calloc (1, sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_polynomial; calloc() failed for polynomial blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  return (blkt_ptr);
}

evalresp_blkt *
alloc_fir (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_fir; malloc() failed for (FIR) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = 0;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.fir.coeffs = (double *)NULL;
  blkt_ptr->blkt_info.fir.ncoeffs = 0;
  blkt_ptr->blkt_info.fir.h0 = 1.0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_ref (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_ref; malloc() failed for (Resp. Ref.) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = REFERENCE;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.reference.num_stages = 0;
  blkt_ptr->blkt_info.reference.stage_num = 0;
  blkt_ptr->blkt_info.reference.num_responses = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_gain (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_gain; malloc() failed for (Gain) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = GAIN;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.gain.gain = 0;
  blkt_ptr->blkt_info.gain.gain_freq = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_list (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_list; malloc() failed for (List) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
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
alloc_generic (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_generic; malloc() failed for (Generic) blkt structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  blkt_ptr->type = GENERIC;
  blkt_ptr->next_blkt = (evalresp_blkt *)NULL;
  blkt_ptr->blkt_info.generic.corner_slope = (double *)NULL;
  blkt_ptr->blkt_info.generic.corner_freq = (double *)NULL;
  blkt_ptr->blkt_info.generic.ncorners = 0;

  return (blkt_ptr);
}

evalresp_blkt *
alloc_deci (evalresp_logger *log)
{
  evalresp_blkt *blkt_ptr;

  if (!(blkt_ptr = (evalresp_blkt *)malloc (sizeof (evalresp_blkt))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_deci; malloc() failed for (Decimation) blkt structure");
    return NULL /* OUT_OF_MEMORY */;
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
alloc_stage (evalresp_logger *log)
{
  evalresp_stage *stage_ptr;

  if (!(stage_ptr = (evalresp_stage *)malloc (sizeof (evalresp_stage))))
  {
    evalresp_log (log, EV_ERROR, 0,
                  "alloc_stage; malloc() failed for stage structure");
    return NULL; /* OUT_OF_MEMORY */
  }

  stage_ptr->sequence_no = 0;
  stage_ptr->output_units = 0;
  stage_ptr->input_units = 0;
  stage_ptr->output_units_str = NULL;
  stage_ptr->input_units_str = NULL;
  stage_ptr->first_blkt = NULL;
  stage_ptr->next_stage = NULL;

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
free_scn (evalresp_sncl *ptr)
{

  free (ptr->station);
  free (ptr->network);
  free (ptr->locid);
  free (ptr->channel);
}

void
evalresp_free_sncls (evalresp_sncls *lst)
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
  if (lst)
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

  if (lst)
  {
    free_file_list (lst->next_file);
    if (lst->name)
      free (lst->name);
    if (lst->next_file)
      free (lst->next_file);
  }
}

void
free_pz (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr)
  {
    if (blkt_ptr->blkt_info.pole_zero.zeros)
      free (blkt_ptr->blkt_info.pole_zero.zeros);
    if (blkt_ptr->blkt_info.pole_zero.poles)
      free (blkt_ptr->blkt_info.pole_zero.poles);
    free (blkt_ptr);
  }
}

void
free_coeff (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.coeff.numer)
      free (blkt_ptr->blkt_info.coeff.numer);
    if (blkt_ptr->blkt_info.coeff.denom)
      free (blkt_ptr->blkt_info.coeff.denom);
    free (blkt_ptr);
  }
}

void
free_fir (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.fir.coeffs)
      free (blkt_ptr->blkt_info.fir.coeffs);
    free (blkt_ptr);
  }
}

void
free_list (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.list.freq)
      free (blkt_ptr->blkt_info.list.freq);
    if (blkt_ptr->blkt_info.list.amp)
      free (blkt_ptr->blkt_info.list.amp);
    if (blkt_ptr->blkt_info.list.phase)
      free (blkt_ptr->blkt_info.list.phase);
    free (blkt_ptr);
  }
}

void
free_generic (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != (evalresp_blkt *)NULL)
  {
    if (blkt_ptr->blkt_info.generic.corner_slope)
      free (blkt_ptr->blkt_info.generic.corner_slope);
    if (blkt_ptr->blkt_info.generic.corner_freq)
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
free_polynomial (evalresp_blkt *blkt_ptr)
{
  if (blkt_ptr != NULL)
  {
    if (blkt_ptr->blkt_info.polynomial.coeffs)
    {
      free (blkt_ptr->blkt_info.polynomial.coeffs);
    }
    if (blkt_ptr->blkt_info.polynomial.coeffs_err)
    {
      free (blkt_ptr->blkt_info.polynomial.coeffs_err);
    }
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
      case POLYNOMIAL:
        free_polynomial (this_blkt);
        break;
      default:
        break;
      }
      this_blkt = next_blkt;
    }

    if (stage_ptr->output_units_str)
      free (stage_ptr->output_units_str);
    if (stage_ptr->input_units_str)
      free (stage_ptr->input_units_str);

    free (stage_ptr);
    stage_ptr = NULL;
  }
}

void
free_channel (evalresp_channel *chan_ptr)
{
  if (chan_ptr)
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
}

void
evalresp_free_channel (evalresp_channel **chan_ptr)
{
  free_channel (*chan_ptr);
  free (*chan_ptr);
  *chan_ptr = NULL;
}

void
evalresp_free_response (evalresp_response **response)
{
  if (*response)
  {
    evalresp_response *this_resp, *next_resp;

    this_resp = *response;
    while (this_resp)
    {
      next_resp = this_resp->next;
      free (this_resp->rvec);
      free (this_resp->freqs); /*IGD for v 3.2.17 */
      free (this_resp);
      this_resp = next_resp;
    }
  }
  *response = NULL;
}

void
evalresp_free_responses (evalresp_responses **responses)
{
  int i;
  if (*responses)
  {
    for (i = 0; i < (*responses)->nresponses; ++i)
    {
      evalresp_free_response (&(*responses)->responses[i]);
    }
    free ((*responses)->responses);
    (*responses)->responses = NULL;
    free (*responses);
  }
  *responses = NULL;
}

int
evalresp_alloc_channels (evalresp_logger *log, evalresp_channels **channels)
{
  int status = EVALRESP_OK;
  if (!(*channels = calloc (1, sizeof (**channels))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate space for channels");
    status = EVALRESP_MEM;
  }
  return status;
}

void
evalresp_free_channels (evalresp_channels **channels)
{
  int i;
  if (*channels)
  {
    for (i = 0; i < (*channels)->nchannels; ++i)
    {
      evalresp_free_channel (&(*channels)->channels[i]);
    }
    free ((*channels)->channels);
    (*channels)->channels = NULL;
  }
  free (*channels);
  *channels = NULL;
}

int
calloc_doubles (evalresp_logger *log, const char *name, int n, double **array)
{
  int status = EVALRESP_OK;
  if (!(*array = calloc (n, sizeof (**array))))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Cannot allocate array for %s", name);
    status = EVALRESP_MEM;
  }
  return status;
}
