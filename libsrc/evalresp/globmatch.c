/***************************************************************************
 * Glob pattern matching.
 *
 * Derived from ms_globmatch() in the miniSEED Library (selection.c).
 *
 * Copyright (c) 2024 Chad Trabant, EarthScope Data Services
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <string.h>

#include "globmatch.h"

static int match_charclass (const char **pp, unsigned char c);

int
evr_globmatch (const char *string, const char *pattern)
{
  const char *star_p = NULL;   /* position of the most recent '*' in pattern */
  const char *star_s = NULL;   /* position in string when that '*' was seen */
  unsigned char star_skip = 0; /* byte to skip past on backtrack, or 0 if none */
  unsigned char c;

  if (string == NULL || pattern == NULL)
    return 0;

  for (;;)
  {
    c = (unsigned char)*pattern++;

    switch (c)
    {
    case '\0':
      /* End of pattern: must also be end of string unless a previous '*'
         can consume more characters. */
      if (*string == '\0')
        return 1;
      if (star_p)
        goto star_backtrack;
      return 0;

    case '?':
      if (*string == '\0')
        goto star_backtrack;
      string++;
      break;

    case '*':
      /* Collapse consecutive '*' */
      while (*pattern == '*')
        pattern++;

      /* Trailing '*' matches everything */
      if (*pattern == '\0')
        return 1;

      /* Determine the literal byte (if any) following the '*'. If it is a
         literal, we can skip string characters that cannot match it. */
      {
        unsigned char next = (unsigned char)*pattern;

        if (next == '\\' && pattern[1])
          next = (unsigned char)pattern[1];
        else if (next == '?' || next == '[')
          next = 0; /* not a literal; skip the optimization */

        star_skip = next;

        if (star_skip)
        {
          const char *found = strchr (string, star_skip);
          if (found == NULL)
            return 0; /* required literal cannot occur in remaining string */
          string = found;
        }
      }

      star_p = pattern - 1;
      star_s = string;
      continue;

    case '[':
    {
      const char *pp = pattern;
      if (*string == '\0')
        goto star_backtrack;
      if (!match_charclass (&pp, (unsigned char)*string))
        goto star_backtrack;
      pattern = pp;
      string++;
      break;
    }

    case '\\':
      if (*pattern)
        c = (unsigned char)*pattern++;
      /* FALLTHROUGH */

    default:
      if ((unsigned char)*string != c)
        goto star_backtrack;
      string++;
      break;
    }

    continue;

  star_backtrack:
    /* If there was a previous '*', backtrack: let it consume one more
       character and retry from pattern just after that '*'. */
    if (star_p)
    {
      if (*star_s == '\0')
        return 0;

      star_s++;

      /* Reuse the saved fast-forward byte so we don't walk non-matching
         characters one at a time on each retry. */
      if (star_skip)
      {
        const char *found = strchr (star_s, star_skip);
        if (found == NULL)
          return 0;
        star_s = found;
      }
      else if (*star_s == '\0')
      {
        return 0;
      }

      string = star_s;
      pattern = star_p + 1;
      continue;
    }
    return 0;
  }
}

/* Match c against the character class at *pp, which points just past '['.
 * A leading '^' or '!' negates the class.  On return *pp is advanced past
 * the closing ']'.  Returns 1 if c matches the class, 0 otherwise. */
static int
match_charclass (const char **pp, unsigned char c)
{
  const char *p;
  int negate = 0;
  int matched = 0;

  if (pp == NULL || *pp == NULL)
    return 0;

  p = *pp;

  if (*p == '^' || *p == '!')
  {
    negate = 1;
    p++;
  }

  /* A leading ']' is literal */
  if (*p == ']')
  {
    matched = (c == ']');
    p++;
  }

  /* A leading '-' is literal */
  if (*p == '-')
  {
    matched |= (c == '-');
    p++;
  }

  while (*p && *p != ']')
  {
    unsigned char pc = (unsigned char)*p;

    if (p[1] == '-' && p[2] && p[2] != ']' && pc <= (unsigned char)p[2])
    {
      /* Ascending range X-Y */
      matched |= (c >= pc && c <= (unsigned char)p[2]);
      p += 3;
    }
    else
    {
      matched |= (c == pc);
      p++;
    }
  }

  /* No closing ']' never matches */
  if (*p != ']')
  {
    *pp = p;
    return 0;
  }

  *pp = p + 1;

  return negate ? !matched : matched;
}
