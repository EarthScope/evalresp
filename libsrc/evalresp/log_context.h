#ifndef __EVALRESP_LOG_CONTEXT_H__
#define __EVALRESP_LOG_CONTEXT_H__

#include "evalresp_log/log.h"

/**
 * @private
 * @ingroup evalresp_private
 * @brief A logger that prefixes errors and warnings with a context string
 *        before passing them to a parent logger.
 * @details Pass &ctx.logger to the code that runs within the context. The
 *          structure must not be moved after initialisation.
 */
typedef struct
{
  evalresp_logger logger;  /**< the logger to pass on */
  evalresp_logger *parent; /**< where messages go; NULL for the default output */
  char prefix[MAX_LOG_MSG_LEN];
} evalresp_log_context;

/**
 * @private
 * @ingroup evalresp_private
 * @brief Initialise a logging context.
 * @param[out] ctx the context
 * @param[in] parent logger that receives the prefixed messages; may be NULL
 * @param[in] fmt printf-style format of the prefix
 */
void evalresp_log_context_init (evalresp_log_context *ctx, evalresp_logger *parent,
                                const char *fmt, ...);

#endif
