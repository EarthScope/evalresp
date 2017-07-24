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
 * @defgroup evalresp_private_x2r evalresp Private XML-to-RSEED Interface
 * @ingroup evalresp_private
 * @brief Private XML-to-RSEED interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        XML-to-RSEED.
 */

#ifndef X2R_H
#define X2R_H

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code OK.
 */
#define X2R_OK 0

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for user error.
 */
#define X2R_ERR_USER 1

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for memory error.
 */
#define X2R_ERR_MEMORY 2

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for insufficient buffer space (internal error).
 */
#define X2R_ERR_BUFFER 3

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for error in XML.
 */
#define X2R_ERR_XML 4

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for IO error.
 */
#define X2R_ERR_IO 5

/**
 * @private
 * @ingroup evalresp_private_x2r
 * @brief Status code for date parsing error.
 */
#define X2R_ERR_DATE 6

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#endif
