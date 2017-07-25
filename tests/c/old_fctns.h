#include "evalresp/private.h"
#include "evalresp/public_api.h"

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define a pointer to a channel structure to use in determining the
 *        input and output units if using "default" units and for use in error
 *        output.
 */
extern struct evalresp_channel_s *GblChanPtr;
extern char myLabel[20];

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Compares an input string (string) with a regular expression or
 *        glob-style "pattern" (expr) using the re_comp() and re_exec()
 *        functions (from stdlib.h).
 * @details - First, if the type-flag is set to the string "-g", the
 *            glob-style 'expr' is changed so that any '*' characters are
 *            converted to '.*' combinations and and '?' characters are
 *            converted to '.' characters.
 *
 *          - If the type-flag is set to "-r" then no conversions are
 *            necessary (the string is merely copied to the new location).
 *
 *          - Finally, the 'regexp_pattern' argument is passed through the
 *            re_comp() routine (compiling the pattern), and the value of
 *            re_exec(string) is returned to the calling function.
 *
 * @param[in] string Input string.
 * @param[in] expr Regularion expression or glob-style pattern expression.
 * @param[in] type_flag Type flag, @c -g or @c -r.
 * @param[in] log Logging structure.
 * @returns 0 if false.
 * @returns >0 if true.
 */
int string_match (const char *string, char *expr, char *type_flag, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Counts the number of white space delimited fields on a given input
 *        line.
 * @param[in] line Input line string.
 * @returns Number of white space delimited fields.
 */
int count_fields (char *line);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Counts the number of fields delimited by @p delim on a given input
 *        line.
 * @note In this routine an empty string has one field in it... with null
 *       length).
 * @param[in] line Input line string.
 * @param[in] delim Delimiter.
 * @returns Number of fields delimated by @p delim.
 */
int count_delim_fields (char *line, char *delim);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns a field from the input line.
 * @param[in] log Logging structure.
 * @param[in] line Input line.
 * @param[in] fld_no Field number.
 * @param[out] return_field Return field.
 * @returns Length of the resulting field if successful.
 * @note Exits with error if no field exists with that number.
 */
