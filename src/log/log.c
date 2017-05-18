#include "log.h"/* TODO fix this header location */
#include <time.h>
#include <stdarg.h>
#include <stdio.h>


int evalresp_log (evalresp_log_func_t log_func, int level, int verbosity, char *fmt, ...)
{
    evalresp_log_msg_t msg[1];
    char date_str[256];/*TODO this is tomany bytes*/
    va_list p_args;
    va_start(p_args, fmt);
    vsnprintf(msg->msg, MAX_LOG_MSG_LEN, fmt, args);
    va_end(p_args)
    msg->log_level=level;
    msg->verbosity_level=verbosity;
    msg->timestamp=time(NULL);
    if (!log_func)
    {
        return log_func(msg);
    }
    strftime(date_str, 256, "%c", localtime(&(msg->timestamp)));
    fprintf(stderr, "%s [%s] %s\n", date_str,
            (level < 4 && level >= 0) ? log_level_strs[level] : "unknown",
            msg->msg);
    return EXIT_SUCCESS;
}

