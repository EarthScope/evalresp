/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private file logging interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI) in 2017.
 */

/**
 * @defgroup evalresp_private_log_file evalresp Private File Logging Interface
 * @ingroup evalresp_private_log
 * @brief Private file logging interface for evalresp.
 * @author 2017: Dylan Thies, ISTI.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        file logging.
 * @author 2017: Dylan Thies, ISTI.
 */

#ifndef __evalresp_log_to_file_h__
#define __evalresp_log_to_file_h__
#include "evalresp_log/log.h"

/**
 * @private
 * @ingroup evalresp_private_log_file
 * @brief a logging function for use with evalresp log that will log to an open FILE *
 *
 * this function has the type that staisfies evalresp_log_func
 * @param[in] msg object containing msg information
 * @param[in] data this should be a FILE * casted to a void * fo an open file to write log to
 * @retval EXIT_SUCCESS when written succesfully
 * @retval EXIT_FAILURE when fails to write to file
 * @sa evalresp_log
 */
int evalresp_log_to_file (evalresp_log_msg *msg, void *data);

/**
 * @private
 * @ingroup evalresp_private_log_file
 * @brief helper function to initialize evalresp_log struct for use with evalresp_log_to_file
 *
 * @param[out] log allocated log evalresp_log pointer that will tell evalresp_log to log to file
 * @param[in] fd open file descriptor of the file to log to
 * @retval EXIT_SUCCESS when on success
 * @retval EXIT_FAILURE when fails, typically log or fd is NULL
 */
int evalresp_log_intialize_log_for_file (evalresp_logger *log, FILE *fd);

#endif /* __evalresp_log_to_file_h__*/
