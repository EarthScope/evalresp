#ifndef __evalresp_log_to_file_h__
#define __evalresp_log_to_file_h__
#include "../log.h"
/** @file to_file.h
 *  @author Dylan Thies
 *  @ brief header for evalresp logging function api for logging to file
 */

/**
 * @brief a logging function for use with evalresp log that will log to an open FILE *
 *
 * this function has the type that staisfies evalresp_log_func_t
 * @param[in] msg object containing msg information
 * @param[in] data this should be a FILE * casted to a void * fo an open file to write log to
 * @retval EXIT_SUCCESS when written succesfully
 * @retval EXIT_FAILURE when fails to write to file
 * @sa evalresp_log
 */
int evalresp_log_to_file(evalresp_log_msg_t *, void *);
/**
 * @brief helper function to initialize evalresp_log_t struct for use with evalresp_log_to_file
 *
 * @param[out] log allocated log evalresp_log_t pointer that will tell evalresp_log to log to file
 * @praram[in] fd open file descriptor of the file to log to
 * @retval EXIT_SUCCESS when on success
 * @retval EXIT_FAILURE when fails, typically log or fd is NULL
 */
int evalresp_log_intialize_log_for_file(evalresp_log_t *, FILE *);

#endif /* __evalresp_log_to_file_h__*/
