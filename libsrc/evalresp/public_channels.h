
#ifndef evalresp_public_low_level_channelS_H
#define evalresp_public_low_level_channelS_H

#include "evalresp/constants.h"
#include "evalresp_log/log.h"

/* define structures for the various types of filters defined in seed */

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief Complex data type.
 */
typedef struct evalresp_complex_s
{
  double real; /**< Real part. */
  double imag; /**< Imaginary part. */
} evalresp_complex;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Response (Poles & Zeros) blockette.
 */
typedef struct evalresp_pole_zero_s
{
  int nzeros;              /**< Number of zeros (blockettes [43] or [53]). */
  int npoles;              /**< Number of poles. */
  double a0;               /**< Poles and zeros normaliztion factor. */
  double a0_freq;          /**< Poles and zeros normaliztion frequency. */
  evalresp_complex *zeros; /**< Array of zeros (complex). */
  evalresp_complex *poles; /**< Array of poles (complex). */
} evalresp_pole_zero;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Response (Coefficients) blockette.
 */
typedef struct evalresp_coeff_s
{
  int nnumer;                     /**< Length of numerator vector . (blockettes [44] or [54]) */
  int ndenom;                     /**< Length of denominator vector. */
  double *numer;                  /**< Numerator vector. */
  double *denom;                  /**< Denominator vector. */
  double h0; /**< Sensitivity. */ /* IGD this field is new v 3.2.17 */
} evalresp_coeff;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Response (Coefficients) blockette.
 */
typedef struct evalresp_polynomial_s
{
  unsigned char approximation_type; /**< Approximation type. (blockettes [42] or [62]) */ /* IGD 05/31/2013 */
  unsigned char frequency_units;                                                          /**< Frequency unit. */
  double lower_freq_bound;                                                                /**< Lower frequency bound. */
  double upper_freq_bound;                                                                /**< Upper frequency bound. */
  double lower_approx_bound;                                                              /**< Lower approximation bound. */
  double upper_approx_bound;                                                              /**< Upper approximation bound. */
  double max_abs_error;                                                                   /**< Maximum absolute error. */
  int ncoeffs;                                                                            /**< Length of coefficients vector. */
  double *coeffs;                                                                         /**< Coefficients vector. */
  double *coeffs_err;                                                                     /**< Error vector. */
} evalresp_polynomial;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A FIR Response blockette.
 */
typedef struct evalresp_fir_s
{
  int ncoeffs;    /**< Number of coefficients (blockettes [41] or [61]). */
  double *coeffs; /**< Array of coefficients. */
  double h0;      /**< Sensitivity. */
} evalresp_fir;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Response (List) blockette.
 */
typedef struct evalresp_list_s
{
  int nresp;     /**< Number of responses (blockettes [45] or [55]). */
  double *freq;  /**< Array of freqencies. */
  double *amp;   /**< Array of amplitudes. */
  double *phase; /**< Array of phases. */
} evalresp_list;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Generic Response blockette.
 */
typedef struct evalresp_generic_s
{
  int ncorners;         /**< Number of corners. (blockettes [46] or [56]) */
  double *corner_freq;  /**< Corner frequency vector. */
  double *corner_slope; /**< Corner slope vector. */
} evalresp_generic;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Decimation blockette.
 */
typedef struct evalresp_decimation_s
{
  double sample_int;   /**< Sample interval. (blockettes [47] or [57]) */
  int deci_fact;       /**< Decimation factor. */
  int deci_offset;     /**< Decimation offset. */
  double estim_delay;  /**< Estimated delay. */
  double applied_corr; /**< Applied correction. */
} evalresp_decimation;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Channel Sensitivity/Gain blockette.
 */
typedef struct evalresp_gain_s
{
  double gain;      /**< Gain. (blockettes [48] or [58]) */
  double gain_freq; /**< Frequency where gain is computed. */
} evalresp_gain;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A Response Reference blockette.
 */
