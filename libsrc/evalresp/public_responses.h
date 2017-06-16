
#ifndef EVALRESP_PUBLIC_RESPONSE_H
#define EVALRESP_PUBLIC_RESPONSE_H

#include "./ugly.h"
#include "./public_channels.h"  // TODO - needed for complex?  needs separate public common?

/**
 * @private
 * @ingroup evalresp_private
 * @brief Response object.
 */
typedef struct evalresp_response_s
{
  char station[STALEN];                        /**< Station name. */
  char network[NETLEN];                        /**< Network name. */
  char locid[LOCIDLEN];                        /**< Location ID. */
  char channel[CHALEN];                        /**< Channel name. */
  evalresp_complex *rvec;                      /**< Output vector. */
  int nfreqs; /**< Number of frequencies. */   /* Add by I.Dricker IGD to support blockette 55 */
  double *freqs; /**< Array of frequencies. */ /* Add by I.Dricker IGD to support blockette 55 */
  struct evalresp_response_s *next;                       /**< Pointer to next response object. */
} evalresp_response;

typedef struct
{
  int nresponses;
  evalresp_response **responses;
} evalresp_responses;

#endif
