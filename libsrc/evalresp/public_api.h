
#ifndef EVALRESP_PUBLIC_API_H
#define EVALRESP_PUBLIC_API_H

#include <stdio.h>

#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

/**
 * @public
 * @ingroup evalresp_public
 * @brief Enumeration of status values.
 *
 * Most routines return a status value that indicates whether an error occurred
 * during the call.  If an error did occur then an explanation was probably logged
 * and the output parameters will not contain useful values.
 *
 * The success value (EVALRESP_OK) is intentionally false to match the standard C
 * idiom where the value is negated to test for failure.
 */
enum evalresp_status_enum
{
  EVALRESP_OK = 0, /**< No error (intentionally false). */
  EVALRESP_MEM, /**< Memory error. */
  EVALRESP_IO, /**< IO Error. */
  EVALRESP_INP, /**< Bad user input. */
  EVALRESP_ERR, /**< Internal (coding) error. */
  EVALRESP_PAR, /**< Parsing error in file. */
  EVALRESP_EOF, /**< End of input. */
  EVALRESP_VAL /**< Validation error. */
};

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Structure used for time comparisons.
 */
typedef struct
{
  int year; /**< Year. */
  int jday; /**< Day of year. */
  int hour; /**< Hour. */
  int min; /**< Minutes. */
  float sec; /**< Seconds. */
} evalresp_datetime;

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Structure used for filtering input data by response DNCL and date/time.
 */
typedef struct
{
  struct evalresp_sncls_s *sncls;
  evalresp_datetime *datetime;
} evalresp_filter;

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Enumeration of output formats (FAP, complex, etc - can require multiple files).
 */
typedef enum {
  evalresp_ap_output_format, /**< Two files, AMP and PHASE. */
  evalresp_fap_output_format, /**< One file, FAP. */
  evalresp_complex_output_format /**< One file, COMPLEX. */
} evalresp_output_format;

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Enumeration of response units (acceleration, velocity or displacement).
 */
typedef enum {
  evalresp_default_unit, /**< Despite the name, this is not the default. */
  evalresp_displacement_unit,
  evalresp_velocity_unit,
  evalresp_acceleration_unit
} evalresp_unit;

#define EVALRESP_ALL_STAGES -1 /**< Default for start and stop stage. */
#define EVALRESP_NO_FREQ -1 /**< Default for frequency limits. */

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Structure containing the options that affect how responses are evaluated and
 * generated.
 */
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
  char format_set;
  int verbose;
  char unit_set;
} evalresp_options;

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[out] filter pointer to be allocated and initialized
 * @brief allocate and intialize a evalresp_filter
 * @retval EVALRESP_OK on success
 */
int evalresp_new_filter (evalresp_logger *log, evalresp_filter **filter);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the year
 * @param[in] year 
 * @brief set the year in an evalresp_filter object
 * @retval EVALRESP_OK on success
 */
int evalresp_set_year (evalresp_logger *log, evalresp_filter *filter, const char *year);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the day of theyear
 * @param[in] julian_day the day of the year
 * @brief set the jday in an evalresp_filter object
 * @retval EVALRESP_OK on success
 */
int evalresp_set_julian_day (evalresp_logger *log, evalresp_filter *filter, const char *julian_day);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the time
 * @param[in] time
 * @brief set the time in an evalresp_filter object
 * @retval EVALRESP_OK on success
 */
int evalresp_set_time (evalresp_logger *log, evalresp_filter *filter, const char *time);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set 
 * @param[in] sta the station code
 * @param[in] net the network code
 * @param[in] chan the channel code
 * @param[in] locid the location id
 * @brief set the sncl in an evalresp_filter object from the text string, expects single station
 * @retval EVALRESP_OK on success
 */
int evalresp_add_sncl_text (evalresp_logger *log, evalresp_filter *filter,
                            const char *sta, const char *net, const char *chan, const char *locid);

/**
 * Splits comma-separated values in input and adds all combinations.
 */
/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set 
 * @param[in] sta the station code list
 * @param[in] net the network code list
 * @param[in] chan the channel code list
 * @param[in] locid the location id list
 * @brief Splits comma-separated values in input and adds all combinations to evalresp_filter object.
 * @retval EVALRESP_OK on success
 */
int evalresp_add_sncl_all (evalresp_logger *log, evalresp_filter *filter,
                           const char *sta, const char *net, const char *chan, const char *locid);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] filter
 * @brief properly free filter object
 * @retval EVALRESP_OK on success
 */
void evalresp_free_filter (evalresp_filter **filter);

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] seed_or_xml char * read from a file or stdio
 * @param[in] options evalresp_options to determine wether or not to convert xml or not
 * @param[in] filter evalresp filter to use when getting the channels
 * @param[out] channels evalresp_channel object that gets allocated and returned
 * @brief Take a char string and parse it into evalresp_channel object
 * @retval EVALRESP_OK on success
 */
int evalresp_char_to_channels (evalresp_logger *log, const char *seed_or_xml,
                               evalresp_options const *const options,
                               const evalresp_filter *filter, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] file stream pointer that contains the seed/xml data to convert
 * @param[in] options evalresp_options to determine wether or not to convert xml or not
 * @param[in] filter evalresp filter to use when getting the channels
 * @param[out] channels evalresp_channel object that gets allocated and returned
 * @brief take a stream and parse it into evalresp_channel object
 * @retval EVALRESP_OK on success
 */
int evalresp_file_to_channels (evalresp_logger *log, FILE *file,
                               evalresp_options const *const options,
                               const evalresp_filter *filter, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] filename name of the file that contains the seed/xml data to convert
 * @param[in] options evalresp_options to determine wether or not to convert xml or not
 * @param[in] filter evalresp filter to use when getting the channels
 * @param[out] channels evalresp_channel object that gets allocated and returned
 * @brief take a filename, open it, and parse it into evalresp_channel object
 * @retval EVALRESP_OK on success
 */
