/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private syslog logging interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI) in 2017.
 */

/**
 * @defgroup evalresp_log_syslog evalresp Private Syslog Logging Interface
 * @ingroup evalresp_log
 * @brief Private syslog logging interface for evalresp.
 * @author 2017: Dylan Thies, ISTI.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        syslog logging.
 * @author 2017: Dylan Thies, ISTI.
 */

#ifndef __evalresp_log_to_syslog_h__
#define __evalresp_log_to_syslog_h__
#include "log.h"
#include <syslog.h>

/**
 * @private
 * @ingroup evalresp_log_syslog
 * @brief a logging function for use with evalresp log that will log to syslog
 *
 * this works in unix like environments only
 *
 * this function has the type that staisfies evalresp_log_func_t
 * @param[in] msg object containing msg information
 * @param[in] data this should be a evalresp_syslog_data_t casted to a void *, if NULL it will use defaulted values
 * @retval EXIT_SUCCESS when written succesfully
 * @retval EXIT_FAILURE when fails to write to file
 * @sa evalresp_log
 */
int evalresp_log_to_syslog(evalresp_log_msg_t *, void *);

/**
 * @private
 * @ingroup evalresp_log_syslog
 * @brief Datastructure containing configuration options for syslog
 */
typedef struct evalresp_syslog_data
{
    int option;
    int facility;
    char *ident;
} evalresp_syslog_data_t;

/**
 * @private
 * @ingroup evalresp_log_syslog
 * @brief function to free evalresp_syslog_data_t object
 *
 * @param[in] log_opt object to be free'd
 */
void evalresp_log_syslog_data_free(evalresp_syslog_data_t *);

/**
 * @private
 * @ingroup evalresp_log_syslog
 * @brief Function to allocate and initialize evalresp_syslog_data_t objects.
 *
 * @param[in] ident the ident string if NULL syslog will use program name
 * @param[in] option options to syslog's openlog function
 * @param[in] facility option to syslog's openlog function
 * @returns pointer to a evalresp_syslog_data_t object
 * @retval NULL on error
 */
evalresp_syslog_data_t * evalresp_log_syslog_data_alloc(char *, int, int);

/**
 * @private
 * @ingroup evalresp_log_syslog
 * @brief A helper function to initialize evalresp_log_t struct for use with evalresp_log_to_syslog.
 *
 * @param[out] log allocated log evalresp_log_t pointer that will tell evalresp_log to log to syslog
 * @praram[in] data evalresp_syslog_data_t that contains parameters for the log, this can be NULL
 * @retval EXIT_SUCCESS when on success
 * @retval EXIT_FAILURE when fails, typically log is NULL
 */
int evalresp_log_intialize_log_for_syslog(evalresp_log_t *, evalresp_syslog_data_t *);

#endif /* __evalresp_log_to_syslog_h__*/
