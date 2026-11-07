#ifndef EVALRESP_GLOBMATCH_H
#define EVALRESP_GLOBMATCH_H

/**
 * @file
 * @brief Declaration of the private glob matching routine.
 */

/**
 * @private
 * @ingroup evalresp_private
 * @brief Check if a string matches a globbing pattern.
 *
 * Supported semantics:
 * - `*` matches zero or more characters, e.g. `*.txt`
 * - `?` matches a single character, e.g. `a?c`
 * - `[abc]` matches any character in the set
 * - `[a-z]` matches any character in the range
 * - `[!abc]` or `[^abc]` matches any character not in the set or range
 * - `\` prefix matches the next character literally, e.g. `\*`, `\?`, `\[`
 *
 * The whole string must match the whole pattern.  Escapes are not
 * interpreted inside `[...]`, descending ranges are treated as literal
 * characters, and a trailing `\` matches a literal `\`.
 *
 * @param[in] string String to check.
 * @param[in] pattern Globbing pattern to match.
 * @returns 0 if the string does not match the pattern, or either is NULL.
 * @returns Non-zero if the string matches.
 */
int evr_globmatch (const char *string, const char *pattern);

#endif
