#include <evalresp_log/log.h>
/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private regular expressions interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_private_regexp evalresp Private Regular Expressions Interface
 * @ingroup evalresp_private
 * @brief Private regular expressions interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        regular expressions.
 */

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
/*
 *   8/28/2001 -- [ET]  Added parameter lists to function declarations.
 *   1/18/2006 -- [ET]  Renamed functions to prevent name clashes with
 *                      other libraries.
 */

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Maximum number of subexpressions.
 */
#define NSUBEXP 10

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Regular expression data type.
 */
typedef struct regexp
{
  char *startp[NSUBEXP]; /**< Start pointers for subexpressions. */
  char *endp[NSUBEXP];   /**< End pointers for subexpressions. */
  char regstart;         /**< Internal use only. */
  char reganch;          /**< Internal use only. */
  char *regmust;         /**< Internal use only. */
  int regmlen;           /**<  Internal use only. */
  char program[1];       /**< Unwarranted chumminess with compiler. */
} regexp;

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Compile a regular expression into internal code.
 * @param[in] exp Regular expression string.
 * @param[in] log Logging structure.
 * @returns Compiled regular expression object.
 * @note We can't allocate space until we know how big the compiled form will
 *       be, but we can't compile it (and thus know how big it is) until we've
 *       got a place to put the code. So we cheat: we compile it twice, once
 *       with code generation turned off and size counting turned on, and once
 *       "for real". This also means that we don't allocate space until we are
 *       sure that the thing really will compile successfully, and we never
 *       have to move the code and thus invalidate pointers into it. (Note
 *       that it has to be in one piece because free() must be able to free it
 *       all.)
 * @warning Beware that the optimization-preparation code in here knows about
 *          some of the structure of the compiled regexp.
 */
regexp *evr_regcomp (char *exp, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Match a regexp against a string.
 * @param[in] prog Compiled regular expression object.
 * @param[in] string String to match against.
 * @param[in] log Logging structure.
 * @returns @c 0 on error or no match.
 * @returns Pointer to position in string if match.
 */
int evr_regexec (regexp *prog, char *string, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Perform substitutions after a regexp match.
 * @param[in] prog Compiled regular expression object.
 * @param[in] source Source string.
 * @param[out] dest Destination string.
 * @param[in] log Logging structure.
 */
void evr_regsub (regexp *prog, char *source, char *dest, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief Report regular expression error.
 * @param[in] s Error message string.
 */
void evr_regerror (char *s);
