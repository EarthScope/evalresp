
#include <evalresp/private.h>

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of error codes possible.
 */
enum error_codes
{
  NON_EXIST_FLD = -2,         /**< Field does not exist. */
  ILLEGAL_RESP_FORMAT = -5,   /**< Illegal response format. */
  PARSE_ERROR = -4,           /**< Parse error. */
  UNDEF_PREFIX = -3,          /**< Undefined prefix. */
  UNDEF_SEPSTR = -6,          /**< Undefined separator. */
  OUT_OF_MEMORY = -1,         /**< Out of memory */
  UNRECOG_FILTYPE = -7,       /**< Unrecognized filter type */
  UNEXPECTED_EOF = -8,        /**< Unexpected EOF. */
  ARRAY_BOUNDS_EXCEEDED = -9, /**< Array bound exceeded. */
  OPEN_FILE_ERROR = 2,        /**< Failure to open the file. */
  RE_COMP_FAILED = 3,         /**< Failure to compile the pattern. */
  MERGE_ERROR = 4,            /**< lure to merge blockettes. */
  SWAP_FAILED = 5,            /**< Swapping failure: not used. */
  USAGE_ERROR = 6,            /**< User errors detencted on teh command line. */
  BAD_OUT_UNITS = 7,          /**< Bad output units error. */
  IMPROP_DATA_TYPE = -10,     /**< Unsupported data type. */
  UNSUPPORT_FILTYPE = -11,    /**< Unsupported filter type. */
  ILLEGAL_FILT_SPEC = -12,    /**< Illigal filter specs. */
  NO_STAGE_MATCHED = -13,     /**< No stage matched error. */
  UNRECOG_UNITS = -14         /**< Unrecognized units. */
};

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Finds the location of a response for the input station-channel
 *        at the specified date.
 * @details If no response is available in the file pointed to by @p fptr,
 *          then a -1 value is returned (indicating failure), otherwise a
 *          value of 1 is returned (indicating success). The matching beg_t,
 *          end_t and station info are returned as part of the input
 *          structure @p this_channel. The pointer to the file (@p fptr) is
 *          left in position for the parse_channel() routine to grab the
 *          response information for that station.
 * @param[in,out] fptr FILE pointer.
 * @param[in] scn Network-station-locid-channel object.
 * @param[in] datime Date-time string.
 * @param[out] this_channel Channel structure.
 * @param[in] log Logging structure.
 * @returns 1 on success.
 * @returns -1 on failure.
 * @note The station information is preloaded into @p this_channel, so the
 *       file pointer does not need to be repositioned to allow for this
 *       information to be reread.
 */
int get_resp (FILE *fptr, evalresp_sncl *scn, char *datime,
              evalresp_channel *this_channel, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Retrieves the info from the RESP file blockettes for a channel.
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. Only the
 *          station/channel/date info is returned. The file pointer is left at
 *          the end of the date info, where it can be used to read the
 *          response information into the filter structures.
 * @param[in,out] fptr File pointer.
 * @param[out] chan Channel structure.
 * @param[in] log Logging structure.
 * @returns 1 on success.
 * @returns 0 on failure.
 */
int get_channel (FILE *fptr, evalresp_channel *chan, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Finds the location of the start of the next response in the file
 *        and positions the file pointer there.
 * @details If no more responses are available then a zero value is returned
 *          (indicating failure to reposition the file pointer), otherwise a
 *          that first line is returned in the global variable FirstLine. The
 *          pointer to the file (fptr) is left in position for the
 *          get_channel() routine to grab the channel information.
 * @param[in,out] fptr FILE pointer.
 * @param[in] log Logging structure.
 * @returns 1 on success.
 * @returns 0 on failure.
 */
int next_resp (FILE *fptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for COEFF filters (in a Blockette
 *        [54] or [44]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 */
void parse_coeff (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for COEFF filters (in a Blockette
 *        [54] or [44]).
 * @details Handles the case of a digital IIR filtering. Derived from
 *          parse_coeff(). Errors cause the program to terminate. The
 *          blockette and field numbers are checked as the file is parsed. As
 *          with parse_pz(), for this routine to work,  the lines must contain
 *          evalresp-3.0 style prefixes.
 * @param[in,out] fptr FILE pointer, evalresp_log_t *log.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 * @author 06/27/00: I.Dricker (i.dricker@isti.com) for 2.3.17 iir.
 */
void parse_iir_coeff (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for list filters (in a Blockette
 *        [55] or [45]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 * @author 06/21/00: Ilya Dricker ISTI: I modify this routine to accomodate
 *         the form of the parsed blockette 55 generated by SeismiQuery.
 *         Since currently the blockette 55 is not supported, we do not
 *         anticipate problems caused by this change.
 */
void parse_list (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file for generic filters (in a Blockette [56] or
 *        [46]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 */
void parse_generic (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file for decimation filters (in a Blockette [57] or
 *        [47]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in] log Logging structure.
 * @returns Sequence number of the stage for verification.
 */
int parse_deci (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for gain filters (in a Blockette
 *        [58] or [48]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in] log Logging structure.
 * @returns Sequence number of the stage for verification.
 */
int parse_gain (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for FIR filters (in a Blockette [61]
 *        or [41]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 */
void parse_fir (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for Response Reference type filters
 *        (in a Blockette [60]).
 * @details Errors cause the program to terminate.  The blockette and field
 *          numbers are checked as the file is parsed. As with parse_pz(), for
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 */
void parse_ref (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for polynomial filters (in a
 *        Blockette [62] or [42]).
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. The lines must contain
 *          evalresp-3.0 style prefixes
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 * @author 05/31/2013: IGD.
 */
void parse_polynomial (FILE *fptr, evalresp_blkt *blkt_ptr,
                       evalresp_stage *stage_ptr, evalresp_logger *log);