typedef struct evalresp_refer_s
{
  int num_stages;    /**< Total number of stages. */
  int stage_num;     /**< Stage number. */
  int num_responses; /**< Number of responses. */
} evalresp_refer;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief Define a blkt as a structure containing the blockette type, a union
 *        (blkt_info) containing the blockette info, and a pointer to the
 *        next blockette in the filter sequence.
 * @details The structures will be assembled to form a linked list of
 *          blockettes that make up a filter, with the last blockette
 *          containing a '(struct blkt *)NULL' pointer in the 'next_blkt'
 *          position.
 */
typedef struct evalresp_blkt_s
{
  int type; /**< Blockette type. */
  union {
    evalresp_pole_zero pole_zero;    /**< Poles and zeros structure. */
    evalresp_coeff coeff;            /**< Coefficients structure. */
    evalresp_fir fir;                /**< FIR structure. */
    evalresp_list list;              /**< List structure. */
    evalresp_generic generic;        /**< Generic response structure. */
    evalresp_decimation decimation;  /**< Decimation blockette structure. */
    evalresp_gain gain;              /**< Gain structure. */
    evalresp_refer reference;        /**< Reference structure. */
    evalresp_polynomial polynomial;  /**< Polynomial type structure. */
  } blkt_info;                       /**< Blockette info. */
  struct evalresp_blkt_s *next_blkt; /**< Pointer to next blockette. */
} evalresp_blkt;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief Define a stage as a structure that contains the sequence number, the
 *        input and output units, a pointer to the first blockette of the
 *        filter, and a pointer to the next stage in the response.
 * @details Again, the last stage in the response will be indicated by a
 *          '(struct stage *)NULL pointer in the 'next_stage' position.
 */
typedef struct evalresp_stage_s
{
  int sequence_no;                     /**< Sequence number. */
  int input_units;                     /**< Input units. */
  int output_units;                    /**< Output units. */
  char *input_units_str;               /**< Input units string. */
  char *output_units_str;              /**< Output units string. */
  evalresp_blkt *first_blkt;           /**< Pointer to first blockette of the filter. */
  struct evalresp_stage_s *next_stage; /**< Pointer to the next stage in the response. */
} evalresp_stage;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief Define a channel as a structure containing a pointer to the head
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
typedef struct evalresp_channel_s
{
  char staname[STALEN];         /**< Station name. */
  char network[NETLEN];         /**< Network name. */
  char locid[LOCIDLEN];         /**< Location ID. */
  char chaname[CHALEN];         /**< Channel name. */
  char beg_t[DATIMLEN];         /**< Start time (string). */
  char end_t[DATIMLEN];         /**< End time (string). */
  char first_units[MAXLINELEN]; /**< Units of the first stage. */
  char last_units[MAXLINELEN];  /**< Units of the last stage. */
  double sensit;                /**< Sensitivity. */
  double sensfreq;              /**< Frequency at sensitivity. */
  double calc_sensit;           /**< Calculated sensitivity. */
  double calc_delay;            /**< Calculated delay. */
  double estim_delay;           /**< Estimated delay. */
  double applied_corr;          /**< Applied correction. */
  double unit_scale_fact;       /**< Used to convert MKS / metric. */
  double sint;                  /**< Inverted sample rate (sample interval). */
  int nstages;                  /**< Number of stages. */
  evalresp_stage *first_stage;  /**< Pointer to the head of a linked list of
                                   stage. */
} evalresp_channel;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A collection of channel structures.
*/
typedef struct evalresp_channels_s
{
  int nchannels;
  evalresp_channel **channels;
} evalresp_channels;

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A function allocating memory for evalresp_channels object
TODO IGD: finish the description
*/

int
evalresp_alloc_channels (evalresp_logger *log, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A function freeing memory after evalresp_channels object
TODO IGD: finish the description
*/
void
evalresp_free_channels (evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_channel
 * @brief A function freeing memory after evalresp_channels object
TODO IGD: finish the description
*/
void
evalresp_free_channel (evalresp_channel **channel);

#endif
