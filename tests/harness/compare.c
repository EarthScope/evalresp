#include "compare.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 16

typedef struct
{
  char *text;  /* whole file */
  char **line; /* NUL-terminated lines inside text */
  int n;
} lines;

static void
free_lines (lines *l)
{
  free (l->text);
  free (l->line);
  l->text = NULL;
  l->line = NULL;
  l->n = 0;
}

static int
read_lines (const char *path, lines *l)
{
  FILE *f = fopen (path, "rb");
  size_t cap = 65536, len = 0, n;
  char *p;
  int count = 0, cap_lines = 0;

  l->text = NULL;
  l->line = NULL;
  l->n = 0;
  if (!f)
    return -1;
  if (!(l->text = malloc (cap)))
  {
    fclose (f);
    return -1;
  }
  while ((n = fread (l->text + len, 1, cap - len - 1, f)) > 0)
  {
    len += n;
    if (len + 1 >= cap)
    {
      char *tmp = realloc (l->text, cap * 2);
      if (!tmp)
      {
        fclose (f);
        free_lines (l);
        return -1;
      }
      l->text = tmp;
      cap *= 2;
    }
  }
  fclose (f);
  l->text[len] = '\0';

  for (p = l->text; p < l->text + len;)
  {
    char *end = memchr (p, '\n', l->text + len - p);
    char *next = end ? end + 1 : l->text + len;
    if (!end)
      end = l->text + len;
    if (end > p && end[-1] == '\r')
      --end;
    *end = '\0';
    if (count == cap_lines)
    {
      char **tmp = realloc (l->line, (cap_lines ? 2 * cap_lines : 256) * sizeof (*tmp));
      if (!tmp)
      {
        free_lines (l);
        return -1;
      }
      l->line = tmp;
      cap_lines = cap_lines ? 2 * cap_lines : 256;
    }
    l->line[count++] = p;
    p = next;
  }
  l->n = count;
  return 0;
}

/* parse exactly ncols whitespace-separated numbers */
static int
parse_floats (const char *line, int ncols, double *out)
{
  int n = 0;
  const char *p = line;
  while (1)
  {
    char *end;
    while (isspace ((unsigned char)*p))
      ++p;
    if (!*p)
      break;
    if (n == ncols)
      return 0;
    out[n] = strtod (p, &end);
    if (end == p || (*end && !isspace ((unsigned char)*end)))
      return 0;
    ++n;
    p = end;
  }
  return n == ncols;
}

static int
equal_values (double a, double b, double tol, int absolute)
{
  double magnitude;
  if (a != a || b != b)
    return a != a && b != b;
  if (absolute)
    return fabs (a - b) <= tol;
  magnitude = fabs (a) > fabs (b) ? fabs (a) : fabs (b);
  if (magnitude < tol)
    return fabs (a - b) <= tol;
  return fabs (a - b) / magnitude <= tol;
}

/* drop the text up to and including the first '[' */
static char *
strip_timestamp (char *line)
{
  char *p = strchr (line, '[');
  return p ? p + 1 : line;
}

/* "...OUTPUT V<digit>... >>" -> "...OUTPUT VROBOT >>"; returns 1 if rewritten */
static int
mask_version (const char *in, char *out, size_t out_size)
{
  static const char tag[] = "EVALRESP RESPONSE OUTPUT V";
  const char *p = strstr (in, tag), *v, *q, *last = NULL;
  if (!p)
    return 0;
  v = p + sizeof (tag) - 1;
  if (!isdigit ((unsigned char)*v))
    return 0;
  for (q = v + 1; (q = strstr (q, " >>")); ++q)
    last = q;
  if (!last || last < v + 2)
    return 0;
  if ((size_t)(v - in) + 5 + strlen (last) + 1 > out_size)
    return 0;
  memcpy (out, in, v - in);
  strcpy (out + (v - in), "ROBOT");
  strcat (out, last);
  return 1;
}

int
er_compare_files (const char *result, const char *target, const er_cmp *opts, char *msg, size_t msg_size)
{
  lines res, tar;
  int i, rc = 0;
  double tol = opts->tol > 0 ? opts->tol : ER_TOL_DEFAULT;

  if (opts->ncols > MAX_COLS)
  {
    snprintf (msg, msg_size, "at most %d columns are supported", MAX_COLS);
    return 1;
  }
  if (read_lines (result, &res))
  {
    snprintf (msg, msg_size, "cannot read result %s", result);
    return 1;
  }
  if (read_lines (target, &tar))
  {
    snprintf (msg, msg_size, "cannot read expected %s", target);
    free_lines (&res);
    return 1;
  }

  for (i = 0; i < res.n && i < tar.n && !rc; ++i)
  {
    char masked[4096];
    const char *r = res.line[i], *t = tar.line[i];
    double rv[MAX_COLS], tv[MAX_COLS];

    if (opts->log_text)
    {
      r = strip_timestamp (res.line[i]);
      if (mask_version (r, masked, sizeof (masked)))
        r = masked;
    }

    if (opts->ncols > 0 && parse_floats (r, opts->ncols, rv) && parse_floats (t, opts->ncols, tv))
    {
      int c;
      for (c = 0; c < opts->ncols; ++c)
      {
        if (!equal_values (rv[c], tv[c], tol, opts->absolute))
        {
          snprintf (msg, msg_size, "%s line %d column %d: %.10g differs from expected %.10g (tolerance %g)", result,
                    i + 1, c + 1, rv[c], tv[c], tol);
          rc = 1;
          break;
        }
      }
    }
    else if (strcmp (r, t))
    {
      snprintf (msg, msg_size, "%s line %d: \"%.200s\" differs from expected \"%.200s\"", result, i + 1, r, t);
      rc = 1;
    }
  }

  if (!rc && res.n != tar.n)
  {
    snprintf (msg, msg_size, "%s has %d lines, expected %d", result, res.n, tar.n);
    rc = 1;
  }
  free_lines (&res);
  free_lines (&tar);
  return rc;
}