int evalresp_filename_to_channels (evalresp_logger *log, const char *filename, evalresp_options const *const options,
                                   const evalresp_filter *filter, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[out] options pointer to options that is allocated
 * @brief allocate and initialize an evalresp_options.
 * @retval EVALRESP_OK on success
 */
int evalresp_new_options (evalresp_logger *log, evalresp_options **options);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] options the options that need to be deallocated
 * @brief guarentee memory allocated for evalresp_options is deallocated
 */
void evalresp_free_options (evalresp_options **options);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in[ filename filename to set in options
 * @brief parse filename into options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_filename (evalresp_logger *log, evalresp_options *options, const char *filename);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] min_freq minimum frequency as a string to parse into evalresp_option
 * @param[in] max_freq Maximum frequency as a string to parse into evalresp_option
 * @param[in] nfreq number of frequencies as a string to parse into evalresp_option
 * @brief parse the frequency string inputs into options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_frequency (evalresp_logger *log, evalresp_options *options,
                            const char *min_freq, const char *max_freq, const char *nfreq);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] format the format string, valid strings are "AP", "FAP", and "CS"
 * @brief parse the format string into options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_format (evalresp_logger *log, evalresp_options *options,
                         const char *format);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] unit sting that contains the unit type, valid strings are "DIS", "VEL", "ACC", or "DEF"
 * @brief[in] parse unit string into options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_unit (evalresp_logger *log, evalresp_options *options,
                       const char *unit);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] spacing string stating if the delta in the frequencies is "LIN" for linear or "LOG" for logaritmic step size
 * @brief parse the spaceing string into options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_spacing (evalresp_logger *log, evalresp_options *options,
                          const char *spacing);

// these are separate because it simplifies calling from main routine

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] stage starting stage as a string, "-1" means start at first availible stage
 * @brief parse stage into starting_stage of options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_start_stage (evalresp_logger *log, evalresp_options *options,
                              const char *stage);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] stage stopping stage as a string
 * @brief parse stage into starting_stage of options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_stop_stage (evalresp_logger *log, evalresp_options *options,
                             const char *stage);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] b62_x counts/volts as a string
 * @brief parse counts or volts from b62_x to options
 * @retval EVALRESP_OK on success
 */
int evalresp_set_b62_x (evalresp_logger *log, evalresp_options *options,
                        const char *b62_x);

/**
 * @public
 * @ingroup evalresp_public_low_level_evaluation
 * @param[in] log logging structure
 * @param[in] channel Evalresp_channel object to be converted into a response
 * @param[in] options evalresp_options that have values to control the way responses are calculated
 * @param[out] response an allocated response created from channel
 * @brief convert an evalresp_channel to and evalresp_response and allocate it
 * @retval EVALRESP_OK on success
 */
int evalresp_channel_to_response (evalresp_logger *log, evalresp_channel *channel,
                                  evalresp_options *options, evalresp_response **response);

/**
 * @public
 * @ingroup evalresp_public_low_level_evaluation
 * @param[in] log logging structure
 * @param[in] channels evalresp_channels to be converted into responses
 * @param[in] options evalresp_options to control the  way responses are calculated
 * @param[in,out] responses a pointer to evalresp_responses object, if *responses == NULL then it will be allocated
 * @brief go through the channels in evalresp_channels and converthem into a evalresp_responses object
 * @retval EVALRESP_OK on success
 */
int evalresp_channels_to_responses (evalresp_logger *log, evalresp_channels *channels,
                                    evalresp_options *options, evalresp_responses **responses);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @brief Enumeration of output file formats (for a single file).
 */
typedef enum {
  evalresp_fap_file_format,
  evalresp_amplitude_file_format,
  evalresp_phase_file_format,
  evalresp_complex_file_format
} evalresp_file_format;

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] output pointer to the char * that the response will be printed into
 * @brief take an evalresp response and put it in an evalresp formated string
 * @retval EVALRESP_OK on success
 * @post output will be allocated on success and must be free'd by other functions
 */
int evalresp_response_to_char (evalresp_logger *log, const evalresp_response *response,
                               evalresp_file_format format, char **output);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] file stream to print the formated response into
 * @brief take an evalresp response and put it in an evalresp formated stream
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_stream (evalresp_logger *log, const evalresp_response *response,
                                 evalresp_file_format format, FILE *const file);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] format enum value of what char format to put the string into
 * @param[out] filename filename to print the formated response into
 * @brief take an evalresp response and put it in an evalresp formated file
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_file (evalresp_logger *log, const evalresp_response *response,
                               evalresp_file_format format, const char *filename);

/**
 * @public
 * @ingroup evalresp_public_high_level
 * @param[in] log logging structure
 * @param[in] options object to control the flow of the conversion to responses
 * @param[in] filter object on how to filter the inputed files
 * @brief take files from cwd and output them into the cwd based on the options and filters
 * @post response files created in current working directory or stdio
 * @retval EVALRESP_OK on success
 */
int evalresp_cwd_to_cwd (evalresp_logger *log,
                         evalresp_options *options, evalresp_filter *filter);

/**
 * @public
 * @ingroup evalresp_public_low_level
 * @param[in] log logging structure
 * @param[in] options object to control flow of program
 * @param[in] channel the channel to print into the log
 * @brief print the channel information of the channel be processed
 * @retval EVALRESP_OK on success
 */
int evalresp_channel_to_log (evalresp_logger *log, evalresp_options const *const options, evalresp_channel *const channel);

#endif
