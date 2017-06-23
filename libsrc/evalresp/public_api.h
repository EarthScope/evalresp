
#ifndef EVALRESP_PUBLIC_API_H
#define EVALRESP_PUBLIC_API_H

#include <stdio.h>

#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

enum evalresp_status_enum
{
  EVALRESP_OK = 0, /**< No error (intentionally false). */
  EVALRESP_MEM,    /**< Memory error. */
  EVALRESP_IO,     /**< IO Error. */
  EVALRESP_INP,    /**< Bad user input. */
  EVALRESP_ERR,    /**< Internal (coding) error. */
  EVALRESP_PAR    /**< Parsing error in file. */
};

// TODO - see design doc for details that should go into comments

/**
 * @public
 * @ingroup evalresp_public
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
 * @public
 * @ingroup evalresp_public
 * @brief Structure used for time comparisons.
 */
typedef struct
{
  int year;  /**< Year. */
  int jday;  /**< Day of year. */
  int hour;  /**< Hour. */
  int min;   /**< Minutes. */
  float sec; /**< Seconds. */
} evalresp_datetime;

/**
 * @public
 * @ingroup evalresp_public
 * @brief List of network-station-locid-channel objects.
 */
typedef struct
{
  int nscn;                /**< Number of network-station-locid-channel objects. */
  evalresp_sncl **scn_vec; /**< Array of network-station-locid-channel objects. */
} evalresp_sncls;

/**
 * @public
 * @ingroup evalresp_public
 * @brief Structure used for filtering input data by response DNCL and date/time.
 */
typedef struct
{
  evalresp_sncls *sncls;
  evalresp_datetime *datetime;
} evalresp_filter;

int evalresp_new_filter (evalresp_log_t *log, evalresp_filter **filter);

int evalresp_set_year (evalresp_log_t *log, evalresp_filter *filter, const char *year);

int evalresp_set_julian_day (evalresp_log_t *log, evalresp_filter *filter, const char *julian_day);

int evalresp_set_time (evalresp_log_t *log, evalresp_filter *filter, const char *time);

int evalresp_add_sncl_text (evalresp_log_t *log, evalresp_filter *filter,
                            const char *sta, const char *net, const char *chan, const char *locid);

int evalresp_add_sncl (evalresp_log_t *log, evalresp_filter *filter, evalresp_sncl *sncl);

/**
 * Splits comma-separated values in input and adds all combinations.
 */
int evalresp_add_sncl_all (evalresp_log_t *log, evalresp_filter *filter,
                           const char *sta, const char *net, const char *chan, const char *locid);

void evalresp_free_filter (evalresp_filter **filter);

/**
 * @public
 * @ingroup evalresp_public
 * @brief A routine that frees up the space associated with a station-channel
 *        list type structure.
 * @param[in,out] sncls SNCL data.
 */
void evalresp_free_sncls (evalresp_sncls *sncls);

int evalresp_char_to_channels (evalresp_log_t *log, const char *seed_or_xml,
                               const evalresp_filter *filter, evalresp_channels **channels);

int evalresp_file_to_channels (evalresp_log_t *log, FILE *file,
                               const evalresp_filter *filter, evalresp_channels **channels);

int evalresp_filename_to_channels (evalresp_log_t *log, const char *filename,
                                   const evalresp_filter *filter, evalresp_channels **channels);

typedef enum {
  evalresp_ap_output_format,     /**< Two files, AMP and PHASE. */
  evalresp_fap_output_format,    /**< One file, FAP. */
  evalresp_complex_output_format /**< One file, COMPLEX. */
} evalresp_output_format;

typedef enum {
  evalresp_default_unit,
  evalresp_displacement_unit,
  evalresp_velocity_unit,
  evalresp_acceleration_unit
} evalresp_unit;

#define EVALRESP_ALL_STAGES -1 /**< Default for start and stop stage. */
#define EVALRESP_NO_FREQ -1    /**< Default for frequency limits. */

typedef struct
{
  char *filename;
  double b62_x;
  double min_freq;
  double max_freq;
  int nfreq;
  int lin_freq;
  int start_stage;
  int stop_stage;
  int use_estimated_delay;
  int unwrap_phase;
  int b55_interpolate;
  int use_total_sensitivity;
  int use_stdio;
  int station_xml;
  evalresp_output_format format;
  evalresp_unit unit;
} evalresp_options;

int evalresp_new_options (evalresp_log_t *log, evalresp_options **options);

void evalresp_free_options (evalresp_options **options);

int evalresp_set_filename (evalresp_log_t *log, evalresp_options *options, const char *filename);

int evalresp_set_frequency (evalresp_log_t *log, evalresp_options *options,
                            const char *min_freq, const char *max_freq, const char *nfreq);

int evalresp_set_format (evalresp_log_t *log, evalresp_options *options,
                         const char *format);

int evalresp_set_unit (evalresp_log_t *log, evalresp_options *options,
                       const char *unit);

int evalresp_set_spacing (evalresp_log_t *log, evalresp_options *options,
                          const char *spacing);

// these are separate because it simplifies calling from main routine

int evalresp_set_start_stage (evalresp_log_t *log, evalresp_options *options,
                              const char *stage);

int evalresp_set_stop_stage (evalresp_log_t *log, evalresp_options *options,
                             const char *stage);

int evalresp_set_b62_x (evalresp_log_t *log, evalresp_options *options,
                        const char *b62_x);

int evalresp_channel_to_response (evalresp_log_t *log, evalresp_channel *channel,
                                  evalresp_options *options, evalresp_response **response);

int evalresp_channels_to_responses (evalresp_log_t *log, evalresp_channels *channels,
                                    evalresp_options *options, evalresp_responses **responses);

typedef enum {
  evalresp_fap_file_format,
  evalresp_amplitude_file_format,
  evalresp_phase_file_format,
  evalresp_complex_file_format
} evalresp_file_format;

/**
 * @public
 * @ingroup evalresp_public
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] output pointer to the char * that the response will be printed into
 * @brief take an evalresp response and put it in an evalresp formated string
 * @retval EVALRESP_OK on success
 * @post output will be allocated on success and must be free'd by other functions
 */
int evalresp_response_to_char (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_file_format format, char **output);

/**
 * @public
 * @ingroup evalresp_public
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] file stream to print the formated response into
 * @brief take an evalresp response and put it in an evalresp formated stream
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_stream (evalresp_log_t *log, const evalresp_response *response,
                                 evalresp_file_format format, FILE *file);

/**
 * @public
 * @ingroup evalresp_public
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] filename filename to print the formated response into
 * @brief take an evalresp response and put it in an evalresp formated file
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_file (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_file_format format, const char *filename);

int evalresp_responses_to_cwd (evalresp_log_t *log, const evalresp_responses *responses,
                               evalresp_output_format format, int use_stdio);

int evalresp_cwd_to_cwd (evalresp_log_t *log,
                         evalresp_options *options, evalresp_filter *filter);

#endif
