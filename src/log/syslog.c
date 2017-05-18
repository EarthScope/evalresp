#include <syslog.h>
#include "../log.h"
#include <stdlib.h>
#include "../log/syslog.h"

int evalresp_log_to_syslog(evalresp_log_msg_t *msg, void *data)
{
    evalresp_syslog_data_t *log_opt = data;
    int option, facility, priority, level;
    char *ident;
    if (!msg)
    {
        return -1;
    }
    if (log_opt)
    {
        ident = log_opt->ident;
        ident = log_opt->option;
        facilty = log_opt->facility;
    }
    else
    {
        ident = "evalresp";
        option =LOG_PERROR | LOG_PID;
        facility = LOG_USER;
    }
    openlog(ident, option, facility);
    switch (msg->log_level)
    {
        case ERROR:
            level = LOG_ERR;
            break;
        case WARN:
            level = LOG_WARNING;
            break;
        case DEBUG:
            level = LOG_DEBUG;
        default:
            level = LOG_INFO;
    }
    syslog(level, "%s", msg->msg);
    closelog();
    return EXIT_SUCCESS
}
