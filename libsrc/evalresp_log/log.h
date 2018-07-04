/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private logging interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI) in 2017.
 */

/**
 * @defgroup evalresp_private_log evalresp Private Logging Interface
 * @ingroup evalresp_private
 * @brief Private logging interface for evalresp.
 * @author 2017: Dylan Thies, ISTI.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        logging.
 * @author 2017: Dylan Thies, ISTI.
 */

#ifndef __EVALRESP_LOG_H__
#define __EVALRESP_LOG_H__

#include <stdarg.h>
#include <time.h>

#ifndef MAX_LOG_MSG_LEN
#define MAX_LOG_MSG_LEN 256
#endif /* MAX_LOG_MSG_LEN  */

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Object created by evalresp library logging functions that will be
 *        passed to logging function if provided.
 */
typedef struct evalresp_log_msg_s
{
  char msg[MAX_LOG_MSG_LEN]; /**< the message itself */
  int log_level;             /**< what log_level_ref this message should be at */
  int verbosity_level;       /**< the verbosity level that this message is */
  time_t timestamp;          /**< seconds since epoch that this message was created */
} evalresp_log_msg;

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief A convenience data type of the logging function.
 */
typedef int (*evalresp_log_func) (evalresp_log_msg *, void *);

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Data type that is for creating a persistent log object.
 *
 * This while be set up by the user of the library. with the information it
 * needs to overwrite logging
 */
typedef struct evalresp_logger_s
{
  evalresp_log_func log_func; /**< the function that the logger should call back */
  void *func_data;            /**< a pointer to the data portion that should be sent to the callback function */
} evalresp_logger;

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief an enum of the different logging levels to expect in evalresp
 */
typedef enum log_level_ref_e {
  EV_ERROR = 0, /**< level when reporting an error */
  EV_WARN,      /**< level when a warning occurs */
  EV_INFO,      /**< the level that information should be presented to user */
  EV_DEBUG      /**< level for the most information */
} log_level_ref;

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Array of the log levels in plain text.
 * 
 * Using the values from log_level_ref will directly correspond to the
 * plaintext. Eg, log_level_strs[ERROR] == "ERROR".
 */
extern const char *log_level_strs[];

