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
 * @defgroup evalresp_evalresp Private evalresp Interface
 * @ingroup evalresp
 * @brief Private evalresp interface.
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
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define TRUE 1

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define FALSE 0

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.

 */
#define QUERY_DELAY -1

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.

 */
#define STALEN 64

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define NETLEN 64

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.

 */
#define LOCIDLEN 64

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define CHALEN 64

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define OUTPUTLEN 256

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define TMPSTRLEN 64

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define UNITS_STR_LEN 16

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define DATIMLEN 23

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define UNITSLEN 20

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define BLKTSTRLEN 4

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define FLDSTRLEN 3

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define MAXFLDLEN 50

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define MAXLINELEN 256

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define FIR_NORM_TOL 0.02

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define CORRECTION_APPLIED_FLAG 0

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define ESTIMATED_DELAY_FLAG 1

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
#define CALC_DELAY_FLAG 2

/* enumeration representing the types of units encountered (Note: if default,
 then the response is just given in input units to output units, no
 interpretation is made of the units used) */

/* IGD 02/03/01 New unit pressure  added */
/* IGD 08/21/06 New units TESLA added */
/* IGD 10/03/13 New units  CENTIGRADE added */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
enum units {
    UNDEF_UNITS,
    DIS,
    VEL,
    ACC,
    COUNTS,
    VOLTS,
    DEFAULT,
    PRESSURE,
    TESLA,
    CENTIGRADE
};

/*  enumeration representing the types of filters encountered  */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
enum filt_types {
    UNDEF_FILT,
    LAPLACE_PZ,
    ANALOG_PZ,
    IIR_PZ,
    FIR_SYM_1,
    FIR_SYM_2,
    FIR_ASYM,
    LIST,
    GENERIC,
    DECIMATION,
    GAIN,
    REFERENCE,
    FIR_COEFFS,
    IIR_COEFFS,
    POLYNOMIAL
};

/* enumeration representing the types of stages that are recognized */
/* IGD 05/15/02 Added GENERIC_TYPE */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
enum stage_types {
    UNDEF_STAGE,
    PZ_TYPE,
    IIR_TYPE,
    FIR_TYPE,
    GAIN_TYPE,
    LIST_TYPE,
    IIR_COEFFS_TYPE,
    GENERIC_TYPE,
    POLYNOMIAL_TYPE
};

/* enumeration representing the types of error codes possible */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
enum error_codes {
    NON_EXIST_FLD = -2,
    ILLEGAL_RESP_FORMAT = -5,
    PARSE_ERROR = -4,
    UNDEF_PREFIX = -3,
    UNDEF_SEPSTR = -6,
    OUT_OF_MEMORY = -1,
    UNRECOG_FILTYPE = -7,
    UNEXPECTED_EOF = -8,
    ARRAY_BOUNDS_EXCEEDED = -9,
    OPEN_FILE_ERROR = 2,
    RE_COMP_FAILED = 3,
    MERGE_ERROR = 4,
    SWAP_FAILED = 5,
    USAGE_ERROR = 6,
    BAD_OUT_UNITS = 7,
    IMPROP_DATA_TYPE = -10,
    UNSUPPORT_FILTYPE = -11,
    ILLEGAL_FILT_SPEC = -12,
    NO_STAGE_MATCHED = -13,
    UNRECOG_UNITS = -14
};

