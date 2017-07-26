#include <evalresp_log/log.h>
#include <stdlib.h>

evalresp_logger *
evalresp_logger_alloc (evalresp_log_func log_func, void *func_data)
{
  evalresp_logger *log = (evalresp_logger *)calloc (1, sizeof (evalresp_logger));
  if (evalresp_logger_init (log, log_func, func_data))
  {
    return NULL;
  }
  return log;
}

void
evalresp_logger_free (evalresp_logger *log)
{
  if (!log)
  {
    return;
  }
  free (log);
}

int
evalresp_logger_init (evalresp_logger *log, evalresp_log_func log_func, void *func_data)
{
  if (!log)
  {
    return EXIT_FAILURE;
  }
  log->log_func = log_func;
  log->func_data = func_data;
  return EXIT_SUCCESS;
}