/* log.c */

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief A function to send log message to log using the log structure.
 *
 * This function is used in the evalresp library to allow external developers
 * to pass their own logging function as well as data needed for the function
 * to operate.
 * 
 * Example use of how to use, note the data is null because the log_func
 * requires data to be null in this case.

 @verbatim
 #include <evalresp_log/log.h>
 int log_func(evalresp_log_msg *msg, void *data)
 {
    if (!data)
    {
        return EXIT_FAILURE;
    }
    if(msg->verbosity_level < 2)
    {
        printf("[%d]%s: %s\n", msg->timestamp, log_level_strs[msg->log_level, msg->msg);
    } else {
        fprintf(stderr, "[%d]%s: %s\n", msg->timestamp, log_level_strs[msg->log_level, msg->msg);
    }
    return EXIT_SUCCESS;
 }

 void evalresp_lib_function()
 {
    int retval = 0;
    evalresp_logger log[1];
    evalresp_logger_init(log, log_func, NULL);

    //do something
    if ((retval = something()))
    {
        evalresp_log(log, EV_WARN, 2, "Something didn't work right, something"
                                   " at line %d failed with error cod %d",
                                   __LINE__ - 4, retval);
    }
 }
@endverbatim

 * In this Example log_func is the user defined function that will be passed
 * to evalresp_log.
 *
 * To develop against libevalresp:

@verbatim
#include <evalresp_log/log.h>
#include <stdio.h>
#include <stdlib.h>

int my_log_func(evalresp_log_msg *msg, void *data)
{
    FILE * fd = (FILE *) data
    if(!data)
    {
        fd = stdout;
    }
    if(msg->verbosity_level < 2)
    {
        fprintf(fd, "[%d]%s: %s\n", msg->timestamp, log_level_strs[msg->log_level, msg->msg);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    evalresp_logger log[1]; // a quick way to allocate a pointer at compile time
    FILE * fd = fopen("MyLog", "w");

    evalresp_logger_init(log, my_log_func, (void *) fd);

    evalresp_function(1, 3.5, "AU", "00", "BHZ", log); //a function call to something in the evalresp library
    return EXIT_SUCCESS
}
@endverbatim

 * evalresp_function is a made up function but will be typical of functions
 * that use evalresp_log
 *
 * @param[in] log structure containing the information for the logging
 *                function and data required to control the log
 * @param[in] level the logging level the msg is intended for
 * @param[in] verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] ... any additional variables needed for fmt
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * @sa { evalresp_logger evalresp_logger_alloc evalresp_logger_free evalresp_logger_init evalresp_log_basic evalresp_log_v }
 */
extern int evalresp_log (evalresp_logger *log, int level, int verbosity, char *fmt, ...);

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief send log message to log using explicit log function pointer and data
 *
 * This function is used in the evalresp library to allow external developers
 * to pass their own logging function as well as data needed for the function
 * to operate.
 *
 * Example use of how to use the log, note the data is null because the
 * log_func requires data to be null in this case:

@verbatim
 #include "log.h"
 int log_func(evalresp_log_msg *msg, void *data)
 {
    if (!data)
    {
        return EXIT_FAILURE;
    }
    if(msg->verbosity_level < 2)
    {
        printf("[%d]%s: %s\n", msg->timestamp, log_level_strs[msg->log_level, msg->msg);
    } else {
        fprintf(stderr, "[%d]%s: %s\n", msg->timestamp, log_level_strs[msg->log_level, msg->msg);
    }
    return EXIT_SUCCESS;
 }

 void function()
 {
    int retval = 0;

    //do something
    if ((retval = something()))
    {
        evalresp_log_basic(log_func, NULL, EV_WARN, 2, "Something didn't work right, something"
                                   " at line %d failed with error cod %d",
                                   __LINE__ - 4, retval);
    }
 }

@endverbatim
 *
 * @param[in] log_func the logging function used for controlled output of log
 * @param[in] log_func_data data needed by the logging function to better control the logging function
 * @param[in] level level the logging level the msg is intended for
 * @param[in] verbosity verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] ... any additional variables needed for fmt
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * \sa { evalresp_logger evalresp_logger_alloc evalresp_logger_free evalresp_logger_init evalresp_log_basic evalresp_log_v }
 */
extern int evalresp_log_basic (evalresp_log_func log_func, void *log_func_data, int level, int verbosity, char *fmt, ...);

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Send log message to log using base parameters and va_list instead of
 *        printf like varadic function
 *
 * This is an extension of evalresp_log_basic that takes a va_list, really
 * should only be used by a wrap[ping function for the logging, such as evalresp_log and evalresp_log_basic
 *
 * @param[in] log_func the logging function used for controlled output of log
 * @param[in] log_func_data data needed by the logging function to better control the logging function
 * @param[in] level level the logging level the msg is intended for
 * @param[in] verbosity verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] args va_list containing variables needed for fmt, follow vprintf
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * \sa { evalresp_logger evalresp_logger_alloc evalresp_logger_free evalresp_logger_init evalresp_log_basic evalresp_log_v }
 */
extern int evalresp_log_v (evalresp_log_func log_func, void *log_func_data, int level, int verbosity, char *fmt, va_list args);

/* log/helpers.c */
/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Allocate a evalresp_logger object and initialize it to values passed
 *        to this function.
 *
 * The resulting object will be created using malloc so be sure to free it.
 *
 * @param[in] log_func evalresp_log_func or function pointer to logging function to use.
 * @param[in] func_data additional data needed to control log_func
 * @returns pointer to log object
 * @retval NULL on error
 */
extern evalresp_logger *evalresp_logger_alloc (evalresp_log_func log_func, void *func_data);

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Free a log object.
 *
 * Currently a wrapper for free().
 *
 * @param[in] log Logging structure.
 */
extern void evalresp_logger_free (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_log
 * @brief Initalize an already allocated evalresp_logger object with the log_func and func_data.
 *
 * @param[in,out] log pointer to log object being initialized
 * @param[in] log_func logging function
 * @param[in] func_data the additional data to control log_func
 * @retval EXIT_SUCCESS if initialized successfully
 * @retval EXIT_FAILURE if failed to initialize, generaly log was NULL
 */
extern int evalresp_logger_init (evalresp_logger *log, evalresp_log_func log_func, void *func_data);

#endif /* __EVALRESP_LOG_H__ */
