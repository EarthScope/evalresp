/**
 * @file
 * @brief This file contains declarations and global structures for evalresp
 *        regular expressions.
 */

/**
 * @private
 * @ingroup evalresp_private_regexp
 * @brief The first byte of the regexp internal "program" is actually this
 *        magic number; the start node begins in the second byte.
 */
#define MAGIC 0234
