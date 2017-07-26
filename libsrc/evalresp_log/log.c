#include "log.h" /* TODO fix this header location */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char *log_level_strs[] = {"ERROR", "WARN", "INFO", "DEBUG"};

int
evalresp_log (evalresp_logger *log, int level, int verbosity, char *fmt, ...)
{
  va_list p_args;
  int status;
  va_start (p_args, fmt);
  if (!log)
  {
    status = evalresp_log_v (NULL, NULL, level, verbosity, fmt, p_args);
  }
  else
  {
    status = evalresp_log_v (log->log_func, log->func_data, level, verbosity, fmt, p_args);
  }
  va_end (p_args);
  return status;
}

int
evalresp_log_basic (evalresp_log_func log_func, void *log_func_data, int level, int verbosity, char *fmt, ...)
{
  int status;
  va_list p_args;
  va_start (p_args, fmt);
  status = evalresp_log_v (log_func, log_func_data, level, verbosity, fmt, p_args);
  va_end (p_args);
  return status;
}

int
evalresp_log_v (evalresp_log_func log_func, void *log_func_data, int level, int verbosity, char *fmt, va_list args)
{
  evalresp_log_msg msg[1];
  char date_str[256]; /*TODO this is tomany bytes*/

  /* create message string */
  vsnprintf (msg->msg, MAX_LOG_MSG_LEN, fmt, args);
  /* setup the rest of the msg */
  msg->log_level = level;
  msg->verbosity_level = verbosity;
  msg->timestamp = time (NULL);
  /* if using a api function then return that */
  if (log_func)
  {
    return log_func (msg, log_func_data);
  }

  /* turn the timestamp into locale std date string */
  strftime (date_str, 256, "%c", localtime (&(msg->timestamp)));
  /* just log the msg to stderr*/
  fprintf (stderr, "%s [%s] %s\n", date_str,
           (level < 4 && level >= 0) ? log_level_strs[level] : "unknown",
           msg->msg);
  return EXIT_SUCCESS;
}
