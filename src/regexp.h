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
 */
#define NSUBEXP  10

/**
 * @private
 * @ingroup evalresp_regexp
 */
typedef struct regexp {
    char *startp[NSUBEXP];
    char *endp[NSUBEXP];
    char regstart; /* Internal use only. */
    char reganch; /* Internal use only. */
    char *regmust; /* Internal use only. */
    int regmlen; /* Internal use only. */
    char program[1]; /* Unwarranted chumminess with compiler. */
} regexp;

/**
 * @private
 * @ingroup evalresp_regexp
 */
regexp *evr_regcomp(char *exp);

/**
 * @private
 * @ingroup evalresp_regexp
 */
int evr_regexec(regexp *prog, char *string);

/**
 * @private
 * @ingroup evalresp_regexp
 */
void evr_regsub(regexp *prog, char *source, char *dest);

/**
 * @private
 * @ingroup evalresp_regexp
 */
void evr_regerror(char *s);
