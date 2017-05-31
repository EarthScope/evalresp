/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements a private X2R interface for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_private_x2r evalresp Private X2R Interface
 * @ingroup evalresp_private
 * @brief Private X2R interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        X2R.
 */

#ifndef X2R_H
#define X2R_H

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_OK 0

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_USER 1

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_MEMORY 2

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_BUFFER 3

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_XML 4

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_IO 5

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief FIXME.
 */
#define X2R_ERR_DATE 6

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif
