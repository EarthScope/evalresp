#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "./log_context.h"

/* Errors and warnings are forwarded to the parent with the prefix, unless
 * the prefix is repeated from an enclosing context; other levels are
 * forwarded unchanged. */
static int
log_with_prefix (evalresp_log_msg *msg, void *data)
{
  evalresp_log_context *ctx = data;

  if (!ctx->repeated && (msg->log_level == EV_ERROR || msg->log_level == EV_WARN))
  {
    return evalresp_log (ctx->parent, msg->log_level, msg->verbosity_level,
                         "%s: %s", ctx->prefix, msg->msg);
  }
  return evalresp_log (ctx->parent, msg->log_level, msg->verbosity_level,
                       "%s", msg->msg);
}

/* Whether name occurs in text as a whole code: not within a longer run of
 * letters or digits. */
static int
names (const char *text, const char *name)
{
  size_t n = strlen (name);
  const char *p;

  if (!n)
    return 0;
  for (p = strstr (text, name); p; p = strstr (p + 1, name))
  {
    int starts = !isalnum ((unsigned char)name[0]) || p == text || !isalnum ((unsigned char)p[-1]);
    int ends = !isalnum ((unsigned char)name[n - 1]) || !isalnum ((unsigned char)p[n]);

    if (starts && ends)
      return 1;
  }
  return 0;
}

/* Whether a context enclosing parent has a prefix that names prefix. */
static int
enclosed_by (const evalresp_logger *parent, const char *prefix)
{
  while (parent && parent->log_func == log_with_prefix)
  {
    const evalresp_log_context *outer = parent->func_data;

    if (names (outer->prefix, prefix))
      return 1;
    parent = outer->parent;
  }
  return 0;
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
  ctx->repeated = enclosed_by (parent, ctx->prefix);
  evalresp_logger_init (&ctx->logger, log_with_prefix, ctx);
}
