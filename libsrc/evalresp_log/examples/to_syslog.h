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
 * @defgroup evalresp_private_log_syslog evalresp Private Syslog Logging Interface
 * @ingroup evalresp_private_log
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
#include <evalresp_log/log.h>
#include <syslog.h>

/**
 * @private
 * @ingroup evalresp_private_log_syslog
 * @brief a logging function for use with evalresp log that will log to syslog
 *
 * this works in unix like environments only
 *
 * this function has the type that staisfies evalresp_log_func
 * @param[in] msg object containing msg information
 * @param[in] data this should be a evalresp_syslog_data casted to a void *, if NULL it will use defaulted values
 * @retval EXIT_SUCCESS when written succesfully
 * @retval EXIT_FAILURE when fails to write to file
 * @sa evalresp_log
 */
extern int evalresp_log_to_syslog (evalresp_log_msg *msg, void *data);

/**
 * @private
 * @ingroup evalresp_private_log_syslog
 * @brief Datastructure containing configuration options for syslog
 */
typedef struct evalresp_syslog_data_s
{
  int option;   /**< Option to pass to openlog(). */
  int facility; /**< The Facility that this program is using. THis is passed to openlog() */
  char *ident;  /**< The name of the program you want to be displayed in the syslog  */
} evalresp_syslog_data;

/**
 * @private
 * @ingroup evalresp_private_log_syslog
 * @brief function to free evalresp_syslog_data object
 *
 * @param[in] log_opt object to be free'd
 */
extern void evalresp_log_syslog_data_free (evalresp_syslog_data *log_opt);

/**
 * @private
 * @ingroup evalresp_private_log_syslog
 * @brief Function to allocate and initialize evalresp_syslog_data objects.
 *
 * @param[in] ident the ident string if NULL syslog will use program name
 * @param[in] option options to syslog's openlog function
 * @param[in] facility option to syslog's openlog function
 * @returns pointer to a evalresp_syslog_data object
 * @retval NULL on error
 */
extern evalresp_syslog_data *evalresp_log_syslog_data_alloc (char *ident, int option, int facility);

/**
 * @private
 * @ingroup evalresp_private_log_syslog
 * @brief A helper function to initialize evalresp_log struct for use with evalresp_log_to_syslog.
 *
 * @param[out] log allocated log evalresp_log pointer that will tell evalresp_log to log to syslog
 * @param[in] data evalresp_syslog_data that contains parameters for the log, this can be NULL
 * @retval EXIT_SUCCESS when on success
 * @retval EXIT_FAILURE when fails, typically log is NULL
 */
extern int evalresp_log_intialize_log_for_syslog (evalresp_logger *log, evalresp_syslog_data *data);

#endif /* __evalresp_log_to_syslog_h__*/
