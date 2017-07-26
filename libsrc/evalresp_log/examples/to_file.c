#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <evalresp_log/examples/to_file.h>
#include <evalresp_log/log.h>

int
evalresp_log_intialize_log_for_file (evalresp_logger *log, FILE *fd)
{
  if (!fd || !log)
  {
    return EXIT_FAILURE;
  }
  log->log_func = evalresp_log_to_file;
  log->func_data = (void *)fd;
  return EXIT_SUCCESS;
}

int
evalresp_log_to_file (evalresp_log_msg *msg, void *data)
{
  FILE *fd = data;
  char date_str[256]; /*TODO this is tomany bytes*/
  if (!fd)
  {
    return EXIT_FAILURE;
  }
  /* turn the timestamp into locale std date string */
  strftime (date_str, 256, "%c", localtime (&(msg->timestamp)));
  /* just log the msg to stderr*/
  fprintf (fd, "%s [%s] %s\n", date_str,
           (msg->log_level < 4 && msg->log_level >= 0) ? log_level_strs[msg->log_level] : "unknown",
           msg->msg);
  return EXIT_SUCCESS;
}
