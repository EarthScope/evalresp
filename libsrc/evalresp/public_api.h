
#ifndef EVALRESP_PUBLIC_API_H
#define EVALRESP_PUBLIC_API_H

#include <stdio.h>

#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

#define EVALRESP_OK 0  /**< No error (intentionally false). */
#define EVALRESP_MEM 1 /**< Memory error. */
#define EVALRESP_IO 2  /**< IO Error. */
#define EVALRESP_INP 3 /**< Bad user input. */

// TODO - see design doc for details that should go into comments

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

typedef struct
{
  int nsncls;
  evalresp_sncl **sncls;
  evalresp_datetime *datetime;
} evalresp_filter;

int evalresp_new_filter (evalresp_log_t *log, evalresp_filter **filter);

int evalresp_set_year (evalresp_log_t *log, evalresp_filter *filter, const char *year);

int evalresp_set_julian_day (evalresp_log_t *log, evalresp_filter *filter, const char *julian_day);

int evalresp_set_time (evalresp_log_t *log, evalresp_filter *filter, const char *time);

int evalresp_add_sncl (evalresp_log_t *log, evalresp_filter *filter,
                       const char *sta, const char *net, const char *chan, const char *locid);

void evalresp_free_filter (evalresp_filter **filter);

int evalresp_char_to_channels (evalresp_log_t *log, const char *seed_or_xml,
                               const evalresp_filter *filter, evalresp_channels **channels);

int evalresp_file_to_channels (evalresp_log_t *log, FILE *file,
                               const evalresp_filter *filter, evalresp_channels **channels);

int evalresp_filename_to_channels (evalresp_log_t *log, const char *filename,
                                   const evalresp_filter *filter, evalresp_channels **channels);

// TODO - everything below here needs implementing

typedef struct
{

} evalresp_eval_options;

int evalresp_channel_to_response (evalresp_log_t *log, const evalresp_channel *channel,
                                  const evalresp_eval_options *eval_options,
                                  evalresp_responses **responses);

int evalresp_channels_to_responses (evalresp_log_t *log, const evalresp_channels *channels,
                                    const evalresp_eval_options *eval_options,
                                    evalresp_responses **responses);

typedef enum {
  evalresp_fap_format,
  evalresp_amplitude_format,
  evalresp_phase_format,
  evalresp_complex_format
} evalresp_format;

int evalresp_response_to_char (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_format format, char **output);

int evalresp_response_to_file (evalresp_log_t *log, const evalresp_response *response,
                               evalresp_format format, const char *filename);

typedef struct
{

} evalresp_output_options;

int evalresp_responses_to_dir (evalresp_log_t *log, const evalresp_responses *responses,
                               evalresp_output_options *output_options, const char *dir);

int evalresp_dir_to_dir (evalresp_log_t *log, const char *dir,
                         evalresp_eval_options *eval_options, evalresp_output_options *output_options);

#endif
