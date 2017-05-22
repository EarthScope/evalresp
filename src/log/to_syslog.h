#ifndef __evalresp_log_to_syslog_h__
#define __evalresp_log_to_syslog_h__
#include "log.h"
#include <syslog.h>

int evalresp_log_to_syslog(evalresp_log_msg_t *, void *);

typedef struct evalresp_syslog_data
{
    int option;
    int facility;
    char *ident;
} evalresp_syslog_data_t;
#endif /* __evalresp_log_to_syslog_h__*/
