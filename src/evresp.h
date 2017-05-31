/**
 * @mainpage Introduction
 *
 * @section purpose Purpose
 *
 * Implements the private interface for evalresp.
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
 * @file
 * @brief This file contains declarations and global structures for evalresp.
 */

/*==================================================================
 *                    evresp.h
 *=================================================================*/

/*
 8/28/2001 -- [ET]  Added 'WIN32' directives for Windows compiler
 compatibility; added 'extern' to variable
 declarations at end of file.
 8/2/2001 -- [ET]  Version 3.2.28:  Modified to allow command-line
 parameters for frequency values to be missing
 (default values used).
 10/21/2005 -- [ET]  Version 3.2.29:  Implemented interpolation of
 amplitude/phase values from responses containing
 List blockettes (55); modified message shown when
 List blockette encountered; modified so as not to
 require characters after 'units' specifiers like
 "M" and "COUNTS"; modified to handle case where
 file contains "B052F03 Location:" and nothing
 after it on line; added warnings for unrecognized
 parameters; fixed issue where program would crash
 under Linux/UNIX ("segmentation fault") if input
 response file contained Windows-style "CR/LF"
 line ends; renamed 'strncasecmp()' function
 to 'strnicmp()' when compiling under Windows.
 11/3/2005 -- [ET]  Version 3.2.30:  Modified to unwrap phase values
 before interpolation and re-wrap after (if needed);
 modified to detect when all requested interpolated
 frequencies are out of range.
 1/18/2006 -- [ET]  Version 3.2.31:  Renamed 'regexp' functions to
 prevent name clashes with other libraries.
 2/13/2006 -- [ET]  Version 3.2.32:  Moved 'use_delay()' function from
 'evalresp.c' to 'evresp.c'; modified to close input
 file when a single response file is specified.
 3/27/2006 -- [ID]  Version 3.2.33:  Added include_HEADERS target
 "evr_spline.h" to "Makefile.am"; upgraded "missing"
 script.
 3/28/2006 -- [ET]  Version 3.2.34:  Added 'free()' calls to "evresp.c"
 to fix memory leaks when program functions used
 in external applications.
 4/4/2006 -- [ET]  Version 3.2.35:  Modified to support channel-IDs with
 location codes equal to an empty string.
 8/21/2006 -- [IGD] Version 3.2.36:  Added support for TESLA units.
 10/16/2006 -- [ET]  Version 3.2.37:  Added two more 'free()' calls to
 "evresp.c" to fix memory leaks when program functions
 are used in external applications.
 8/11/2006 -- [IGD] Libtoolized evresp library
 4/03/2007 -- [IGD] Added myLabel global variable which is used to add NSLC
 labels in evalresp logging if --enable-log-label config
 option is used
 5/14/2010 -- [ET]  Version 3.3.3:  Added "#define strcasecmp stricmp"
 if Windows.
 5/31/2013 -- [IGD] Version 3.3.4: Adding polynomial filter
 10/02/2013 -- [IGD] Version 3.3.4: Adding DEGREE CENTIGRADE as units
 */

#ifndef EVRESP_H
#define EVRESP_H

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>

/* if Windows compiler then redefine 'complex' to */
/*  differentiate it from the existing struct,    */
/*  and rename 'strcasecmp' functions:            */
#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/* IGD 10/16/04 This is for Windows which does not use Makefile.am */
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "4.0.6"
#endif

// once this is defined (v4.0.6 onwards), some functions take an additional
// axml flag argument
#define EVRESP_XML

/**
 * @private
 * @ingroup evalresp_private
 * @brief True.
 */
#define TRUE 1

/**
 * @private
 * @ingroup evalresp_private
 * @brief False.
 */
#define FALSE 0

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
 * @brief Maximum length of station name.

 */
#define STALEN 64

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of network name.
 */
#define NETLEN 64

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of location ID.

 */
#define LOCIDLEN 64

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of channel name.
 */
#define CHALEN 64

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of output string.
 */
#define OUTPUTLEN 256

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of temporary string.
 */
#define TMPSTRLEN 64

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of unit name strings.
 */
#define UNITS_STR_LEN 16

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum length of date-time string.
 */
#define DATIMLEN 23


/**
 * @private
 * @ingroup evalresp_private
 * @brief Blockette string length.
 */
#define BLKTSTRLEN 4

/**
 * @private
 * @ingroup evalresp_private
 * @brief Field string length.
 */
#define FLDSTRLEN 3

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum field string length.
 */
#define MAXFLDLEN 50

/**
 * @private
 * @ingroup evalresp_private
 * @brief Maximum line string length.
 */
