#ifndef __EVALRESP_LOG_H__
#define __EVALRESP_LOG_H__

#include <time.h>
#include <stdarg.h>

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

typedef struct evalresp_log
{
    evalresp_log_func_t log_func;
    void *func_data;
} evalresp_log_t;

typedef enum log_level_ref
{
    ERROR=0,
    WARN,
    INFO,
    DEBUG
} log_level_ref_t;
extern const char *log_level_strs[];


/* log.c */
extern int evalresp_log (evalresp_log_t*, int, int, char *, ...);
extern int evalresp_log_basic (evalresp_log_func_t, void *, int, int, char *, ...);
extern int evalresp_log_v (evalresp_log_func_t, void *, int, int, char *, va_list);

/* log/helpers.c */
evalresp_log_t * evalresp_log_t_alloc(evalresp_log_func_t, void *);
void evalresp_log_t_free(evalresp_log_t *);
int evalresp_log_t_init(evalresp_log_t *, evalresp_log_func_t, void *);

#endif /* __EVALRESP_LOG_H__ */
