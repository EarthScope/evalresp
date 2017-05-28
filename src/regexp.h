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
 * @defgroup evalresp_log evalresp Private Regular Expressions Interface
 * @ingroup evalresp
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
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
#define NSUBEXP  10

/**
 * @private
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
typedef struct regexp {
    char *startp[NSUBEXP];  /**< FIXME. */
    char *endp[NSUBEXP];  /**< FIXME. */
    char regstart;  /**< FIXME. */ /* Internal use only. */
    char reganch;  /**< FIXME. */ /* Internal use only. */
    char *regmust;  /**< FIXME. */ /* Internal use only. */
    int regmlen;  /**< FIXME. */ /* Internal use only. */
    char program[1];  /**< FIXME. */ /* Unwarranted chumminess with compiler. */
} regexp;

/**
 * @private
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
regexp *evr_regcomp(char *exp);

/**
 * @private
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
int evr_regexec(regexp *prog, char *string);

/**
 * @private
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
void evr_regsub(regexp *prog, char *source, char *dest);

/**
 * @private
 * @ingroup evalresp_regexp
 * @brief FIXME.
 */
void evr_regerror(char *s);