#define MAXLINELEN 256

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
enum units {
    UNDEF_UNITS,  /**< Undefined. */
    DIS,  /**< Displacement. */
    VEL,  /**< Velocity. */
    ACC,  /**< Acceleration. */
    COUNTS,  /**< Counts. */
    VOLTS,  /**< Volts. */
    DEFAULT,  /**< Default. */
    PRESSURE,  /**< Pressure. */
    TESLA,  /**< Tesla. */
    CENTIGRADE  /**< Degrees Celsius. */
};


/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of filters encountered with corresponding SEED blockettes.
 */
enum filt_types {
    UNDEF_FILT,  /**< Undefined filter. */
    LAPLACE_PZ,  /**< Laplace transform filter: poles and zeros representation B53 . */
    ANALOG_PZ,  /**< Analog filter: poles and zeros representation B53. */
    IIR_PZ,  /**< Infinite Impulse Response: polez and zeros representation B53. */
    FIR_SYM_1,  /**< Finite Impulse Response Filter (symmetrical with odd number of weights) B61. */
    FIR_SYM_2,  /**< Finie Impulse Response Filter (symmetrical with even number of weights) B61. */
    FIR_ASYM,  /**< Finite Impulse Response Filter (assymetrical) B54. */
    LIST,  /**< Filter presented as a list B55 (Frequency/amplitude). */
    GENERIC,  /**< Filter presented as a generi B56 (via Corener frequencies/slopes. */
    DECIMATION,  /**< Decimation B57. */
    GAIN,  /**< Channel Sensitiity/Gain B58. */
    REFERENCE,  /**< Response Reference B60 to replace B53-58,61 with the dictionary counterparts. */
    FIR_COEFFS,  /**< FIR response: coefficients representation B61. */
    IIR_COEFFS,  /**< Infinite Impulse response represented in B54. */
    POLYNOMIAL  /**< Polynomial filter via B62. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of stages in RESP fi;es that are recognized.
 */
enum stage_types {
    UNDEF_STAGE,  /**< Undefined stage. */
    PZ_TYPE,  /**< Polez and zeros stage. */
    IIR_TYPE,  /**< Infinite Impulse response stage. */
    FIR_TYPE,  /**< Finite Impulse response stage. */
    GAIN_TYPE,  /**< Channel Sensitivity/Gain stage. */
    LIST_TYPE,  /**< List response representation stage. */
    IIR_COEFFS_TYPE,  /**< Infinite Impulse response in coefficient form stage. */
    GENERIC_TYPE,  /**< Generic response stage. */
    POLYNOMIAL_TYPE  /**< Polynomial type stage. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Enumeration representing the types of error codes possible.
 */
enum error_codes {
    NON_EXIST_FLD = -2,  /**< Field does not exist. */
    ILLEGAL_RESP_FORMAT = -5,  /**< Illegal response format. */
    PARSE_ERROR = -4,  /**< Parse error. */
    UNDEF_PREFIX = -3,  /**< Undefined prefix. */
    UNDEF_SEPSTR = -6,  /**< Undefined separator. */
    OUT_OF_MEMORY = -1,  /**< Out of memory */
    UNRECOG_FILTYPE = -7,  /**< Unrecognized filter type */
    UNEXPECTED_EOF = -8,  /**< Unexpected EOF. */
    ARRAY_BOUNDS_EXCEEDED = -9,  /**< Array bound exceeded. */
    OPEN_FILE_ERROR = 2,  /**< Failure to open the file. */
    RE_COMP_FAILED = 3,  /**< Failure to compile the pattern. */
    MERGE_ERROR = 4,  /**< lure to merge blockettes. */
    SWAP_FAILED = 5,  /**< Swapping failure: not used. */
    USAGE_ERROR = 6,  /**< User errors detencted on teh command line. */
    BAD_OUT_UNITS = 7,  /**< Bad output units error. */
    IMPROP_DATA_TYPE = -10,  /**< Unsupported data type. */
    UNSUPPORT_FILTYPE = -11,  /**< Unsupported filter type. */
    ILLEGAL_FILT_SPEC = -12,  /**< Illigal filter specs. */
    NO_STAGE_MATCHED = -13,  /**< No stage matched error. */
    UNRECOG_UNITS = -14  /**< Unrecognized units. */
};


/* define structures for the compound data types used in evalesp */

/**
 * @private
 * @ingroup evalresp_private
 * @brief Complex data type.
 */
struct evr_complex {
    double real;  /**< Real part. */
    double imag;  /**< Imaginary part. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Array of string objects.
 */
struct string_array {
    int nstrings;  /**< Number of strings. */
    char **strings;  /**< String array. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Network-station-locid-channel object.
 */
struct scn {
    char *station;  /**< Station name. */
    char *network;  /**< Network name. */
    char *locid;  /**< Location ID. */
    char *channel;  /**< Channel name. */
    int found;  /**< Flag (true/false) if found in the input RESP files. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Response object.
 */
struct response {
    char station[STALEN];  /**< Station name. */
    char network[NETLEN];  /**< Network name. */
    char locid[LOCIDLEN];  /**< Location ID. */
    char channel[CHALEN];  /**< Channel name. */
    struct evr_complex *rvec;  /**< Output vector. */
    int nfreqs;   /**< Number of frequencies. */ /* Add by I.Dricker IGD to support blockette 55 */
    double *freqs;  /**< Array of frequencies. */ /* Add by I.Dricker IGD to support blockette 55 */
    struct response *next;  /**< Pointer to next response object. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief File list object.
 */
struct file_list {
    char *name;  /**< File name. */
    struct file_list *next_file;  /**< Pointer to next file list object. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Matches files object.
 */
struct matched_files {
    int nfiles;  /**< Number of files. */
    struct file_list *first_list;  /**< Array of file list objects. */
    struct matched_files *ptr_next;  /**< Pointer to next matches files object. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief List of network-station-locid-channel objects.
 */
struct scn_list {
    int nscn;  /**< Number of network-station-locid-channel objects. */
    struct scn **scn_vec;  /**< Array of network-station-locid-channel objects. */
};

/* define structures for the various types of filters defined in seed */

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Response (Poles & Zeros) blockette.
 */
struct pole_zeroType {
    int nzeros;  /**< Number of zeros (blockettes [43] or [53]). */
    int npoles;  /**< Number of poles. */
    double a0;  /**< FIXME. */
    double a0_freq;  /**< FIXME. */
    struct evr_complex *zeros;  /**< Array of zeros (complex). */
    struct evr_complex *poles;  /**< Array of poles (complex). */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Response (Coefficients) blockette.
 */
struct coeffType {
    int nnumer;  /**< FIXME. (blockettes [44] or [54]) */
    int ndenom;  /**< FIXME. */
    double *numer;  /**< FIXME. */
    double *denom;  /**< FIXME. */
    double h0;  /**< FIXME. */  /* IGD this field is new v 3.2.17 */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Response (Coefficients) blockette.
 */
struct polynomialType {
    unsigned char approximation_type;  /**< FIXME. (blockettes [42] or [62]) */  /* IGD 05/31/2013 */
    unsigned char frequency_units;  /**< FIXME. */
    double lower_freq_bound;  /**< FIXME. */
    double upper_freq_bound;  /**< FIXME. */
    double lower_approx_bound;  /**< FIXME. */
    double upper_approx_bound;  /**< FIXME. */
    double max_abs_error;  /**< FIXME. */
    int ncoeffs;  /**< FIXME. */
    double *coeffs;  /**< FIXME. */
    double *coeffs_err;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A FIR Response blockette.
 */
struct firType {
    int ncoeffs;  /**< Number of coefficients (blockettes [41] or [61]). */
    double *coeffs;  /**< Array of coefficients. */
    double h0;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Response (List) blockette.
 */
struct listType {
    int nresp;  /**< Number of responses (blockettes [45] or [55]). */
    double *freq;  /**< Array of freqencies. */
    double *amp;  /**< Array of amplitudes. */
    double *phase;  /**< Array of phases. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Generic Response blockette.
 */
struct genericType {
    int ncorners;  /**< FIXME. (blockettes [46] or [56]) */
    double *corner_freq;  /**< FIXME. */
    double *corner_slope;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Decimation blockette.
 */
struct decimationType {
    double sample_int;  /**< FIXME. (blockettes [47] or [57]) */
    int deci_fact;  /**< FIXME. */
    int deci_offset;  /**< FIXME. */
    double estim_delay;  /**< FIXME. */
    double applied_corr;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Channel Sensitivity/Gain blockette.
 */
struct gainType {
    double gain;  /**< FIXME. (blockettes [48] or [58]) */
    double gain_freq;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief A Response Reference blockette.
 */
struct referType {
    int num_stages;  /**< FIXME. */
    int stage_num;  /**< FIXME. */
    int num_responses;  /**< FIXME. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define a blkt as a stucture containing the blockette type, a union
 *        (blkt_info) containing the blockette info, and a pointer to the
 *        next blockette in the filter sequence.
 * @details The structures will be assembled to form a linked list of
 *          blockettes that make up a filter, with the last blockette
 *          containing a '(struct blkt *)NULL' pointer in the 'next_blkt'
 *          position.
 */
struct blkt {
    int type;  /**< Blockette type. */
    union {
        struct pole_zeroType pole_zero;  /**< FIXME. */
        struct coeffType coeff;  /**< FIXME. */
        struct firType fir;  /**< FIXME. */
        struct listType list;  /**< FIXME. */
        struct genericType generic;  /**< FIXME. */
        struct decimationType decimation;  /**< FIXME. */
        struct gainType gain;  /**< FIXME. */
        struct referType reference;  /**< FIXME. */
        struct polynomialType polynomial;  /**< FIXME. */
    } blkt_info;  /**< Blockette info. */
    struct blkt *next_blkt;  /**< Pointer to next blockette. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define a stage as a structure that contains the sequence number, the
 *        input and output units, a pointer to the first blockette of the
 *        filter, and a pointer to the next stage in the response.
 * @details Again, the last stage in the response will be indicated by a
 *          '(struct stage *)NULL pointer in the 'next_stage' position.
 */
struct stage {
    int sequence_no;  /**< Sequence number. */
    int input_units;  /**< Input units. */
    int output_units;  /**< Output units. */
    struct blkt *first_blkt;  /**< Pointer to first blockette of the filter. */
    struct stage *next_stage;  /**< Pointer to the next stage in the response. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief And define a channel as a structure containing a pointer to the head
 *        of a linked list of stages.
 * @details Will access the pieces one stages at a time in the same order that
 *          they were read from the input file, so a linked list is the
 *          easiest way to do this (since we don't have to worry about the
 *          time penalty inherent in following the chain). As an example, if
 *          the first stage read was a pole-zero stage, then the parts of that
 *          stage contained in a channel structure called "ch" would be
 *          accessed as:

@verbatim
struct stage *stage_ptr;
struct blkt *blkt_ptr;

stage_ptr = ch.first_stage;
blkt_ptr = stage_ptr->first_blkt;
if(blkt_ptr->type == LAPLACE_PZ || blkt_ptr->type == ANALOG_PZ ||
blkt_ptr->type == IIR_PZ){
nzeros = blkt_ptr->blkt_info.poles_zeros.nzeros;
........
}
@endverbatim

 */
struct channel {
    char staname[STALEN];  /**< FIXME. */
    char network[NETLEN];  /**< FIXME. */
    char locid[LOCIDLEN];  /**< FIXME. */
    char chaname[CHALEN];  /**< FIXME. */
    char beg_t[DATIMLEN];  /**< FIXME. */
    char end_t[DATIMLEN];  /**< FIXME. */
    char first_units[MAXLINELEN];  /**< FIXME. */
    char last_units[MAXLINELEN];  /**< FIXME. */
    double sensit;  /**< FIXME. */
    double sensfreq;  /**< FIXME. */
    double calc_sensit;  /**< FIXME. */
    double calc_delay;  /**< FIXME. */
    double estim_delay;  /**< FIXME. */
    double applied_corr;  /**< FIXME. */
    double sint;  /**< FIXME. */
    int nstages;  /**< FIXME. */
    struct stage *first_stage;  /**< Pointer to the head of a linked list of
                                   stage. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief Structure used for time comparisons.
 */
struct dateTime {
    int year;  /**< Year. */
    int jday;  /**< Day of year. */
    int hour;  /**< Hour. */
    int min;  /**< Minutes. */
    float sec;  /**< Seconds. */
};

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 * @author 2007/02/27: IGD.
 */
extern char myLabel[20];
//char myLabel[20] = "aa";

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
 * @returns Array of string objects.
 */
struct string_array *ev_parse_line(char *line);

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
 * @returns Array of string objects.
 */
struct string_array *parse_delim_line(char *line, char *delim);

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
 * @returns Length of the resulting field if successful.
 * @note Here a field is any string of non-white characters surrounded by
 *       white space.
 */
int get_field(FILE *fptr, char *return_field, int blkt_no, int fld_no,
              char *sep, int fld_wanted);

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
 * @returns Length of the resulting field if successful.
 * @returns Value of zero otherwise.
 * @note Here a field is any string of non-white characters surrounded by
 *       white space.
 */
int test_field(FILE *fptr, char *return_field, int *blkt_no, int *fld_no,
               char *sep, int fld_wanted);
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
 * @returns length of the resulting line if successful
 * @note Exits with error if no non-comment lines left in file or if expected
 *       blockette and field numbers do not match those found in the next
 *       non-comment line.
 * @author 2004.079: SBH: Added code to skip over valid lines that we didn't
 *                        expect. Support for SHAPE formatte RESP files, and
 *                        to skip blank lines.
 */
int get_line(FILE *fptr, char *return_line, int blkt_no, int fld_no, char *sep);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the next 'non-comment' line from a RESP file.
 * @param[in,out] fptr FILE pointer.
 * @param[out] return_line Return line.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Field number.
 * @param[in] sep Separator.
 * @returns The fld_no of the resulting line if successful.
 * @returns 0 if no non-comment lines left in file, regardless of the
 *          blockette and field numbers for the line (these values are
 *          returned as the values of the input pointer variables @p fld_no
 *          and @p blkt_no).
 * @author 2004.079: SBH: Added code to skip blank lines.
 */
int next_line(FILE *fptr, char *return_line, int *blkt_no, int *fld_no, char *sep);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Counts the number of white space delimited fields on a given input
 *        line.
 * @param[in] line Input line string.
 * @returns Number of white space delimited fields.
 */
int count_fields(char *line);

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
int count_delim_fields(char *line, char *delim);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns a field from the input line.
 * @param[in] line Input line.
 * @param[in] fld_no Field number.
 * @param[out] return_field Return field.
 * @returns Length of the resulting field if successful.
 * @note Exits with error if no field exists with that number.
 */
int parse_field(char *line, int fld_no, char *return_field);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns a field from the input line, delimited by @p delim.
 * @param[in] line Input line.
 * @param[in] fld_no Field number.
 * @param[in] delim Delimiter.
 * @param[out] return_field Return field.
 * @returns Length of the resulting field if successful.
 * @note Exits with error if no field exists with that number.
 */
int parse_delim_field(char *line, int fld_no, char *delim, char *return_field);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Returns the blockette and field numbers in the prefix of the next
 *        'non-comment' line from a RESP file.
 * @param[in,out] fptr FILE pointer.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Field number.
 * @param[out] in_line Line string.
 * @returns 1 if a non-comment field is found.
 * @returns @c NULL if no non-comment line is found.
 * @author 2004.079: SBH: Added code to skip blank lines.
 */
int check_line(FILE *fptr, int *blkt_no, int *fld_no, char *in_line);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Uses get_fld() to return the integer value of the input string.
 * @details If the requested field is not a proper representation of a number,
 *          then 'IMPROPER DATA TYPE' error is signaled.
 * @param[in] in_line Input string.
 * @returns Integer value on success.
 */
int get_int(char *in_line);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Uses get_fld() to return the double-precision value of the input
 *        string.
 * @details If the requested field is not a proper representation of a number,
 *          then 'IMPROPER DATA TYPE' error is signaled.
 * @param[in] in_line Input string.
 * @returns Double value on success.
 */
double get_double(char *in_line);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief Checks an incoming line for keys that indicate the units represented
 *        by a filter.
 * @details If the units are not recognized, an 'UNDEFINED UNITS' error
 *          condition is signaled. If the user specified that 'default' unit
 *          should be used, then the line is simply saved as the value of
 *          'SEEDUNITS[DEFAULT]' and no check of the units is made.
 * @param[in] line Incoming line.
 * @see units
 */
int check_units(char *line);

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
 * @returns 0 if false.
 * @returns >0 if true.
 */
int string_match(const char *string, char *expr, char *type_flag);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string can be converted into an
 *        integer using string_match().
 * @param[in] test String to test.
 * @returns 0 if false.
 * @returns >0 if true.
 */
int is_int(const char *test);

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string can be converted into an
 *        double using string_match().
 * @param[in] test String to test.
 * @returns 0 if false.
 * @returns >0 if true.
*/
int is_real(const char *test);

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
int is_IIR_coeffs(FILE *fp, int position);

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
 * @returns Index of the matching station-channel pair.
 * @returns -1 on failure.
 * @note The station information is preloaded into @p this_channel, so the
 *       file pointer does not need to be repositioned to allow for this
 *       information to be reread.
 */
int find_resp(FILE *fptr, struct scn_list *scn_lst, char *datime,
              struct channel *this_channel);

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
 * @returns 1 on success.
 * @returns -1 on failure.
 * @note The station information is preloaded into @p this_channel, so the
 *       file pointer does not need to be repositioned to allow for this
 *       information to be reread.
 */
int get_resp(FILE *fptr, struct scn *scn, char *datime,
             struct channel *this_channel);

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
 * @returns 1 on success.
 * @returns 0 on failure.
 */
int get_channel(FILE *fptr, struct channel* chan);

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
 * @returns 1 on success.
 * @returns 0 on failure.
 */
int next_resp(FILE *fptr);

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
 * @returns Pointer to the head of the linked list of matches files.
 * @returns @c NULL if no files were found that match request.
 */
struct matched_files *find_files(char *file, struct scn_list *scn_lst,
                                 int *mode);

/**
 * @private
 * @ingroup evalresp_private_file
 * @brief Get filenames matching the expression in @p in_file.
 * @param[in] in_file File name matching expression.
 * @param[out] file Pointer to the head of the linked list of matches files.
 * @returns Number of files found matching the expression.
 */
int get_names(char *in_file, struct matched_files *file);

/* routines used to allocate vectors of the basic data types used in the
 filter stages */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of complex numbers.
 * @param[in] npts Number of complex numbers to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
struct evr_complex *alloc_complex(int npts);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of responses.
 * @details A 'response' is a combination of a complex array, a
 *          station-channel-network, and a pointer to the next 'response' in
 *          the list.
 * @param[in] npts Number of responses to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
struct response *alloc_response(int npts);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of strings.
 * @param[in] nstring Number of strings to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p nstrings is zero.
 * @warning Exits with error if allocation fails.
 */
struct string_array *alloc_string_array(int nstrings);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a station-channel structure
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct scn *alloc_scn(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of station/channel pairs.
 * @param[in] nscn Number of station/channel pairs to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p nscn is zero.
 * @warning Exits with error if allocation fails.
 */
struct scn_list *alloc_scn_list(int nscn);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an element of a linked list of filenames.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct file_list *alloc_file_list(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an element of a linked list of matching files.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct matched_files *alloc_matched_files(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of double precision numbers
 * @param[in] npts Number of double precision numbers to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p npts is zero.
 * @warning Exits with error if allocation fails.
 */
double *alloc_double(int npts);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of characters.
 * @param[in] len Number of characters to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p len is zero.
 * @warning Exits with error if allocation fails.
 */
char *alloc_char(int len);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for an array of char pointers.
 * @param[in] len Number of char pointers to allocate in array.
 * @returns Pointer to allocated array.
 * @returns @c NULL if @p len is zero.
 * @warning Exits with error if allocation fails.
 */
char **alloc_char_ptr(int len);

/* allocation routines for the various types of filters */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a pole-zero type filter structure.
 * @returns Pointer to allocated structure.
 * @note The space for the complex poles and zeros is not allocated here, the
 *       space for these vectors must be allocated as they are read, since the
 *       number of poles and zeros is unknown until the blockette is partially
 *       parsed.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_pz(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a coefficients-type filter.
 * @returns Pointer to allocated structure.
 * @note See alloc_pz() for details (like alloc_pz(), this does not allocate
 *       space for the numerators and denominators, that is left until
 *       parse_fir()).
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_coeff(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a fir-type filter.
 * @returns Pointer to allocated structure.
 * @note See alloc_pz() for details (like alloc_pz(), this does not allocate
 *       space for the numerators and denominators, that is left until
 *       parse_fir()).
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_fir(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a response reference type filter structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_ref(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a gain type filter structure.
 * @returns Pointer to allocated structure.
 * @note The space for the calibration vectors is not allocated here, the
 *       space for these vectors must be allocated as they are read, since the
 *       number of calibration points is unknown until the blockette is
 *       partially parsed.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_gain(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a list type filter structure.
 * @returns Pointer to allocated structure.
 * @note The space for the amplitude, phase and frequency vectors is not
 *       allocated here the user must allocate space for these parameters once
 *       the number of frequencies is known.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_list(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a generic type filter structure.
 * @returns Pointer to allocated structure.
 * @note The space for the corner_freq, and corner_slope vectors is not
 *       allocated here the user must allocate space for these parameters once
 *       the number of frequencies is known.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_generic(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a decimation type filter structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct blkt *alloc_deci(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a polynomial Blockette 62.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 * @author 05/31/2013: IGD.
 */
struct blkt *alloc_polynomial(void);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief Allocates space for a decimation type filter structure.
 * @returns Pointer to allocated structure.
 * @warning Exits with error if allocation fails.
 */
struct stage *alloc_stage(void);

/* routines to free up space associated with dynamically allocated
 structure members */

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a string list type
 *        structure.
 * @param[in,out] lst String list type structure.
 */
void free_string_array(struct string_array *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a station-channel
 *        type structure.
 * @param[in,out] ptr Station-channel type structure.
 */
void free_scn(struct scn *);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a station-channel
 *        list type structure.
 * @param[in,out] lst Station-channel list type structure.
 */
void free_scn_list(struct scn_list *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a matched files
 *        type structure.
 * @param[in,out] lst Matched files type structure.
 */
void free_matched_files(struct matched_files *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a file list type
 *        structure.
 * @param[in,out] lst Matched files type structure.
 */
void free_file_list(struct file_list *lst);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a pole-zero type
 *        filter.
 * @param[in,out] blkt_ptr Pole-zero type filter blockette structure.
 */
void free_pz(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a coefficients
 *        type filter.
 * @param[in,out] blkt_ptr Coefficients type filter blockette structure.
 */
void free_coeff(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a fir type filter.
 * @param[in,out] blkt_ptr Fir type filter blockette structure.
 */
void free_fir(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a list type
 *        filter.
 * @param[in,out] blkt_ptr List type filter blockette structure.
 */
void free_list(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a generic type
 *        filter.
 * @param[in,out] blkt_ptr Generic type filter blockette structure.
 */
void free_generic(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a gain type
 *        filter.
 * @param[in,out] blkt_ptr Gain type filter blockette structure.
 */
void free_gain(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a decimation type
 *        filter.
 * @param[in,out] blkt_ptr Decimation type filter blockette structure.
 */
void free_deci(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a response
 *        reference type filter.
 * @param[in,out] blkt_ptr Response reference type filter blockette structure.
 */
void free_ref(struct blkt *blkt_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a stages in a
 *        channel's response.
 * @param[in,out] stage_ptr Stage structure.
 */
void free_stages(struct stage *stage_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
f * @brief A routine that frees up the space associated with a channel's filter
 *        sequence.
 * @param[in,out] chan_ptr Channel structure.
 */
void free_channel(struct channel *chan_ptr);

/**
 * @private
 * @ingroup evalresp_private_alloc
 * @brief A routine that frees up the space associated with a linked list of
 *        response information.
 * @param[in,out] chan_ptr Response structure.
 */
void free_response(struct response *resp_ptr);

/* simple error handling routines to standardize the output error values and
 allow for control to return to 'evresp' if a recoverable error occurs */

/**
 * @private
 * @ingroup evalresp_private_error
 * @brief Prints a user supplied error message to stderr and exits with the
 *        user supplied error condition.
 * @param[in] cond Exit status for exit().
 * @param[in] msg Message format string.
 * @param[in] ... Arguments to format string.
 * @warning Do not use in library calls.
 */
void error_exit(int cond, char *msg, ...);

/**
 * @private
 * @ingroup evalresp_private_error
 * @brief Prints a user supplied error message to stderr and returns control
 *        to the calling routine at the point that that routine calls
 *        'setjmp(jump_buffer)'.
 * @param[in] cond Return value from setjmp()
 * @param[in] msg Message format string.
 * @param[in] ... Arguments to format string.
 */
void error_return(int cond, char *msg, ...);

/* a simple routine that parses the station information from the input file */

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
 * @returns First field number.
 */
int parse_channel(FILE *fptr, struct channel* chan);

/* parsing routines for various types of filters */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Used to parse the prefix from a non-comment RESP file line.
 * @details Returns the blockette number and starting field number for that
 *          line as arguments, return values are -1 for comment lines (if they
 *          sneak through), 1 for lines that start with 'B' and zero for lines
 *          that are not comments and don't start with a 'B'.
 * @param[out] blkt_no Blockette number.
 * @param[out] fld_no Starting field number.
 * @param[in] line Non-comment RESP file line.
 * @returns 1 on success.
 * @returns 0 on failure.
 */
int parse_pref(int *blkt_no, int *fld_no, char *line);

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
 */
void parse_pz(FILE *fptr, struct blkt *blkt_ptr, struct stage *stage_ptr);

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
 */
void parse_coeff(FILE *fptr, struct blkt *blkt_ptr, struct stage *stage_ptr);

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
 * @param[in,out] fptr FILE pointer.
 * @param[in,out] blkt_ptr Blockette structure.
 * @param[in,out] stage_ptr Stage structure.
 * @author 06/27/00: I.Dricker (i.dricker@isti.com) for 2.3.17 iir.
 */
void parse_iir_coeff(FILE *fptr, struct blkt *blkt_ptr, struct stage *stage_ptr);

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
 * @author 06/21/00: Ilya Dricker ISTI: I modify this routine to accomodate
 *         the form of the parsed blockette 55 generated by SeismiQuery.
 *         Since currently the blockette 55 is not supported, we do not
 *         anticipate problems caused by this change.
 */
void parse_list(FILE *fptr, struct blkt *blkt_ptr, struct stage *stage_ptr);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
void parse_generic(FILE *, struct blkt *, struct stage *); /* generic */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
int parse_deci(FILE *, struct blkt *); /* decimation */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
int parse_gain(FILE *, struct blkt *); /* gain/sensitivity */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
void parse_fir(FILE *, struct blkt *, struct stage *); /* fir */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
void parse_ref(FILE *, struct blkt *, struct stage *); /* response reference */

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief FIXME.
 */
void parse_polynomial(FILE *, struct blkt *, struct stage *); /* polynomial B42 B62 IGD 05/31/2013 */

/* remove trailing white space from (if last arg is 'a') and add null character to
 end of (if last arg is 'a' or 'e') input (FORTRAN) string */

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
 */
int add_null(char *s, int len, char where);

/* run a sanity check on the channel's filter sequence */

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief FIXME.
 */
void merge_coeffs(struct blkt *, struct blkt **);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief FIXME.
 */
void merge_lists(struct blkt *, struct blkt **); /* Added by I.Dricker IGD for v
 3.2.17 of evalresp */

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief FIXME.
 */
void check_channel(struct channel *);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief FIXME.
 */
void check_sym(struct blkt *, struct channel *);

/* routines used to calculate the instrument responses */

/*void calc_resp(struct channel *, double *, int, struct evr_complex *,char *, int, int);*/
/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void calc_resp(struct channel *chan, double *freq, int nfreqs,
        struct evr_complex *output, char *out_units, int start_stage,
        int stop_stage, int useTotalSensitivityFlag, double x_for_b62);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void convert_to_units(int, char *, struct evr_complex *, double);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void analog_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void fir_sym_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void fir_asym_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void iir_pz_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void calc_time_shift(double, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void zmul(struct evr_complex *, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void norm_resp(struct channel *, int, int);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void calc_list(struct blkt *, int, struct evr_complex *); /*IGD i.dricker@isti.c
om for version 3.2.17 */
/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void calc_polynomial(struct blkt *, int, struct evr_complex *, double); /*IGD 06/01/2013 */

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
void iir_trans(struct blkt *, double, struct evr_complex *); /* IGD for version 3.2.17 */

/**
 * @private
 * @ingroup evalresp_private_string
 * @brief A function that tests whether a string looks like a time string
 *        using string_match().
 * @details Time strings must be in the format 'hh:mm:ss[.#####]', so more
 *          than 14 characters is an error (too many digits).
 * @param[in] test String to test.
 * @returns 0 if false.
 * @returns >0 if true.
 */
int is_time(const char *test);

/**
 * @private
 * @ingroup evalresp_private_parse
 * @brief Compare two times and determine if the first is greater, equal to,
 *        or less than the second.
 */
int timecmp(struct dateTime *dt1, struct dateTime *dt2);

/**
 * @private
 * @ingroup evalresp_private_print
 * @brief Print the channel info, followed by the list of filters.
 */
void print_chan(struct channel *, int, int, int, int, int, int);

/**
 * @private
 * @ingroup evalresp_private_print
 * @brief Print the response information to the output files.
 */
void print_resp(double *, int, struct response *, char *, int);

/**
 * @private
 * @ingroup evalresp_private_print
 * @brief Print the response information to the output files.
 */
void print_resp_itp(double *, int, struct response *, char *, int, int, double,
        int);

/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user.
 * @remark Calls evresp_itp() but with listinterp_tension set to 0.
 */
struct response *evresp(char *, char *, char *, char *, char *, char *, char *,
        double *, int, char *, char *, int, int, int, int, double, int);

/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user.
 */
struct response *evresp_itp(char *, char *, char *, char *, char *, char *,
        char *, double *, int, char *, char *, int, int, int, int, int, double,
        int, double, int);
/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user.
 * @remark Fortran interface.
 */
int evresp_1(char *sta, char *cha, char *net, char *locid, char *datime,
        char *units, char *file, double *freqs, int nfreqs, double *resp,
        char *rtype, char *verbose, int start_stage, int stop_stage,
        int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
		int xml_flag);

/**
 * @private
 * @ingroup evalresp_private_response
 * @brief Interpolates amplitude and phase values from the set of frequencies
 *        in the List blockette to the requested set of frequencies.
 */
void interpolate_list_blockette(double **freq, double **amp, double **phase,
        int *p_number_points, double *req_freq_arr, int req_num_freqs,
        double tension);

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern char SEEDUNITS[][UNITS_STR_LEN];

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define the "first line" and "first field" arguments that are used by
 *        the parsing routines (they are used to compensate for the fact that
 *        in parsing the RESP files, one extra line is always read because the
 *        "end" of a filter sequence cannot be determined until the first line
 *        of the "next" filter sequence or station-channel pair is read from
 *        the file.
 */
extern char FirstLine[];

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define the "first line" and "first field" arguments that are used by
 *        the parsing routines (they are used to compensate for the fact that
 *        in parsing the RESP files, one extra line is always read because the
 *        "end" of a filter sequence cannot be determined until the first line
 *        of the "next" filter sequence or station-channel pair is read from
 *        the file.
 */
extern int FirstField;

/* values for use in calculating responses (defined in 'evresp_') */

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern double Pi;

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern double twoPi;

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define a global flag to use if using "default" units.
 */
extern int def_units_flag;

/**
 * @private
 * @ingroup evalresp_private
 * @brief Define a pointer to a channel structure to use in determining the
 *        input and output units if using "default" units and for use in error
 *        output.
 */
extern struct channel *GblChanPtr;

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern float unitScaleFact;

/* define global variables for use in printing error messages */

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern char *curr_file;

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern int curr_seq_no;

/* and set a global variable to contain the environment for the setjmp/longjmp
 combination for error handling */

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
extern jmp_buf jump_buffer;

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
double unwrap_phase(double phase, double prev_phase, double range,
        double *added_value);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief FIXME.
 */
double wrap_phase(double phase, double range, double *added_value);

/**
 * @private
 * @ingroup evalresp_private
 * @brief FIXME.
 */
int use_estimated_delay(int flag);

#endif /* !EVRESP_H */
