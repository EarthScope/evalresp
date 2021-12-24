/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements the interfaces for evalresp.
 *
 * @section history History
 *
 * Written by <a href="http://www.isti.com/">Instrumental Software
 * Technologies, Inc.</a> (ISTI).
 */

/**
 * @defgroup evalresp_private evalresp Private Interface
 * @ingroup evalresp
 * @brief Private evalresp interface.
 */

/**
 * @defgroup evalresp_private_alloc evalresp Private Memory Allocation Interface
 * @ingroup evalresp_private
 * @brief Private evalresp memory allocation interface.
 */

/**
 * @defgroup evalresp_private_calc evalresp Private Calculation Interface
 * @ingroup evalresp_private
 * @brief Private evalresp calculation interface.
 */

/**
 * @defgroup evalresp_private_error evalresp Private Error Handling Interface
 * @ingroup evalresp_private
 * @brief Private evalresp error handling interface.
 */

/**
 * @defgroup evalresp_private_file evalresp Private File Operations Interface
 * @ingroup evalresp_private
 * @brief Private evalresp file operations interface.
 */

/**
 * @defgroup evalresp_private_parse evalresp Private Parse Interface
 * @ingroup evalresp_private
 * @brief Private evalresp parse interface.
 */

/**
 * @defgroup evalresp_private_print evalresp Private Print Interface
 * @ingroup evalresp_private
 * @brief Private evalresp print interface.
 */

/**
 * @defgroup evalresp_private_resp evalresp Private Response Interface
 * @ingroup evalresp_private
 * @brief Private evalresp response interface.
 */

/**
 * @defgroup evalresp_private_string evalresp Private String Interface
 * @ingroup evalresp_private
 * @brief Private evalresp string interface.
 */

/**
 * @defgroup evalresp_private_spline evalresp Private Spline Interpolation Interface
 * @ingroup evalresp_private
 * @brief Private cubic spline interpolation interface for evalresp.
 */

/**
 * @file
 * @brief This file contains declarations and global structures for evalresp.
 */

#ifndef EVALRESP_PRIVATE_H
#define EVALRESP_PRIVATE_H

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include "evalresp/constants.h"
#include "evalresp/public_api.h"
#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

/* if Windows compiler then redefine 'complex' to */
/*  differentiate it from the existing struct,    */
/*  and rename 'strcasecmp' functions:            */
#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/**
 * @private
 * @ingroup evalresp_private
 * @brief Flag to query the value of the flag to use or not use the estimated
 *        delay in response computation.
 * @see use_estimated_delay()
 */
#define QUERY_DELAY -1

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIR normal tolerance.
 */
#define FIR_NORM_TOL 0.02

/**
 * @private
 * @ingroup evalresp_private
 * @brief Set flag to use the estimated delay in response computation.
 * @see use_estimated_delay()
 */
#define ESTIMATED_DELAY_FLAG 1

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of units encountered.
 * @note If default, then the response is just given in input units to output
 *       units, no interpretation is made of the units used).
 * @author 02/03/01: IGD: new unit pressure added.
 * @author 08/21/06: IGD: new unit TESLA added.
 * @author 10/03/13: IGD: new units CENTIGRADE added.
