#include <stdlib.h>
#include <string.h>

#include "evalresp/log_context.h"
#include "evalresp/public_api.h"
#include "harness/harness.h"

#define MAX_CAPTURED 16

typedef struct
{
  int n;
  int level[MAX_CAPTURED];
  char msg[MAX_CAPTURED][MAX_LOG_MSG_LEN];
} capture;

static int
capture_msg (evalresp_log_msg *msg, void *data)
{
  capture *c = data;

  if (c->n < MAX_CAPTURED)
  {
    c->level[c->n] = msg->log_level;
    strcpy (c->msg[c->n], msg->msg);
    c->n++;
  }
  return EXIT_SUCCESS;
}

/* errors and warnings are prefixed; other levels are not */
static void
test_log_context_levels (void)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_log_context ctx;

  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  evalresp_log_context_init (&ctx, parent, "%s.%s", "IU", "ANMO");
  evalresp_log (&ctx.logger, EV_ERROR, 0, "bad %d", 1);
  evalresp_log (&ctx.logger, EV_WARN, 0, "careful");
  evalresp_log (&ctx.logger, EV_INFO, 0, "note");
  evalresp_log (&ctx.logger, EV_DEBUG, 0, "detail");

  ER_CHECK (c.n == 4);
  ER_CHECK_MSG (!strcmp (c.msg[0], "IU.ANMO: bad 1"), "got '%s'", c.msg[0]);
  ER_CHECK_MSG (!strcmp (c.msg[1], "IU.ANMO: careful"), "got '%s'", c.msg[1]);
  ER_CHECK_MSG (!strcmp (c.msg[2], "note"), "got '%s'", c.msg[2]);
  ER_CHECK_MSG (!strcmp (c.msg[3], "detail"), "got '%s'", c.msg[3]);
  ER_CHECK (c.level[0] == EV_ERROR && c.level[1] == EV_WARN);
  ER_CHECK (c.level[2] == EV_INFO && c.level[3] == EV_DEBUG);
}

/* contexts nest from the outermost to the innermost */
static void
test_log_context_nested (void)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_log_context file, channel;

  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  evalresp_log_context_init (&file, parent, "station.xml");
  evalresp_log_context_init (&channel, &file.logger, "IU.ANMO..BHZ");
  evalresp_log (&channel.logger, EV_WARN, 0, "careful");

  ER_CHECK (c.n == 1);
  ER_CHECK_MSG (!strcmp (c.msg[0], "station.xml: IU.ANMO..BHZ: careful"), "got '%s'", c.msg[0]);
}

/* a context whose prefix is already named by an enclosing one adds nothing */
static void
test_log_context_not_repeated (void)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_log_context file, channel, inner;

  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  evalresp_log_context_init (&file, parent, "/data/RESP.IU.ANMO..BHZ");
  evalresp_log_context_init (&channel, &file.logger, "IU.ANMO..BHZ");
  evalresp_log_context_init (&inner, &channel.logger, "IU.ANMO..BHZ");
  evalresp_log (&channel.logger, EV_WARN, 0, "careful");
  evalresp_log (&inner.logger, EV_ERROR, 0, "bad");
  evalresp_log (&channel.logger, EV_INFO, 0, "note");

  ER_CHECK (c.n == 3);
  ER_CHECK_MSG (!strcmp (c.msg[0], "/data/RESP.IU.ANMO..BHZ: careful"), "got '%s'", c.msg[0]);
  ER_CHECK_MSG (!strcmp (c.msg[1], "/data/RESP.IU.ANMO..BHZ: bad"), "got '%s'", c.msg[1]);
  ER_CHECK_MSG (!strcmp (c.msg[2], "note"), "got '%s'", c.msg[2]);
}

/* only a whole code counts as named, and an empty network is part of the name */
static void
test_log_context_partial_name (void)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_log_context file, channel, no_net_file, no_net;

  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  evalresp_log_context_init (&file, parent, "RESP.IU.ANMO..BHZE");
  evalresp_log_context_init (&channel, &file.logger, "IU.ANMO..BHZ");
  evalresp_log (&channel.logger, EV_WARN, 0, "careful");
  evalresp_log_context_init (&no_net_file, parent, "RESP..ANMO..BHZ");
  evalresp_log_context_init (&no_net, &no_net_file.logger, ".ANMO..BHZ");
  evalresp_log (&no_net.logger, EV_WARN, 0, "careful");

  ER_CHECK (c.n == 2);
  ER_CHECK_MSG (!strcmp (c.msg[0], "RESP.IU.ANMO..BHZE: IU.ANMO..BHZ: careful"), "got '%s'", c.msg[0]);
  ER_CHECK_MSG (!strcmp (c.msg[1], "RESP..ANMO..BHZ: careful"), "got '%s'", c.msg[1]);
}

/* a prefix longer than a message still yields a terminated message */
static void
test_log_context_long_prefix (void)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_log_context ctx;
  char longname[2 * MAX_LOG_MSG_LEN];

  memset (longname, 'x', sizeof (longname) - 1);
  longname[sizeof (longname) - 1] = '\0';
  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  evalresp_log_context_init (&ctx, parent, "%s", longname);
  evalresp_log (&ctx.logger, EV_WARN, 0, "careful");

  ER_CHECK (c.n == 1);
  ER_CHECK (strlen (c.msg[0]) < MAX_LOG_MSG_LEN);
  ER_CHECK (c.msg[0][0] == 'x');
}

/* a channel warning raised while reading a file names the file, and the channel
   when the file name does not */
static void
check_file_warning (const char *path, const char *between)
{
  capture c = {0};
  evalresp_logger parent[1];
  evalresp_channels *channels = NULL;
  size_t len = strlen (path), blen = strlen (between);
  int i, found = 0;

  ER_CHECK (!evalresp_logger_init (parent, capture_msg, &c));
  ER_CHECK (!evalresp_filename_to_channels (parent, path, NULL, NULL, &channels));
  for (i = 0; i < c.n; ++i)
  {
    if (c.level[i] == EV_WARN && strstr (c.msg[i], "Missing gain in stage 1"))
    {
      ER_CHECK_MSG (!strncmp (c.msg[i], path, len) && !strncmp (c.msg[i] + len, between, blen) &&
                        !strncmp (c.msg[i] + len + blen, "Missing gain", 12),
                    "got '%s'", c.msg[i]);
      found = 1;
    }
  }
  ER_CHECK (found);
  evalresp_free_channels (&channels);
}

static void
test_log_context_file_warning (void)
{
  check_file_warning (er_data_path ("RESP.IM.ATTU..BHE"), ": ");
}

static void
test_log_context_file_warning_other_name (void)
{
  /* the file is not named for its channel */
  ER_CHECK (!er_copy_file (er_data_path ("RESP.IM.ATTU..BHE"), "attu.resp"));
  check_file_warning ("attu.resp", ": IM.ATTU..BHE: ");
}

const er_test unit_log_context_tests[] = {
    {"levels", test_log_context_levels, NULL},
    {"nested", test_log_context_nested, NULL},
    {"not_repeated", test_log_context_not_repeated, NULL},
    {"partial_name", test_log_context_partial_name, NULL},
    {"long_prefix", test_log_context_long_prefix, NULL},
    {"file_warning", test_log_context_file_warning, NULL},
    {"file_warning_other_name", test_log_context_file_warning_other_name, NULL},
    {NULL, NULL, NULL},
};