int parse_field (char *line, int fld_no, char *return_field, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns a field from the input line, delimited by @p delim.
 * @param[in] line Input line.
 * @param[in] fld_no Field number.
 * @param[in] delim Delimiter.
 * @param[out] return_field Return field.
 * @param[in] log Logging structure.
 * @returns Length of the resulting field if successful.
 * @note Exits with error if no field exists with that number.
 */
int parse_delim_field (char *line, int fld_no, char *delim, char *return_field, evalresp_logger *log);

/* utility routines that are used to parse the input file line by line and
 convert the input to what the user wants */

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Parses the fields on a line into separate strings.
 * @details The definition of a field is a sequence of any non-white space
 *          characters with bordering white space. The result is a structure
 *          containing the number of fields on the line and an array of
 *          character strings (which are easier to deal with than the original
 *          line).
 * @param[in] line String to parse.
 * @param[in] log Logging structure.
 * @returns Array of string objects.
 */
struct string_array *ev_parse_line (char *line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Parses the fields on a line into separate strings using delimeter.
 * @details The definition of a field is a sequence of any non-delimeter
 *          characters with bordering delimiter. The result is a structure
 *          containing the number of fields on the line and an array of
 *          character strings (which are easier to deal with than the original
 *          line).
 * @param[in] line String to parse.
 * @param[in] delim Delimiter string.
 * @param[in] log Logging structure.
 * @returns Array of string objects.
 */
struct string_array *parse_delim_line (char *line, char *delim, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the indicated field from the next 'non-comment' line from a
 *        RESP file using blockette number and field number.
 * @details Exits with error if no non-comment lines left in file or if
 *          expected blockette and field numbers do not match those found in
 *          the next non-comment line.
 * @param[in,out] fptr FILE pointer.
 * @param[out] return_field Return field.
 * @param[in] blkt_no Blockette number.
 * @param[in] fld_no Field number.
 * @param[in] sep Field separator.
 * @param[in] fld_wanted Field wanted.
 * @param[in] log Logging structure.
 * @returns Length of the resulting field if successful.
 * @note Here a field is any string of non-white characters surrounded by
 *       white space.
 */
int get_field (FILE *fptr, char *return_field, int blkt_no, int fld_no,
               char *sep, int fld_wanted, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the indicated field from the next 'non-comment' line from a
 *        RESP file.
 * @details Returns with a value of zero if no non-comment lines left in file
 *          or if expected blockette and field numbers do not match those
 *          found in the next non-comment line.
 * @param[in,out] fptr FILE pointer.
 * @param[out] return_field Return field.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Field number.
 * @param[in] sep Field separator.
 * @param[in] fld_wanted Field wanted.
 * @param[in] log Logging structure.
 * @returns Length of the resulting field if successful.
 * @returns Value of zero otherwise.
 * @note Here a field is any string of non-white characters surrounded by
 *       white space.
 */
int test_field (FILE *fptr, char *return_field, int *blkt_no, int *fld_no,
                char *sep, int fld_wanted, evalresp_logger *log);
/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the next 'non-comment' line from a RESP file using blockette
 *        number and field number.
 * @details Exits with error if no non-comment lines left in file or if
 *          expected blockette and field numbers do not match those found in
 *          the next non-comment line.
 * @param[in,out] fptr FILE pointer.
 * @param[out] return_line Return line.
 * @param[in] blkt_no Blockette number.
 * @param[in] fld_no Field number.
 * @param[in] sep Separator.
 * @param[in] log Logging structure.
 * @returns length of the resulting line if successful
 * @note Exits with error if no non-comment lines left in file or if expected
 *       blockette and field numbers do not match those found in the next
 *       non-comment line.
 * @author 2004.079: SBH: Added code to skip over valid lines that we didn't
 *                        expect. Support for SHAPE formatte RESP files, and
 *                        to skip blank lines.
 */
int get_line (FILE *fptr, char *return_line, int blkt_no, int fld_no, char *sep, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the next 'non-comment' line from a RESP file.
 * @param[in,out] fptr FILE pointer.
 * @param[out] return_line Return line.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Field number.
 * @param[in] sep Separator.
 * @param[in] log Logging structure.
 * @returns The fld_no of the resulting line if successful.
 * @returns 0 if no non-comment lines left in file, regardless of the
 *          blockette and field numbers for the line (these values are
 *          returned as the values of the input pointer variables @p fld_no
 *          and @p blkt_no).
 * @author 2004.079: SBH: Added code to skip blank lines.
 */
int next_line (FILE *fptr, char *return_line, int *blkt_no, int *fld_no, char *sep, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the blockette and field numbers in the prefix of the next
 *        'non-comment' line from a RESP file.
 * @param[in,out] fptr FILE pointer.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Field number.
 * @param[out] in_line Line string.
 * @param[in] log Logging structure.
 * @returns 1 if a non-comment field is found.
 * @returns @c NULL if no non-comment line is found.
 * @author 2004.079: SBH: Added code to skip blank lines.
 */
int check_line (FILE *fptr, int *blkt_no, int *fld_no, char *in_line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Uses get_fld() to return the integer value of the input string.
 * @details If the requested field is not a proper representation of a number,
 *          then 'IMPROPER DATA TYPE' error is signaled.
 * @param[in] in_line Input string.
 * @param[in] log Logging structure.
 * @returns Integer value on success.
 */
int get_int (char *in_line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Uses get_fld() to return the double-precision value of the input
 *        string.
 * @details If the requested field is not a proper representation of a number,
 *          then 'IMPROPER DATA TYPE' error is signaled.
 * @param[in] in_line Input string.
 * @param[in] log Logging structure.
 * @returns Double value on success.
 */
double get_double (char *in_line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Checks an incoming line for keys that indicate the units represented
 *        by a filter.
 * @details If the units are not recognized, an 'UNDEFINED UNITS' error
 *          condition is signaled. If the user specified that 'default' unit
 *          should be used, then the line is simply saved as the value of
 *          'SEEDUNITS[DEFAULT]' and no check of the units is made.
 * @param[in] channel Current channel.
 * @param[in] line Incoming line.
 * @param[in] log Logging structure.
 * @see units
 */
int check_units (evalresp_channel *channel, char *line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Used to check out if we are using a FIR or IIR coefficients in
 *        blockette 54.
 * @details This information is contained in the 10th field of this blockette
 *          (number of denominators); if the number of denominators is 0: we
 *          got FIR. Otherwise, it is IIR. is_IIR_coeff() reads the text
 *          response file; finds the proper line and decides if the response is
 *          IIR or FIR. The text file is then fseek() to the original position
 *          and the function returns.
 * @param[in,out] fp FILE pointer.
 * @param[in] position Position.
 * @returns 1 if it is IIR.
 * @returns 0 if it is not IIR.
 * @returns 0 in case of error.
 * @author 07/00: IGD I.Dricker ISTI i.dricker@isti.com for evalresp 3.2.17.
 */
int is_IIR_coeffs (FILE *fp, int position);

/* routines used to load a channel's response information into a linked
 list of filter stages, each containing a linked list of blockettes */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Finds the location of a response for one of the input
 *        station-channels at the specified date.
 * @details If no response is available in the file pointed to by @p fptr,
 *          then a -1 value is returned (indicating failure), otherwise the
 *          index of the matching station-channel pair. The matching beg_t,
 *          end_t and station info are returned as part of the input structure
 *          @p this_channel. The pointer to the file (@p fptr) is left in
 *          position for the parse_channel() routine to grab the response
 *          information for that station.
 * @param[in,out] fptr FILE pointer.
 * @param[in] scn_lst List of network-station-locid-channel objects.
 * @param[in] datime Date-time string.
 * @param[out] this_channel Channel structure.
 * @param[in] log Logging structure.
 * @returns Index of the matching station-channel pair.
 * @returns -1 on failure.
 * @note The station information is preloaded into @p this_channel, so the
 *       file pointer does not need to be repositioned to allow for this
 *       information to be reread.
 */
int find_resp (FILE *fptr, evalresp_sncls *scn_lst, char *datime,
               evalresp_channel *this_channel, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses the RESP file blockettes for a channel.
 * @details Errors cause the program to terminate. The blockette and field
 *          numbers are checked as the file is parsed. As with the filter
 *          blockettes, the actual positions of the fields on a line will not
 *          effect this routine. This routine constructs a linked list of
 *          filters until a non-filter blockette (or the EOF) is found. For
 *          this routine to work, the lines must contain evalresp-3.0 style
 *          prefixes. Also, it is assumed that the file pointer has been
 *          prepositioned at the end of the station/channel/time information
 *          in the RESP file for the channel of interest, (i.e. that the next
 *          non-comment line starts the actual response information).
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] chan Channel structure.
 * @param[in] log Logging structure.
 * @returns First field number.
 */
int parse_channel (FILE *fptr, evalresp_channel *chan, evalresp_logger *log);

/* parsing routines for various types of filters */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Used to parse the prefix from a non-comment RESP file line.
 * @details Returns the blockette number and starting field number for that
 *          line as arguments, return values are -1 for comment lines (if they
 *          sneak through), 1 for lines that start with 'B' and zero for lines
 *          that are not comments and don't start with a 'B'.
 * @param[in] log Logging structure.
 * @param[in] line Non-comment RESP file line.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Starting field number.
 * @returns 1 on success.
 * @returns 0 on failure.
 */
//int read_pref (evalresp_log_t *log, char * const line, int *blkt_no, int *fld_no);
int parse_pref (int *blkt_no, int *fld_no, char *line, evalresp_logger *log);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Parses RESP file poles & zeros (in a Blockette [53] or [43]).
 * @details Errors cause program termination. The blockette and field numbers
 *          are checked as the file is parsed. The field-format of the RESP
 *          file is assumed to be fixed (i.e. the same number of fields per
 *          line in the same order), but the actual positions of those fields
 *          on the line will not effect the parsing routines. For this routine
 *          to work, the lines must contain evalresp-3.0 style prefixes.
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @param[in] log Logging structure.
 */
void parse_pz (FILE *fptr, evalresp_blkt *blkt_ptr, evalresp_stage *stage_ptr, evalresp_logger *log);