/* define structures for the compound data types used in evalesp */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct evr_complex {
    double real;
    double imag;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct string_array {
    int nstrings;
    char **strings;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct scn {
    char *station;
    char *network;
    char *locid;
    char *channel;
    int found;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct response {
    char station[STALEN];
    char network[NETLEN];
    char locid[LOCIDLEN];
    char channel[CHALEN];
    struct evr_complex *rvec;
    int nfreqs; /*Add by I.Dricker IGD to  support blockette 55 */
    double *freqs; /*Add by I.Dricker IGD to  support blockette 55 */
    struct response *next;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct file_list {
    char *name;
    struct file_list *next_file;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct matched_files {
    int nfiles;
    struct file_list *first_list;
    struct matched_files *ptr_next;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct scn_list {
    int nscn;
    struct scn **scn_vec;
};

/* define structures for the various types of filters defined in seed */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct pole_zeroType { /* a Response (Poles & Zeros) blockette */
    int nzeros; /* (blockettes [43] or [53]) */
    int npoles;
    double a0;
    double a0_freq;
    struct evr_complex *zeros;
    struct evr_complex *poles;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct coeffType { /* a Response (Coefficients) blockette */
    int nnumer; /* (blockettes [44] or [54]) */
    int ndenom;
    double *numer;
    double *denom;
    double h0; /*IGD this field is new v 3.2.17 */
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct polynomialType { /* a Response (Coefficients) blockette */
    unsigned char approximation_type; /* (blockettes [42] or [62]) IGD 05/31/2013 */
    unsigned char frequency_units;
    double lower_freq_bound;
    double upper_freq_bound;
    double lower_approx_bound;
    double upper_approx_bound;
    double max_abs_error;
    int ncoeffs;
    double *coeffs;
    double *coeffs_err;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct firType { /* a FIR Response blockette */
    int ncoeffs; /* (blockettes [41] or [61])*/
    double *coeffs;
    double h0;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct listType { /* a Response (List) blockette */
    int nresp; /* (blockettes [45] or [55]) */
    double *freq;
    double *amp;
    double *phase;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct genericType { /* a Generic Response blockette */
    int ncorners; /* (blockettes [46] or [56]) */
    double *corner_freq;
    double *corner_slope;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct decimationType { /* a Decimation blockette */
    double sample_int; /* (blockettes [47] or [57]) */
    int deci_fact;
    int deci_offset;
    double estim_delay;
    double applied_corr;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct gainType { /* a Channel Sensitivity/Gain blockette */
    double gain; /* (blockettes [48] or [58]) */
    double gain_freq;
};

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct referType { /* a Response Reference blockette */
    int num_stages;
    int stage_num;
    int num_responses;
};

/* define a blkt as a stucture containing the blockette type, a union
 (blkt_info) containing the blockette info, and a pointer to the next
 blockette in the filter sequence.  The structures will be assembled to
 form a linked list of blockettes that make up a filter, with the last
 blockette containing a '(struct blkt *)NULL' pointer in the 'next_blkt'
 position */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt {
    int type;
    union {
        struct pole_zeroType pole_zero;
        struct coeffType coeff;
        struct firType fir;
        struct listType list;
        struct genericType generic;
        struct decimationType decimation;
        struct gainType gain;
        struct referType reference;
        struct polynomialType polynomial;
    } blkt_info;
    struct blkt *next_blkt;
};

/* define a stage as a structure that contains the sequence number, the
 input and output units, a pointer to the first blockette of the filter,
 and a pointer to the next stage in the response.  Again, the last
 stage in the response will be indicated by a '(struct stage *)NULL pointer
 in the 'next_stage' position */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct stage {
    int sequence_no;
    int input_units;
    int output_units;
    struct blkt *first_blkt;
    struct stage *next_stage;
};

/* and define a channel as a stucture containing a pointer to the head of a
 linked list of stages.  Will access the pieces one stages at a time in the
 same order that they were read from the input file, so a linked list is the
 easiest way to do this (since we don't have to worry about the time penalty
 inherent in following the chain).  As an example, if the first stage read
 was a pole-zero stage, then the parts of that stage contained in a channel
 structure called "ch" would be accessed as:

 struct stage *stage_ptr;
 struct blkt *blkt_ptr;

 stage_ptr = ch.first_stage;
 blkt_ptr = stage_ptr->first_blkt;
 if(blkt_ptr->type == LAPLACE_PZ || blkt_ptr->type == ANALOG_PZ ||
 blkt_ptr->type == IIR_PZ){
 nzeros = blkt_ptr->blkt_info.poles_zeros.nzeros;
 ........
 }

 */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct channel {
    char staname[STALEN];
    char network[NETLEN];
    char locid[LOCIDLEN];
    char chaname[CHALEN];
    char beg_t[DATIMLEN];
    char end_t[DATIMLEN];
    char first_units[MAXLINELEN];
    char last_units[MAXLINELEN];
    double sensit;
    double sensfreq;
    double calc_sensit;
    double calc_delay;
    double estim_delay;
    double applied_corr;
    double sint;
    int nstages;
    struct stage *first_stage;
};

/* structure used for time comparisons */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct dateTime {
    int year;
    int jday;
    int hour;
    int min;
    float sec;
};

/* IGD 2007/02/27 */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern char myLabel[20];
//char myLabel[20] = "aa";

/* utility routines that are used to parse the input file line by line and
 convert the input to what the user wants */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct string_array *ev_parse_line(char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct string_array *parse_delim_line(char *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_field(FILE *, char *, int, int, char *, int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int test_field(FILE *, char *, int *, int *, char *, int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_line(FILE *, char *, int, int, char *); /* checks blkt & fld nos */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int next_line(FILE *, char *, int *, int *, char *); /* returns blkt & fld nos */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int count_fields(char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int count_delim_fields(char *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_field(char *, int, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_delim_field(char *, int, char *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int check_line(FILE *, int *, int *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_int(char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
double get_double(char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int check_units(char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int string_match(const char *, char *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int is_int(const char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int is_real(const char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int is_IIR_coeffs(FILE *, int); /*IGD */

/* routines used to load a channel's response information into a linked
 list of filter stages, each containing a linked list of blockettes */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int find_resp(FILE *, struct scn_list *, char *, struct channel *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_resp(FILE *, struct scn *, char *, struct channel *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_channel(FILE *, struct channel *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int next_resp(FILE *);

/* routines used to create a list of files matching the users request */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct matched_files *find_files(char *, struct scn_list *, int *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int get_names(char *, struct matched_files *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int start_child(char *, FILE **, FILE **, FILE **);

/* routines used to allocate vectors of the basic data types used in the
 filter stages */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct evr_complex *alloc_complex(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct response *alloc_response(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct string_array *alloc_string_array(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct scn *alloc_scn(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct scn_list *alloc_scn_list(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct file_list *alloc_file_list(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct matched_files *alloc_matched_files(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
double *alloc_double(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
char *alloc_char(int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
char **alloc_char_ptr(int);

/* allocation routines for the various types of filters */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_pz(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_coeff(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_fir(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_ref(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_gain(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_list(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_generic(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_deci(void);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct blkt *alloc_polynomial(void); /*IGD 05/31/2013 */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct stage *alloc_stage(void);

/* routines to free up space associated with dynamically allocated
 structure members */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_string_array(struct string_array *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_scn(struct scn *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_scn_list(struct scn_list *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_matched_files(struct matched_files *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_file_list(struct file_list *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_pz(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_coeff(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_fir(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_list(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_generic(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_gain(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_deci(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_ref(struct blkt *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_stages(struct stage *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_channel(struct channel *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void free_response(struct response *);

/* simple error handling routines to standardize the output error values and
 allow for control to return to 'evresp' if a recoverable error occurs */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void error_exit(int, char *, ...);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void error_return(int, char *, ...);

/* a simple routine that parses the station information from the input file */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_channel(FILE *, struct channel *);

/* parsing routines for various types of filters */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_pref(int *, int *, char *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_pz(FILE *, struct blkt *, struct stage *); /* pole-zero */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_coeff(FILE *, struct blkt *, struct stage *); /* fir */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_iir_coeff(FILE *, struct blkt *, struct stage *); /*I.Dricker IGD for
 2.3.17 iir */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_list(FILE *, struct blkt *, struct stage *); /* list */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_generic(FILE *, struct blkt *, struct stage *); /* generic */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_deci(FILE *, struct blkt *); /* decimation */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int parse_gain(FILE *, struct blkt *); /* gain/sensitivity */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_fir(FILE *, struct blkt *, struct stage *); /* fir */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_ref(FILE *, struct blkt *, struct stage *); /* response reference */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void parse_polynomial(FILE *, struct blkt *, struct stage *); /* polynomial B42 B62 IGD 05/31/2013 */

/* remove trailing white space from (if last arg is 'a') and add null character to
 end of (if last arg is 'a' or 'e') input (FORTRAN) string */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int add_null(char *, int, char);

/* run a sanity check on the channel's filter sequence */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void merge_coeffs(struct blkt *, struct blkt **);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void merge_lists(struct blkt *, struct blkt **); /* Added by I.Dricker IGD for v
 3.2.17 of evalresp */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void check_channel(struct channel *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void check_sym(struct blkt *, struct channel *);

/* routines used to calculate the instrument responses */

/*void calc_resp(struct channel *, double *, int, struct evr_complex *,char *, int, int);*/
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void calc_resp(struct channel *chan, double *freq, int nfreqs,
        struct evr_complex *output, char *out_units, int start_stage,
        int stop_stage, int useTotalSensitivityFlag, double x_for_b62);

/**
 * @private
 * @ingroup evalresp_evalresp]
 * @brief FIXME.
 */
void convert_to_units(int, char *, struct evr_complex *, double);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void analog_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void fir_sym_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void fir_asym_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void iir_pz_trans(struct blkt *, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void calc_time_shift(double, double, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void zmul(struct evr_complex *, struct evr_complex *);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void norm_resp(struct channel *, int, int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void calc_list(struct blkt *, int, struct evr_complex *); /*IGD i.dricker@isti.c
om for version 3.2.17 */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void calc_polynomial(struct blkt *, int, struct evr_complex *, double); /*IGD 06/01/2013 */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void iir_trans(struct blkt *, double, struct evr_complex *); /* IGD for version 3.2.17 */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int is_time(const char *);

/* compare two times and determine if the first is greater, equal to, or less than the second */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int timecmp(struct dateTime *dt1, struct dateTime *dt2);

/* print the channel info, followed by the list of filters */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void print_chan(struct channel *, int, int, int, int, int, int);

/* print the response information to the output files */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void print_resp(double *, int, struct response *, char *, int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void print_resp_itp(double *, int, struct response *, char *, int, int, double,
        int);

/* evaluate responses for user requested station/channel/network tuple at the
 frequencies requested by the user */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct response *evresp(char *, char *, char *, char *, char *, char *, char *,
        double *, int, char *, char *, int, int, int, int, double, int);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
struct response *evresp_itp(char *, char *, char *, char *, char *, char *,
        char *, double *, int, char *, char *, int, int, int, int, int, double,
        int, double, int);
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int evresp_1(char *sta, char *cha, char *net, char *locid, char *datime,
        char *units, char *file, double *freqs, int nfreqs, double *resp,
        char *rtype, char *verbose, int start_stage, int stop_stage,
        int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
		int xml_flag);

/* Interpolates amplitude and phase values from the set of frequencies
 in the List blockette to the requested set of frequencies. */
/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
void interpolate_list_blockette(double **freq, double **amp, double **phase,
        int *p_number_points, double *req_freq_arr, int req_num_freqs,
        double tension);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern char SEEDUNITS[][UNITS_STR_LEN];

/* define the "first line" and "first field" arguments that are used by the
 parsing routines (they are used to compensate for the fact that in
 parsing the RESP files, one extra line is always read because the
 "end" of a filter sequence cannot be determined until the first line
 of the "next" filter sequence or station-channel pair is read from the
 file */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern char FirstLine[];

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern int FirstField;

/* values for use in calculating responses (defined in 'evresp_') */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern double Pi;

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern double twoPi;

/* define a global flag to use if using "default" units */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern int def_units_flag;

/* define a pointer to a channel structure to use in determining the input and
 output units if using "default" units and for use in error output*/

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern struct channel *GblChanPtr;

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern float unitScaleFact;

/* define global variables for use in printing error messages */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern char *curr_file;

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern int curr_seq_no;

/* and set a global variable to contain the environment for the setjmp/longjmp
 combination for error handling */

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
extern jmp_buf jump_buffer;

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
double unwrap_phase(double phase, double prev_phase, double range,
        double *added_value);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
double wrap_phase(double phase, double range, double *added_value);

/**
 * @private
 * @ingroup evalresp_evalresp
 * @brief FIXME.
 */
int use_estimated_delay(int flag);

#endif /* !EVRESP_H */
