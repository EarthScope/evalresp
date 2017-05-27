/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private X2R logging interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_x2r_log evalresp Private X2R Logging Interface
 * @ingroup evalresp_x2r
 * @brief Private X2R logging interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        X2R logging.
 */

#ifndef X2R_LOG_H
#define X2R_LOG_H

#include <stdio.h>

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
#define X2R_SILENT 0

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
#define X2R_ERROR 1

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
#define X2R_WARN 2

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
#define X2R_INFO 3

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
#define X2R_DEBUG 4

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
typedef struct {
    FILE *log;
    int level;
} x2r_log;

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
int x2r_alloc_log(int level, FILE *file, x2r_log **log);

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
int x2r_free_log(x2r_log *log, int status);

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
void x2r_debug(x2r_log *log, char *template, ...);

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
void x2r_info(x2r_log *log, char *template, ...);

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
void x2r_warn(x2r_log *log, char *template, ...);

/**
 * @private
 * @ingroup evalresp_x2r_log
 */
int x2r_error(x2r_log *log, int status, char *template, ...);

#endif
