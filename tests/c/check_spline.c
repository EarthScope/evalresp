#include <check.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "evalresp/constants.h"
#include "evalresp/private.h"
#include "spline/spline.h"

/* analytic response of an 8th-order Butterworth low-pass, corner at 20Hz */
static void
butterworth8 (double f, double *amp, double *phase_deg)
{
  int k;
  double re = 1.0, im = 0.0, fc = 20.0, x = f / fc;
  /* denominator is product of (s^2 + a_k s + 1), s = j x */
  for (k = 1; k <= 4; ++k)
  {
    double a = 2.0 * sin (M_PI * (2 * k - 1) / 16.0);
    double dr = 1.0 - x * x, di = a * x;
    double nr = re * dr - im * di, ni = re * di + im * dr;
    re = nr;
    im = ni;
  }
  /* H = 1 / D */
  *amp = 1.0 / sqrt (re * re + im * im);
  *phase_deg = -atan2 (im, re) * 180.0 / M_PI;
}

/* wrap an angle in degrees into (-180, 180] */
static double
wrap180 (double a)
{
  a = fmod (a, 360.0);
  if (a > 180.0)
    a -= 360.0;
  if (a <= -180.0)
    a += 360.0;
  return a;
}

/*
 * A cubic spline through non-uniformly spaced knots must have a continuous
 * first derivative.  (Regression: the sub- and super-diagonal coefficients
 * in spline_cubic_set were swapped, which is invisible for uniform knots.)
 */
START_TEST (test_c1_nonuniform)
{
  enum { N = 12 };
  double t[N], y[N], *ypp, d1, d2, dd;
  int i;
  for (i = 0; i < N; ++i)
  {
    t[i] = pow (1.6, i);
    y[i] = sin (0.3 * t[i]) + 0.1 * t[i];
  }
  ypp = spline_cubic_set (N, t, y, 2, 0.0, 2, 0.0, NULL);
  fail_if (ypp == NULL);
  for (i = 1; i < N - 1; ++i)
  {
    double e = 1e-9 * t[i];
    spline_cubic_val (N, t, y, ypp, t[i] - e, &d1, &dd);
    spline_cubic_val (N, t, y, ypp, t[i] + e, &d2, &dd);
    fail_if (fabs (d1 - d2) > 1e-6, "First derivative jumps by %g at knot %d", fabs (d1 - d2), i);
  }
  free (ypp);
}
END_TEST

/*
 * Steep roll-off sampled at 10 points/decade.  Interpolating in linear space
 * overshoots and goes negative (~45dB error); log-log must stay positive and
 * close to the true response.
 */
START_TEST (test_b55_steep_rolloff)
{
  const int per_dec = 10;
  int n = 4 * per_dec + 1; /* 0.01 - 100 Hz */
  int nreq = 500, i;
  double *freq = malloc (n * sizeof (double));
  double *amp = malloc (n * sizeof (double));
  double *pha = malloc (n * sizeof (double));
  double *req = malloc (nreq * sizeof (double));
  double max_db = 0, max_pha = 0;

  for (i = 0; i < n; ++i)
  {
    freq[i] = 0.01 * pow (10.0, (double)i / per_dec);
    butterworth8 (freq[i], &amp[i], &pha[i]);
    pha[i] = wrap180 (pha[i]); /* as a real list blockette would be */
  }
  for (i = 0; i < nreq; ++i)
    req[i] = 0.0105 * pow (99.0 / 0.0105, (double)i / (nreq - 1));

  fail_if (interpolate_list_blockette (&freq, &amp, &pha, &n, req, nreq, NULL));
  fail_if (n != nreq, "Unexpected number of points: %d", n);
  for (i = 0; i < n; ++i)
  {
    double ta, tp, db, dp;
    butterworth8 (freq[i], &ta, &tp);
    fail_if (!(amp[i] > 0.0), "Non-positive amplitude %g at %g Hz", amp[i], freq[i]);
    db = fabs (20.0 * log10 (amp[i] / ta));
    dp = fabs (wrap180 (pha[i] - tp));
    if (db > max_db)
      max_db = db;
    if (dp > max_pha)
      max_pha = dp;
  }
  fail_if (max_db > 0.5, "Max amplitude error %g dB", max_db);
  fail_if (max_pha > 5.0, "Max phase error %g deg", max_pha);

  free (freq);
  free (amp);
  free (pha);
  free (req);
}
END_TEST

/* Requesting exactly the listed frequencies must reproduce the listed values. */
START_TEST (test_b55_exact_at_knots)
{
  int n = 21, i;
  double *freq = malloc (n * sizeof (double));
  double *amp = malloc (n * sizeof (double));
  double *pha = malloc (n * sizeof (double));
  double *req = malloc (n * sizeof (double));
  double *amp0 = malloc (n * sizeof (double));
  double *pha0 = malloc (n * sizeof (double));

  for (i = 0; i < n; ++i)
  {
    freq[i] = req[i] = 0.1 * pow (10.0, (double)i / 10);
    butterworth8 (freq[i], &amp[i], &pha[i]);
    pha[i] = wrap180 (pha[i]);
    amp0[i] = amp[i];
    pha0[i] = pha[i];
  }

  fail_if (interpolate_list_blockette (&freq, &amp, &pha, &n, req, 21, NULL));
  fail_if (n != 21, "Unexpected number of points: %d", n);
  for (i = 0; i < n; ++i)
  {
    fail_if (fabs (amp[i] - amp0[i]) > 1e-9 * amp0[i], "Amplitude %d: %g vs %g", i, amp[i], amp0[i]);
    fail_if (fabs (wrap180 (pha[i] - pha0[i])) > 1e-6, "Phase %d: %g vs %g", i, pha[i], pha0[i]);
  }

  free (freq);
  free (amp);
  free (pha);
  free (req);
  free (amp0);
  free (pha0);
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_c1_nonuniform);
  tcase_add_test (tc, test_b55_steep_rolloff);
  tcase_add_test (tc, test_b55_exact_at_knots);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-spline.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
