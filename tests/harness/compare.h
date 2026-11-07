#ifndef ER_COMPARE_H
#define ER_COMPARE_H

#include <stddef.h>

#define ER_TOL_DEFAULT 1e-5

/*
 * How two text files are compared.  Lines are compared with trailing CR/LF
 * removed.
 *
 * With ncols > 0, a line that holds exactly ncols numbers in both files is
 * compared numerically; any other line pair must be identical text.  The
 * default numeric test is relative (against the larger magnitude, switching
 * to absolute below tol).
 */
typedef struct
{
  int ncols;    /* numeric columns per line; 0 for plain text */
  double tol;   /* tolerance; <= 0 selects ER_TOL_DEFAULT */
  int absolute; /* test |a-b| <= tol instead of the relative test */
  int log_text; /* before comparing, strip the leading "date [" from each
                   result line and mask the version in the output banner */
} er_cmp;

/* returns 0 if the files match; otherwise fills msg and returns non-zero */
int er_compare_files (const char *result, const char *target, const er_cmp *opts, char *msg, size_t msg_size);

#endif
