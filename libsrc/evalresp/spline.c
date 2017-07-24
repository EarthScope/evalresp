
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <evalresp_log/log.h>
#include <spline/spline.h>

#include "public_api.h"
#include "spline.h"

int
spline_interpolate (int num_points, double *t, double *y,
                    double *xvals_arr, int num_xvals,
                    double **p_retvals_arr, int *p_num_retvals, evalresp_logger *log)
{
  int i, status = EVALRESP_OK;
  const int ibcbeg = 2;
  const int ybcbeg = 0.0;
  const int ibcend = 2;
  const int ybcend = 0.0;
  double *ypp = NULL;
  double tval;
  double yval;
  double ypval;
  double yppval;

  *p_retvals_arr = NULL;
  *p_num_retvals = 0;

  if (!(ypp = spline_cubic_set (num_points, t, y, ibcbeg, ybcbeg, ibcend, ybcend, log)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Call to spline_cubic_set failed");
    status = EVALRESP_ERR;
  }
  else
  {

    if (!(*p_retvals_arr = (double *)calloc (num_xvals, sizeof (double))))
    {
      evalresp_log (log, EV_ERROR, EV_ERROR, "Failed to allocate p_retvals_arr array");
      status = EVALRESP_MEM;
    }
    else
    {
      for (i = 0; i < num_xvals; ++i)
      {
        tval = xvals_arr[i];
        yval = spline_cubic_val (num_points, t, y, ypp, tval, &ypval, &yppval);
        (*p_retvals_arr)[i] = yval;
      }
      *p_num_retvals = num_xvals;
    }
  }

  free (ypp);

  if (!*p_num_retvals)
  {
    free (*p_retvals_arr);
  }

  return status;
}