*/
enum units
{
  UNDEF_UNITS, /**< Undefined. */
  DIS,         /**< Displacement. */
  VEL,         /**< Velocity. */
  ACC,         /**< Acceleration. */
  COUNTS,      /**< Counts. */
  VOLTS,       /**< Volts. */
  DEFAULT,     /**< Default. */
  PRESSURE,    /**< Pressure. */
  TESLA,       /**< Tesla. */
  CENTIGRADE   /**< Degrees Celsius. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of filters encountered with corresponding SEED blockettes.
 */
enum filt_types
{
  UNDEF_FILT, /**< Undefined filter. */
  LAPLACE_PZ, /**< Laplace transform filter: poles and zeros representation B53 . */
  ANALOG_PZ,  /**< Analog filter: poles and zeros representation B53. */
  IIR_PZ,     /**< Infinite Impulse Response: polez and zeros representation B53. */
  FIR_SYM_1,  /**< Finite Impulse Response Filter (symmetrical with odd number of weights) B61. */
  FIR_SYM_2,  /**< Finie Impulse Response Filter (symmetrical with even number of weights) B61. */
  FIR_ASYM,   /**< Finite Impulse Response Filter (assymetrical) B54. */
  LIST,       /**< Filter presented as a list B55 (Frequency/amplitude). */
  GENERIC,    /**< Filter presented as a generi B56 (via Corener frequencies/slopes. */
  DECIMATION, /**< Decimation B57. */
  GAIN,       /**< Channel Sensitiity/Gain B58. */
  REFERENCE,  /**< Response Reference B60 to replace B53-58,61 with the dictionary counterparts. */
  FIR_COEFFS, /**< FIR response: coefficients representation B61. */
  IIR_COEFFS, /**< Infinite Impulse response represented in B54. */
  POLYNOMIAL  /**< Polynomial filter via B62. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of stages in RESP fi;es that are recognized.
 */
enum stage_types
{
  UNDEF_STAGE,     /**< Undefined stage. */
  PZ_TYPE,         /**< Polez and zeros stage. */
  IIR_TYPE,        /**< Infinite Impulse response stage. */
  FIR_TYPE,        /**< Finite Impulse response stage. */
  GAIN_TYPE,       /**< Channel Sensitivity/Gain stage. */
  LIST_TYPE,       /**< List response representation stage. */
  IIR_COEFFS_TYPE, /**< Infinite Impulse response in coefficient form stage. */
  GENERIC_TYPE,    /**< Generic response stage. */
  POLYNOMIAL_TYPE  /**< Polynomial type stage. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Network-station-locid-channel object.
 */
typedef struct
{
  char *station; /**< Station name. */
  char *network; /**< Network name. */
  char *locid;   /**< Location ID. */
  char *channel; /**< Channel name. */
  int found;     /**< Number of times found in the input RESP file. */
} evalresp_sncl;

/**
 * @private
 * @ingroup evalresp_private
 * @brief List of network-station-locid-channel objects.
 */
typedef struct evalresp_sncls_s
{
  int nscn;                /**< Number of network-station-locid-channel objects. */
  evalresp_sncl **scn_vec; /**< Array of network-station-locid-channel objects. */
} evalresp_sncls;

/**
 * @private
 * @ingroup evalresp_private
 * @brief A routine that frees up the space associated with a station-channel
 *        list type structure.
 * @param[in,out] sncls SNCL data.
 */
void evalresp_free_sncls (evalresp_sncls *sncls);

/* define structures for the compound data types used in evalesp */

/**
 * @private
 * @ingroup evalresp_private
 * @brief Array of string objects.
 */
struct string_array
{
  int nstrings;   /**< Number of strings. */
  char **strings; /**< String array. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief File list object.
 */
struct file_list
{
  char *name;                  /**< File name. */
  struct file_list *next_file; /**< Pointer to next file list object. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Matches files object.
 */
struct matched_files
{
  int nfiles;                     /**< Number of files. */
  struct file_list *first_list;   /**< Array of file list objects. */
  struct matched_files *ptr_next; /**< Pointer to next matches files object. */
};

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that returns a string describing the specified unit value.
 * @param[in] unit Unit value, one of evalresp_unit.
 * @returns pointer to string describing unit.
 */
const char *evalresp_unit_string (const evalresp_unit unit);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string can be converted into an
 *        integer using string_match().
 * @param[in] test String to test.
 * @param[in] log Logging structure.
 * @returns 0 if false.
 * @returns >0 if true.
 */
int is_int (const char *test, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string can be converted into an
 *        double using string_match().
 * @param[in] test String to test.
 * @param[in] log Logging structure.
 * @returns 0 if false.
 * @returns >0 if true.
*/
int is_real (const char *test, evalresp_logger *log);

/* routines used to create a list of files matching the users request */

/**
 * @private
 * @ingroup evalresp_private_file
 * @brief Creates a linked list of files to search based on the @p file name
 *        and @p scn_lst input arguments, i.e. based on the filename (if it
 *        is non-NULL) and the list of stations and channels.
 * @details If the filename exists as a directory, then that directory is
 *          searched for a list of files that look like
 *          'RESP.NETCODE.STA.CHA'. The names of any matching files will be
 *          placed in the linked list of file names to search for matching
 *          response information.  If no match is found for a requested
 *          'RESP.NETCODE.STA.CHA' file in that directory, then the search
 *          for a matching file will stop (see discussion of SEEDRESP case
 *          below).
 *
 *          If the filename is a file in the current directory, then a
 *          (matched_files *)NULL will be returned.
 *
 *          If the filename is NULL the current directory and the directory
 *          indicated by the environment variable 'SEEDRESP' will be searched
 *          for files of the form 'RESP.NETCODE.STA.CHA'. Files in the
 *          current directory will override matching files in the directory
 *          pointed to by 'SEEDRESP'. The routine will behave exactly as if
 *          the filenames contained in these two directories had been
 *          specified.
 *
 *          The mode is set to zero if the user named a specific filename and
 *          to one if the user named a directory containing RESP files (or if
 *          the SEEDRESP environment variable was used to find RESP files.
 *
 *          If a pattern cannot be found, then a value of NULL is set for the
 *          'names' pointer of the linked list element representing that
 *          station-channel-network pattern.
 * @param[in] file File name.
 * @param[in] scn_lst List of network-station-locid-channel objects.
 * @param[out] mode 1 for directory search or 0 if file was specified.
 * @param[in] log Logging structure.
 * @returns Pointer to the head of the linked list of matches files.
 * @returns @c NULL if no files were found that match request.
 */
struct matched_files *find_files (char *file, evalresp_sncls *scn_lst,
                                  int *mode, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_file
 * @brief Get filenames matching the expression in @p in_file.
 * @param[in] in_file File name matching expression.
 * @param[out] file Pointer to the head of the linked list of matches files.
 * @param[in] log Logging structure.
 * @returns Number of files found matching the expression.
 */
int get_names (char *in_file, struct matched_files *file, evalresp_logger *log);

/* routines used to allocate vectors of the basic data types used in the
 filter stages */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of complex numbers.
 * @param[in] npts Number of complex numbers to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
evalresp_complex *alloc_complex (int npts, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of responses.
 * @details A 'response' is a combination of a complex array, a
 *          station-channel-network, and a pointer to the next 'response' in
 *          the list.
 * @param[in] npts Number of responses to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
evalresp_response *alloc_response (int npts, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of strings.
 * @param[in] nstrings Number of strings to allocate in array.
 * @param[in] log Logging structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p nstrings is zero.
 * @warning Exits with error if allocation fails.
 */
struct string_array *alloc_string_array (int nstrings, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a station-channel structure
 * @returns Pointer to allocated structure.
 * @param[in] log Logging structure.
 * @warning Exits with error if allocation fails.
 */
evalresp_sncl *alloc_scn (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of station/channel pairs.
 * @param[in] nscn Number of station/channel pairs to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p nscn is zero.
 * @warning Exits with error if allocation fails.
 */
evalresp_sncls *alloc_scn_list (int nscn, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an element of a linked list of filenames.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct file_list *alloc_file_list (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an element of a linked list of matching files.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct matched_files *alloc_matched_files (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of double precision numbers
 * @param[in] npts Number of double precision numbers to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
double *alloc_double (int npts, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of characters.
 * @param[in] len Number of characters to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p len is zero.
 * @warning Exits with error if allocation fails.
 */
char *alloc_char (int len, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of char pointers.
 * @param[in] len Number of char pointers to allocate in array.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p len is zero.
 * @warning Exits with error if allocation fails.
 */
char **alloc_char_ptr (int len, evalresp_logger *log);

/* allocation routines for the various types of filters */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a pole-zero type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note The space for the complex poles and zeros is not allocated here, the
 *       space for these vectors must be allocated as they are read, since the
 *       number of poles and zeros is unknown until the blockette is partially
 *       parsed.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_pz (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a coefficients-type filter.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note See alloc_pz() for details (like alloc_pz(), this does not allocate
 *       space for the numerators and denominators, that is left until
 *       parse_fir()).
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_coeff (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a fir-type filter.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note See alloc_pz() for details (like alloc_pz(), this does not allocate
 *       space for the numerators and denominators, that is left until
 *       parse_fir()).
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_fir (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a response reference type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_ref (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a gain type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note The space for the calibration vectors is not allocated here, the
 *       space for these vectors must be allocated as they are read, since the
 *       number of calibration points is unknown until the blockette is
 *       partially parsed.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_gain (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a list type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note The space for the amplitude, phase and frequency vectors is not
 *       allocated here the user must allocate space for these parameters once
 *       the number of frequencies is known.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_list (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a generic type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @note The space for the corner_freq, and corner_slope vectors is not
 *       allocated here the user must allocate space for these parameters once
 *       the number of frequencies is known.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_generic (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a decimation type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
evalresp_blkt *alloc_deci (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a polynomial Blockette 62.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 * @author 05/31/2013: IGD.
 */
evalresp_blkt *alloc_polynomial (evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a decimation type filter structure.
 * @param[in] log Logging structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
evalresp_stage *alloc_stage (evalresp_logger *log);

/* routines to free up space associated with dynamically allocated
 structure members */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a string list type
 *        structure.
 * @param[in,out] lst String list type structure.
 */
void free_string_array (struct string_array *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a station-channel
 *        type structure.
 * @param[in,out] ptr Station-channel type structure.
 */
void free_scn (evalresp_sncl *ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a matched files
 *        type structure.
 * @param[in,out] lst Matched files type structure.
 */
void free_matched_files (struct matched_files *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a file list type
 *        structure.
 * @param[in,out] lst Matched files type structure.
 */
void free_file_list (struct file_list *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a pole-zero type
 *        filter.
 * @param[in,out] blkt_ptr Pole-zero type filter blockette structure.
 */
void free_pz (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a coefficients
 *        type filter.
 * @param[in,out] blkt_ptr Coefficients type filter blockette structure.
 */
void free_coeff (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a fir type filter.
 * @param[in,out] blkt_ptr Fir type filter blockette structure.
 */
void free_fir (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a list type
 *        filter.
 * @param[in,out] blkt_ptr List type filter blockette structure.
 */
void free_list (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a generic type
 *        filter.
 * @param[in,out] blkt_ptr Generic type filter blockette structure.
 */
void free_generic (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a gain type
 *        filter.
 * @param[in,out] blkt_ptr Gain type filter blockette structure.
 */
void free_gain (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a decimation type
 *        filter.
 * @param[in,out] blkt_ptr Decimation type filter blockette structure.
 */
void free_deci (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a response
 *        reference type filter.
 * @param[in,out] blkt_ptr Response reference type filter blockette structure.
 */
void free_ref (evalresp_blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a stages in a
 *        channel's response.
 * @param[in,out] stage_ptr Stage structure.
 */
void free_stages (evalresp_stage *stage_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
f * @brief A routine that frees up the space associated with a channel's filter
 *        sequence.
 * @param[in,out] chan_ptr Channel structure.
 */
void free_channel (evalresp_channel *chan_ptr);

/* simple error handling routines to standardize the output error values and
 allow for control to return to 'evresp' if a recoverable error occurs */

/* a simple routine that parses the station information from the input file */

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Add a null character to the end of a string.
 * @details @p where is a character that specifies where the null character
 *          should be placed, the possible values are:
 *
 *          - @c a - remove extra spaces from end of string, then adds null
 *                   character;
 *
 *          - @c e - adds null character to end of character string.
 *
 * @param[in,out] s String.
 * @param[in] len Length of string.
 * @param[in] where Where to place null character, @c a or @c e.
 * @returns New length of string.
 * @remark Practical to use with input FORTRAN strings.
 */
int add_null (char *s, int len, char where);

/* run a sanity check on the channel's filter sequence */

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief A routine that merges two fir filters.
 * @details The coefficients from the second filter are copied into the first
 *          filter and the number of coefficients in the first filter is
 *          adjusted to reflect the new filter size. Then the next_blkt
 *          pointer for the first filter is reset to the value of the
 *          next_blkt pointer of the second filter and the space associated
 *          with the second filter is free'd. Finally, the second filter
 *          pointer is reset to the next_blkt pointer value of the first
 *          filter
 * @param[in,out] first_blkt First filter.
 * @param[in,out] second_blkt Second filter.
 * @param[in] log Logging structure.
 */
//void merge_coeffs (evalresp_blkt *first_blkt, evalresp_blkt **second_blkt, evalresp_log_t *log);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief A routine that merges two lists filters (blockettes 55).
 * @details The frequencies, amplitudes and phases from the second filter are
 *          copied into the first filter and the number of coefficients in the
 *          first filter is adjusted to reflect the new filter size. Then the
 *          next_blkt pointer for the first filter is reset to the value of
 *          the next_blkt pointer of the second filter and the space
 *          associated with the second filter is free'd. Finally, the second
 *          filter pointer is reset to the next_blkt pointer value of the
 *          first filter.
 * @param[in,out] first_blkt First filter.
 * @param[in,out] second_blkt Second filter.
 * @param[in] log Logging structure.
 * @author 07/07/00: Ilya Dricker IGD (i.dricker@isti.com): Modified from
 *         merge_coeffs() for 3.2.17 of evalresp.
 */
//void merge_lists (evalresp_blkt *first_blkt, evalresp_blkt **second_blkt, evalresp_log_t *log);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief A routine that checks a channel's filter stages.
 * @details (1) Run a sanity check on the filter sequence. And that LAPLACE_PZ
 *              and ANALOG_PZ filters will be followed by a GAIN blockette
 *              only. REFERENCE blockettes are ignored (since they contain no
 *              response information).
 *
 *          (2) As the routine moves through the stages in the filter
 *              sequence, several other checks are made. First, the output
 *              units of this stage are compared with the input units of the
 *              next on to ensure that no stages have been skipped. Second,
 *              the filter type of this blockette is compared with the filter
 *              type of the next. If they are the same and have the same
 *              stage-sequence number, then they are merged to form one
 *              filter.  At the present time this is only implemented for the
 *              FIR type filters (since those are the only filter types that
 *              typically are continued to a second filter blockette). The new
 *              filter will have the combined number of coefficients and a new
 *              vector containing the coefficients for both filters, one after
 *              the other.
 *
 *          (3) the expected delay from the FIR filter stages in the channel's
 *              filter sequence is calculated and stored in the filter
 *              structure.
 * @param[in] log Logging structure.
 * @param[in] chan Channel structure.
 * @retval EVALRESP_OK on success
 */
int check_channel (evalresp_logger *log, evalresp_channel *chan);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief Checks to see if a FIR filter can be converted to a symmetric FIR
 *        filter.
 * @details If so, the conversion is made and the filter type is redefined.
 * @param[in,out] f FIR filter.
 * @param[in] chan Channel structure.
 * @param[in] log Logging structure.
 */
void check_sym (evalresp_blkt *f, evalresp_channel *chan, evalresp_logger *log);

/* routines used to calculate the instrument responses */
/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief Calculate response.
 * @param[in] log Logging structure.
 * @param[in] chan Channel structure.
 * @param[in] freq Frequency array.
 * @param[in] nfreqs Number if numbers in @p freq.
 * @param[in] output Output.
 */
int calculate_response (evalresp_logger *log, evalresp_options *options, evalresp_channel *chan, double *freq, int nfreqs, evalresp_complex *output);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief Normalize response.
 * @param[in] log Logging structure.
 * @param[in] options object to control the flow of the conversion to responses
 * @param[in,out] chan Channel structure.
 */
int normalize_response (evalresp_logger *log, evalresp_options const *const options, evalresp_channel *chan);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string looks like a time string
 *        using string_match().
 * @details Time strings must be in the format 'hh:mm:ss[.#####]', so more
 *          than 14 characters is an error (too many digits).
 * @param[in] test String to test.
 * @param[in] log Logging structure.
 * @returns 0 if false.
 * @returns >0 if true.
 */
int is_time (const char *test, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Compare two times and determine if the first is greater, equal to,
 *        or less than the second.
 * @param[in] dt1 Date-time 1.
 * @param[in] dt2 Date-time 2.
 * @returns An integer indicating whether the time in the input argument
 *          @p dt1 is greater than (1), equal to (0), or less than (-1) the
 *          time in the input argument @p dt2.
 */
int timecmp (evalresp_datetime *dt1, evalresp_datetime *dt2);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief Interpolates amplitude and phase values from the set of frequencies
 *        in the List blockette to the requested set of frequencies.
 * @details The given frequency, amplitude and phase arrays are deallocated
 *          and replaced with newly allocated arrays containing the
 *          interpolated values. The @p p_number_points value is also updated.
 * @param[in,out] frequency_ptr Reference to array of frequency values.
 * @param[in,out]  amplitude_ptr Reference to array of amplitude values.
 * @param[in,out] phase_ptr Reference to array of phase values.
 * @param[in,out] p_number_points Reference to number of points value.
 * @param[in] req_freq_arr Array of requested frequency values.
 * @param[in] eq_num_freqs Number values in @p req_freq_arr array.
 * @param[in] log Logging structure.
 */
int interpolate_list_blockette (double **frequency_ptr,
                                double **amplitude_ptr, double **phase_ptr,
                                int *p_number_points, double *req_freq_arr,
                                int req_num_freqs, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief Phase unwrapping function.
 * @details It works only inside a loop over phases.
 * @param[in] phase Phase value to process.
 * @param[in] prev_phase Previous phase.
 * @param[in] range Range value to use.
 * @param[in,out] added_value Pointer to offset value used for each call.
 * @returns "Unwrapped" version of the given phase value.
 * @author 04/05/04: IGD.
 */
double unwrap_phase (double phase, double prev_phase, double range,
                     double *added_value);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief Wraps a set of phase values so that all of the values are between
 *        "-range" and "+range" (inclusive).
 * @details This function is called iteratively, once for each phase value.
 * @param[in] phase Phase value to process.
 * @param[in] range Range value to use.
 * @param[in,out] added_value Pointer to offset value used for each call.
 * @returns "Wrapped" version of the given phase value.
 */
double wrap_phase (double phase, double range, double *added_value);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] name label to use if needing to log error
 * @param[in] str string to convert
 * @param[out] value the result of the conversion
 * @brief convert string to integar value
 * @retval EVALRESP_OK on success
 */
int
parse_int (evalresp_logger *log, const char *name, const char *str, int *value);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] name label to use if needing to log error
 * @param[in] str string to convert
 * @param[out] value the result of the conversion
 * @brief convert string to double value
 * @retval EVALRESP_OK on success
 */
int
parse_double (evalresp_logger *log, const char *name, const char *str, double *value);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] name label to use if needing to log error
 * @param[in] n number of elements to allocate
 * @param[out] array the returned array
 * @brief allocate a array of doubles with length n
 * @retval EVALRESP_OK on success
 */
int
calloc_doubles (evalresp_logger *log, const char *name, int n, double **array);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] options object to control the flow of the conversion to responses
 * @param[in] filter object on how to filter the inputed files
 * @param[out] responses object pointer containing responses
 * @brief take information from file and converthem into responses
 * @retval EVALRESP_OK on success
 */
int process_cwd (evalresp_logger *log, evalresp_options *options,
                 evalresp_filter *filter, evalresp_responses **responses);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] options object to control the flow of the conversion to responses
 * @param[in] filter object on how to filter the inputed files
 * @param[out] responses object pointer containing responses
 * @brief take information form stdin and convert them to responses
 * @retval EVALRESP_OK on success
 */
int process_stdio (evalresp_logger *log, evalresp_options *options,
                   evalresp_filter *filter, evalresp_responses **responses);

/**
 * @private
 * @ingroup evalresp_private
 * @param[in] log logging structure
 * @param[in] responses evalresp_responses object to be printed out
 * @param[in] format evalresp_output_format that determines what files are outputed
 * @param[in] use_stdio flag to determine if printing to stdio instead of to files
 * @brief create files in the cwd (or stdio) based on the output formats selected
 * @post files created in the current working directory
 * @retval EVALRESP_OK on success
 */
int responses_to_cwd (evalresp_logger *log, const evalresp_responses *responses,
                      int unwrap, evalresp_output_format format, int use_stdio);

int                                     /* O - Number of bytes formatted */
_evalresp_snprintf (char *buffer,       /* I - Output buffer */
                    size_t bufsize,     /* I - Size of output buffer */
                    const char *format, /* I - Printf-style format string */
                    ...);               /* I - Additional arguments as needed */

#endif
