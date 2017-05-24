#ifndef __EVALRESP_LOG_H__
#define __EVALRESP_LOG_H__

#include <time.h>
#include <stdarg.h>

#ifndef MAX_LOG_MSG_LEN
#define MAX_LOG_MSG_LEN 256
#endif /* MAX_LOG_MSG_LEN  */
/** @file log.h
 *  @author Dylan Thies
 *  @ brief header for evalresp logging function
 */

/**
 * @brief data type of the object that gets based to a logging function
 */
typedef struct evalresp_log_msg
{
    char msg[MAX_LOG_MSG_LEN]; /**< the message itself */
    int log_level; /**<  at what log_level_ref this message should be at */
    int verbosity_level; /**< the verbosity level that this message is */
    time_t timestamp; /**< seconds since epoch that this message was created */
} evalresp_log_msg_t;

/** @brief data type of the logging function */
typedef int (*evalresp_log_func_t)(evalresp_log_msg_t *, void *);

/** @brief data type that is for creating a persistent log object */
typedef struct evalresp_log
{
    evalresp_log_func_t log_func; /**< the function that the loger should call back */
    void *func_data; /**< a pointer to the data portion that should be sent to the callback function */
} evalresp_log_t;

/** @brief an enum of the different logging levels to expect in evalresp */
typedef enum log_level_ref
{
    ERROR=0, /**< level when reporting an error */
    WARN, /**< level when a warning occurs */
    INFO, /**< the level that information should be presented to user */
    DEBUG /**< level for the most information */
} log_level_ref_t;

/** const array  of the log levels  in plain text */
extern const char *log_level_strs[];


/* log.c */
/**
 * @brief send log message to log using the log struct
 *
 * example use of how to use the log, note the data is null because the
 * log_func reqires data to be null in this case

 #include "log.h"
 int log_func(evalresp_log_msg_t *msg, void *data)
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
    evalresp_log_t log[1];
    evalresp_log_t_init(log, log_func, NULL);

    //do something
    if ((retval = something()))
    {
        evalresp_log(log, WARN, 2, "Something didn't work right, something"
                                   " at line %d failed with error cod %d",
                                   __LINE__ - 4, retval);
    }
 }

 *
 * @param[in] log structure containing the information for the logging function and data required to control the log
 * @param[in] level the logging level the msg is intended for
 * $param[in] verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] ... any additional variables needed for fmt
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * @sa { evalresp_log_t evalresp_log_t_alloc evalresp_log_t_free evalresp_log_t_init evalresp_log_basic evalresp_log_v }
 * @file log.c
 */
extern int evalresp_log (evalresp_log_t*, int, int, char *, ...);
/**
 * @brief send log message to log using base parameters
 *
 * example use of how to use the log, note the data is null because the
 * log_func reqires data to be null in this case

 #include "log.h"
 int log_func(evalresp_log_msg_t *msg, void *data)
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
        evalresp_log_basic(log_func, NULL, WARN, 2, "Something didn't work right, something"
                                   " at line %d failed with error cod %d",
                                   __LINE__ - 4, retval);
    }
 }

 *
 * @param[in] log_func the logging function used for controlled output of log
 * @param[in] log_func_data data needed by the logging function to better control the logging function
 * @param[in] level level the logging level the msg is intended for
 * $param[in] verbosity verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] ... any additional variables needed for fmt
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * \sa { evalresp_log_t evalresp_log_t_alloc evalresp_log_t_free evalresp_log_t_init evalresp_log_basic evalresp_log_v }
 */
extern int evalresp_log_basic (evalresp_log_func_t, void *, int, int, char *, ...);
/**
 * @brief send log message to log using base parameters and va_list instead of printf like varadic function
 *
 * this is an extension of evalresp_log_basic that takes a va_list, really should only be used by a wrap[ping function
 * for the logging, suchas evalresp_log and evalresp_log_basic
 *
 * @param[in] log_func the logging function used for controlled output of log
 * @param[in] log_func_data data needed by the logging function to better control the logging function
 * @param[in] level level the logging level the msg is intended for
 * $param[in] verbosity verbosity the verbosity level the mesg is intended for
 * @param[in] fmt a format string that follows the printf style format
 * @param[in] args va_list containing variables needed for fmt, follow vprintf
 * @retval EXIT_SUCCESS when succesfully called
 * @retval EXIT_FAILURE if something went wrong
 *
 * \sa { evalresp_log_t evalresp_log_t_alloc evalresp_log_t_free evalresp_log_t_init evalresp_log_basic evalresp_log_v }
 */
extern int evalresp_log_v (evalresp_log_func_t, void *, int, int, char *, va_list);

/* log/helpers.c */
/**
 * @brief allocate a evalresp_log_t object and initialize it to values passed to this function
 *
 * @param[in] log_func evalresp_log_func_t or function pointer to logging function to use.
 * @param[in] func_data additional data needed to control log_func
 * @returns pointer to log object
 * @retval NULL on error
 */
extern evalresp_log_t *evalresp_log_t_alloc(evalresp_log_func_t, void *);
/**
 * @brief free a log object
 *
 * @param[in] log
 */
extern void evalresp_log_t_free(evalresp_log_t *);
/**
 * @brief initalize an already allocated log with log_func and func_data
 *
 * @param[in,out] log pointer to log object being initialized
 * @param[in] log_func logging function
 * @param[in] func_data the additional data to control log_func
 * @retval EXIT_SUCCESS if initialized successfully
 * @retval EXIT_FAILURE if failed to initialize, generaly log was NULL
 */
extern int evalresp_log_t_init(evalresp_log_t *, evalresp_log_func_t, void *);

#endif /* __EVALRESP_LOG_H__ */
