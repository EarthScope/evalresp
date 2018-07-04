
#ifndef evalresp_public_low_level_response_H
#define evalresp_public_low_level_response_H

#include "evalresp/constants.h"
#include "evalresp/public_channels.h" // TODO - needed for complex?  needs separate public common?

/**
 * @public
 * @ingroup evalresp_public_low_level_response
 * @brief Response object.
 */
typedef struct evalresp_response_s
{
  char station[STALEN];             /**< Station name. */
  char network[NETLEN];             /**< Network name. */
  char locid[LOCIDLEN];             /**< Location ID. */
  char channel[CHALEN];             /**< Channel name. */
  evalresp_complex *rvec;           /**< Output vector. */
  int nfreqs;                       /**< Number of frequencies. */
  double *freqs;                    /**< Array of frequencies. */
  struct evalresp_response_s *next; /**< Pointer to next response object (unused in new API, required for compatibility layer). */
} evalresp_response;

/**
 * @public
 * @ingroup evalresp_public_low_level_response
 * @brief A collection of response objects.
 */
typedef struct
{
  int nresponses;
  evalresp_response **responses;
} evalresp_responses;

/**
 * @public
 * @ingroup evalresp_public_low_level_response
 * @brief A routine that frees up the space associated with a single response
 * structure.
 * @param[in,out] resp_ptr Response structure.
 */
void evalresp_free_response (evalresp_response **response);

/**
 * @public
 * @ingroup evalresp_public_low_level_response
 * @brief A routine that frees up the space associated with a collection of
 *        responses.
 * @param[in,out] resp_ptr Response structure.
 */
void evalresp_free_responses (evalresp_responses **responses);

#endif
