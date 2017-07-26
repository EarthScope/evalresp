#include <check.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "evalresp_log/examples/to_file.h"
#include "evalresp_log/examples/to_syslog.h"
#include "evalresp_log/log.h"

void
run_test (evalresp_log_func log_func, void *data, char is_syslog)
{
  ck_assert (EXIT_SUCCESS == evalresp_log_basic (log_func, data, EV_INFO, 0, "This is a TEST"));
  if (is_syslog)
  {
    return;
  }
}

/* test stderr */
START_TEST (test_log_1)
{
  run_test (NULL, NULL, 0);
}
END_TEST

/* test file */
START_TEST (test_log_2)
{
  FILE *fd;
  ck_assert (NULL != (fd = fopen ("log_test.txt", "a+")));
  run_test (evalresp_log_to_file, (void *)fd, 0);
}
END_TEST

/* test syslog */
START_TEST (test_log_3)
{
  evalresp_syslog_data data[1];
  data->ident = "test_stuff";
  data->option = LOG_PID;
  data->facility = LOG_USER;
  run_test (evalresp_log_to_syslog, (void *)data, 1);
}
END_TEST

START_TEST (test_log_4)
{
  evalresp_logger *log;
  evalresp_syslog_data *data;
  ck_assert (NULL != (log = evalresp_logger_alloc (NULL, NULL)));
  ck_assert (NULL != (data = evalresp_log_syslog_data_alloc ("test stuff", LOG_PID, LOG_USER)));
  ck_assert (EXIT_SUCCESS == evalresp_log_intialize_log_for_syslog (log, data));
  ck_assert (EXIT_SUCCESS == evalresp_log (log, EV_INFO, 0, "This is a bigger Test"));
  evalresp_logger_free (log);
  evalresp_log_syslog_data_free (data);
}
END_TEST

int
main (void)
{
  int number_failed;
  Suite *s = suite_create ("suite");
  TCase *tc = tcase_create ("case");
  tcase_add_test (tc, test_log_1);
  tcase_add_test (tc, test_log_2);
  tcase_add_test (tc, test_log_3);
  tcase_add_test (tc, test_log_4);
  suite_add_tcase (s, tc);
  SRunner *sr = srunner_create (s);
  srunner_set_xml (sr, "check-log.xml");
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return number_failed;
}
