#ifndef __EVALRESP_LOG_H__
#define __EVALRESP_LOG_H__

#include <time.h>

#ifndef MAX_LOG_MSG_LEN 
#define MAX_LOG_MSG_LEN 256
#endif /* MAX_LOG_MSG_LEN  */

typedef struct evalresp_log_msg
{
    char msg[MAX_LOG_MSG_LEN];
    int log_level;
    int verbosity_level;
    time_t timestamp;
} evalresp_log_msg_t;

typedef int (*evalresp_log_func_t)(evalresp_log_msg_t *, void *);

typedef enum log_level_ref
{
    ERROR=0,
    WARN,
    INFO,
    DEBUG
} log_level_ref_t;
const char *log_level_strs[]={"ERROR", "WARN", "INFO", "DEBUG"};

extern int evalresp_log (evalresp_log_func_t, void *, int, int, char *, ...);

#endif /* __EVALRESP_LOG_H__ */
