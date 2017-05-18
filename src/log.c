#include "log.h"/* TODO fix this header location */
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int evalresp_log (evalresp_log_func_t log_func, void *log_func_data, int level, int verbosity, char *fmt, ...)
{
    evalresp_log_msg_t msg[1];
    char date_str[256];/*TODO this is tomany bytes*/
    va_list p_args;

    /* create message string */
    va_start(p_args, fmt);
    vsnprintf(msg->msg, MAX_LOG_MSG_LEN, fmt, p_args);
    va_end(p_args);
    /* setup the rest of the msg */
    msg->log_level=level;
    msg->verbosity_level=verbosity;
    msg->timestamp=time(NULL);
    /* if using a api function then return that */
    if (!log_func)
    {
        return log_func(msg, log_func_data);
    }

    /* turn the timestamp into locale std date string */
    strftime(date_str, 256, "%c", localtime(&(msg->timestamp)));
    /* just log the msg to stderr*/
    fprintf(stderr, "%s [%s] %s\n", date_str,
            (level < 4 && level >= 0) ? log_level_strs[level] : "unknown",
            msg->msg);
    return EXIT_SUCCESS;
}

