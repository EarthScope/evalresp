#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalresp_log/examples/to_file.h"
#include "evalresp_log/log.h"
#include "harness/harness.h"
#ifndef _WIN32
#include "evalresp_log/examples/to_syslog.h"
#endif

static void
run_test (evalresp_log_func log_func, void *data)
{
  ER_CHECK (EXIT_SUCCESS == evalresp_log_basic (log_func, data, EV_INFO, 0, "This is a TEST"));
}

/* test stderr */
static void
test_log_1 (void)
{
  run_test (NULL, NULL);
}

/* test file */
static void
test_log_2 (void)
{
  FILE *fd;
  char line[1000] = "";
  ER_CHECK (NULL != (fd = fopen ("log_test.txt", "w+")));
  run_test (evalresp_log_to_file, (void *)fd);
  rewind (fd);
  ER_CHECK (NULL != fgets (line, sizeof (line), fd));
  ER_CHECK_MSG (NULL != strstr (line, "This is a TEST"), "unexpected log line: %s", line);
  fclose (fd);
}

#ifndef _WIN32
/* test syslog */
static void
test_log_3 (void)
{
  evalresp_syslog_data data[1];
  data->ident = "test_stuff";
  data->option = LOG_PID;
  data->facility = LOG_USER;
  run_test (evalresp_log_to_syslog, (void *)data);
}

static void
test_log_4 (void)
{
  evalresp_logger *log;
  evalresp_syslog_data *data;
  ER_CHECK (NULL != (log = evalresp_logger_alloc (NULL, NULL)));
  ER_CHECK (NULL != (data = evalresp_log_syslog_data_alloc ("test stuff", LOG_PID, LOG_USER)));
  ER_CHECK (EXIT_SUCCESS == evalresp_log_intialize_log_for_syslog (log, data));
  ER_CHECK (EXIT_SUCCESS == evalresp_log (log, EV_INFO, 0, "This is a bigger Test"));
  evalresp_logger_free (log);
  evalresp_log_syslog_data_free (data);
}
#endif

const er_test unit_log_tests[] = {
    {"1", test_log_1, NULL},
    {"2", test_log_2, NULL},
#ifndef _WIN32
    {"3", test_log_3, NULL},
    {"4", test_log_4, NULL},
#endif
    {NULL, NULL, NULL},
};
