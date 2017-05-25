#include "../log.h"
#include <stdlib.h>

evalresp_log_t *evalresp_log_t_alloc(evalresp_log_func_t log_func, void *func_data)
{
    evalresp_log_t *log = (evalresp_log_t *)calloc(1, sizeof(evalresp_log_t));
    if (evalresp_log_t_init(log, log_func, func_data))
    {
        return NULL;
    }
    return log;
}
void evalresp_log_t_free(evalresp_log_t *log)
{
    if (!log)
    {
        return;
    }
    free(log);
}
int evalresp_log_t_init(evalresp_log_t *log, evalresp_log_func_t log_func, void *func_data)
{
    if (!log)
    {
        return EXIT_FAILURE;
    }
    log->log_func = log_func;
    log->func_data = func_data;
    return EXIT_SUCCESS;
}
