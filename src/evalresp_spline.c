#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "spline.h"
#include <evalresp_log/log.h>

#include "evr_spline.h"

char * evr_spline(int num_points, double *t, double *y,
                  double tension, /* Not used anymore */
                  double k, /* Not used anymore */
                  double *xvals_arr, int num_xvals,
                  double **p_retvals_arr, int *p_num_retvals, evalresp_log_t *log)
{
  char *return_string = NULL;
  int i;
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

  ypp = spline_cubic_set(num_points, t, y, ibcbeg, ybcbeg, ibcend, ybcend, log);
  if (NULL == ypp) {
    return_string = "Call to spline_cubic_set failed";
    goto exit;
  }

  *p_retvals_arr = (double *)calloc(num_xvals, sizeof(double));
  if (NULL == *p_retvals_arr) {
    return_string = "Failed to allocate p_retvals_arr array";
    goto exit;
  }

  for (i = 0; i < num_xvals; ++i) {
    tval = xvals_arr[i];
    yval = spline_cubic_val(num_points, t, y, ypp, tval, &ypval, &yppval);
    (*p_retvals_arr)[i] = yval;
  }
  *p_num_retvals = num_xvals;

 exit:

  if (NULL != ypp) {
    free(ypp);
    ypp = NULL;
  }

  if (NULL != *p_retvals_arr && 0 == *p_num_retvals) {
    free(*p_retvals_arr);
    *p_retvals_arr = NULL;
  }

  return return_string;
}
