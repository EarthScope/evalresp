
#ifndef EVALRESP_PUBLIC_API_H
#define EVALRESP_PUBLIC_API_H

#include <stdio.h>

#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

#if 0
#define EVALRESP_OK 0
#define EVALRESP_MEM 1
#define EVALRESP_IO 2
#define EVALRESP_INP 3
#endif
enum evalresp_status_enum
{
  EVALRESP_OK = 0,    /**< No error (intentionally false). */
  EVALRESP_MEM,       /**< Memory error. */
  EVALRESP_IO,        /**< IO Error. */
  EVALRESP_INP,       /**< Bad user input. */
  EVALRESP_INV_FORMAT /**< Bad format for a function */
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
 * @brief Structure used for filtering input data by response DNCL and date/time.
 */
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

// TODO - rename to _format ?
typedef enum {
  evalresp_ap_user_format,     /**< Two files, AMP and PHASE. */
  evalresp_fap_user_format,    /**< One file, FAP. */
  evalresp_complex_user_format /**< One file, COMPLEX. */
} evalresp_user_format;

typedef enum {
  evalresp_default_user_unit,
  evalresp_displacement_user_unit,
  evalresp_velocity_user_unit,
  evalresp_acceleration_user_unit
} evalresp_user_unit;

#define EVALRESP_ALL_STAGES -1 /**< Default for start and stop stage. */

typedef struct
{
  double min_freq;
  double max_freq;
  int nfreq;
  int lin_freq;
  evalresp_user_format format;
  int start_stage;
  int stop_stage;
  int use_estimated_delay;
  int unwrap_phase;
  double b62_x;
} evalresp_options;

int evalresp_new_options (evalresp_log_t *log, evalresp_options **options);

void evalresp_free_options (evalresp_options **options);

int evalresp_set_frequency (evalresp_log_t *log, evalresp_options *options,
                            const char *min_freq, const char *max_freq, const char *nfreq);

int evalresp_set_format (evalresp_log_t *log, evalresp_options *options,
                         const char *format);

// these are separate because it simplifies calling from main routine

int evalresp_set_start_stage (evalresp_log_t *log, evalresp_options *options,
                              const char *stage);

int evalresp_set_stop_stage (evalresp_log_t *log, evalresp_options *options,
                             const char *stage);

int evalresp_set_b62_x (evalresp_log_t *log, evalresp_options *options,
                        const char *b62_x);

int evalresp_channel_to_response (evalresp_log_t *log, const evalresp_channel *channel,
                                  evalresp_options *options, evalresp_response **response);

int evalresp_channels_to_responses (evalresp_log_t *log, const evalresp_channels *channels,
                                    evalresp_options *options, evalresp_responses **responses);

// TODO - rename to file_format or similar
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

// TODO - do we need separate options here?  separate something out from earlier?
int evalresp_responses_to_dir (evalresp_log_t *log, const evalresp_responses *responses,
                               evalresp_output_options *output_options, const char *dir);

int evalresp_dir_to_dir (evalresp_log_t *log, const char *dir,
                         evalresp_options *options, evalresp_output_options *output_options);

#endif
