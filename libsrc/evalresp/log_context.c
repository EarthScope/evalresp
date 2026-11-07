#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "./log_context.h"

/* Errors and warnings are forwarded to the parent with the prefix;
 * other levels are forwarded unchanged. */
static int
log_with_prefix (evalresp_log_msg *msg, void *data)
{
  evalresp_log_context *ctx = data;

  if (msg->log_level == EV_ERROR || msg->log_level == EV_WARN)
  {
    return evalresp_log (ctx->parent, msg->log_level, msg->verbosity_level,
                         "%s: %s", ctx->prefix, msg->msg);
  }
  return evalresp_log (ctx->parent, msg->log_level, msg->verbosity_level,
                       "%s", msg->msg);
}

void
evalresp_log_context_init (evalresp_log_context *ctx, evalresp_logger *parent,
                           const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vsnprintf (ctx->prefix, sizeof (ctx->prefix), fmt, ap);
  va_end (ap);

  ctx->parent = parent;
  evalresp_logger_init (&ctx->logger, log_with_prefix, ctx);
}
