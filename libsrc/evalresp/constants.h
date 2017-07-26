
#ifndef EVALRESP_CONSTANTS_H
#define EVALRESP_CONSTANTS_H

/**
 * @private
 * @ingroup evalresp_private
 * @brief True.
 */
#ifndef TRUE
#define TRUE 1
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief False.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of station name.

 */
#ifndef STALEN
#define STALEN 64
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of network name.
 */
#ifndef NETLEN
#define NETLEN 64
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of location ID.

 */
#ifndef LOCIDLEN
#define LOCIDLEN 64
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of channel name.
 */
#ifndef CHALEN
#define CHALEN 64
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of output string.
 */
#ifndef OUTPUTLEN
#define OUTPUTLEN 256
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of temporary string.
 */
#ifndef TMPSTRLEN
#define TMPSTRLEN 64
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of unit name strings.
 */
#ifndef UNITS_STR_LEN
#define UNITS_STR_LEN 16
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of date-time string.
 */
#ifndef DATIMLEN
#define DATIMLEN 23
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Blockette string length.
 */
#ifndef BLKTSTRLEN
#define BLKTSTRLEN 4
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Field string length.
 */
#ifndef FLDSTRLEN
#define FLDSTRLEN 3
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum field string length.
 */
#ifndef MAXFLDLEN
#define MAXFLDLEN 50
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum line string length.
 */
#ifndef MAXLINELEN
#define MAXLINELEN 256
#endif

#ifndef DATIMESIZE
#define DATIMESIZE 32
#endif
#ifndef TODAYSIZE
#define TODAYSIZE 10
#endif

#endif
