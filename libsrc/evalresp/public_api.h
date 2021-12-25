
#ifndef EVALRESP_PUBLIC_API_H
#define EVALRESP_PUBLIC_API_H

#include <stdio.h>

#include "evalresp/public_channels.h"
#include "evalresp/public_responses.h"
#include "evalresp_log/log.h"

/**
 * @defgroup evalresp evalresp Application Programming Interface (API)
 * @ingroup iris
 * @brief The Application Programming Interface (API) for evalresp.  Most users will
 * want to call routines in the @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public evalresp Public Interface
 * @ingroup evalresp
 * @brief The evalresp Public Interface is the main API for people using this package as
 * a library.
 *
 * It contains sub-APIs that describe channel data, options that influence
 * how responses are evaluated, the responses themselves, and both low and high-level
 * routines for processing data.
 *
 * #### Example Using High Level Routine
 *
 * In this example we prepare the options and filter and then invoke the high level
 * interface.  Note that some error returns are ignored (with `(void)` casts) to keep
 * the example compact.  Also, we are using a `NULL` log (see @ref evalresp_log).
@verbatim
#include <evalresp/public_api.h>

int
highlevel ()
{
  int status;
  evalresp_options *options = NULL;
  evalresp_filter *filter = NULL;

  if ((status = evalresp_new_options (NULL, &options)))
  {
    printf ("Failed to create options");
  }
  else if ((status = evalresp_new_filter (NULL, &filter)))
  {
    printf ("Failed to create filter");
  }

  if (!status) // If we have no error, set options
  {
    // Log spaced from 0.1 to 10 Hz, 20 bins
    options->min_freq = 0.1;
    options->max_freq = 10;
    options->nfreq = 20;

    // Reading station.xml formatted data.
    options->station_xml = 1;
    (void)evalresp_set_filename (NULL, options, "mydata.xml");

    // Set date
    filter->datetime->year = 2017;
    filter->datetime->jday = 123;

    // Set SNCL to extract
    (void)evalresp_add_sncl_text (NULL, filter, "NET", "STA", "LOC", "CHN");
  }

  if (!status) // If we have no error, process data
  {
    // This will read mydata.xml from the current directory, extract the
    // SNCL NET.STA.LOC.CHN, evaluate the response at the give date, and
    // write AMP... and PHA... files in velocity units (the defaults).
    if ((status = evalresp_cwd_to_cwd (NULL, options, filter)))
    {
      printf ("Failed to process data");
    }
  }

  evalresp_free_options (&options);
  evalresp_free_filter (&filter);

  return status;
}
@endverbatim
 * #### Example Using Low Level Routines
 *
 * This example is similar to the above, but we implement all the steps ourselves in
 * the lower level interface.
@verbatim
#include <evalresp/public_api.h>

int
lowlevel ()
{
  int status;
  evalresp_options *options = NULL;
  evalresp_filter *filter = NULL;
  evalresp_channels *channels = NULL;
  evalresp_response *response = NULL;

  if ((status = evalresp_new_options (NULL, &options)))
  {
    printf ("Failed to create options");
  }
  else if ((status = evalresp_new_filter (NULL, &filter)))
  {
    printf ("Failed to create filter");
  }

  if (!status) // If we have no error, set options
  {
    // Log spaced from 0.1 to 10 Hz, 20 bins
    options->min_freq = 0.1;
    options->max_freq = 10;
    options->nfreq = 20;

    // Reading station.xml formatted data.
    options->station_xml = 1;
    // Filename is given in call to evalresp_filename_to_channels

    // Set date
    filter->datetime->year = 2017;
    filter->datetime->jday = 123;

    // Set SNCL to extract
    (void)evalresp_add_sncl_text (NULL, filter, "NET", "STA", "LOC", "CHN");
  }

  if (!status) // If we have no error, process data
  {
    // Read in the channel
    (void)evalresp_filename_to_channels (NULL, "mydata.xml", options, filter, &channels);
    if (channels->nchannels != 1)
    {
      printf ("Failed to read single channel");
    }
    else
    {
      // Evaluate the response
      (void)evalresp_channel_to_response (NULL, channels->channels[0], options, &response);
      // Write the AMP and PHA files
      (void)evalresp_response_to_file (NULL, response, options->unwrap_phase, evalresp_amplitude_file_format, "AMP.NET.STA.LOC.CHN");
      (void)evalresp_response_to_file (NULL, response, options->unwrap_phase, evalresp_phase_file_format, "PHA.NET.STA.LOC.CHN");
    }
  }

  evalresp_free_response (&response);
  evalresp_free_channels (&channels);
  evalresp_free_options (&options);
  evalresp_free_filter (&filter);

  return status;
}
@endverbatim
 */

/**
 * @defgroup evalresp_public_high_level evalresp Public High-Level Interface
 * @ingroup evalresp_public
 * @brief The evalresp Public High-Level Interface implements the entire functionality
 * of the command line tool.  It combines the routines in @ref evalresp_public_low_level
 * to read channels, evaluate their responses, and print the results.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_options evalresp Public Options Interface
 * @ingroup evalresp_public
 * @brief The evalresp Public Options Interface allows users to configure exactly how the
 * library selects, evaluates and writes data.
 *
 * This is done via three structures:
 * - @ref evalresp_options - Provides constraints on evaluation and output.
 * - @ref evalresp_filter - Selects which files, channels and dates are processed.
 * - @ref evalresp_datetime - Embedded within @ref evalresp_filter, but publicly
 *   visible so that members can be set directly during configuration
 *   (see the examples in @ref evalresp_public).
 *
 * For both @ref evalresp_options and @ref evalresp_filter, the user should create an instance,
 * set appropriate values, and then pass the instance to the appropriate routine(s).
 * Once no longer needed, instances should be freed.
 *
 * Simple numerical values can be set directly, but for other values, for parsing
 * strings, and for creating and freeing instances, evalresp provides a set of
 * utility functions.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level evalresp Public Low-Level Interface
 * @ingroup evalresp_public
 * @brief The evalresp Public Low-Level Interface provides library access to the basic
 * functionality provided by the library.  Sub-APIs allow the user user to read
 * (@ref evalresp_public_low_level_input) channels (@ref evalresp_public_low_level_channel),
 * evaluate (@ref evalresp_public_low_level_evaluation) responses
 * (@ref evalresp_public_low_level_response), and write (@ref evalresp_public_low_level_output)
 * results.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level_input evalresp Public Low-Level Input Interface
 * @ingroup evalresp_public_low_level
 * @brief The evalresp Public Low-Level Input Interface reads channels
 * (@ref evalresp_public_low_level_channel) from RSEED and station.xml data.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level_evaluation evalresp Public Low-Level Evaluation Interface
 * @ingroup evalresp_public_low_level
 * @brief The evalresp Public Low-Level Evaluation Interface calculates responses
 * (@ref evalresp_public_low_level_response) from channels (@ref evalresp_public_low_level_channel).
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level_output evalresp Public Low-Level Output Interface
 * @ingroup evalresp_public_low_level
 * @brief The evalresp Public Low-Level Output Interface writes responses
 * (@ref evalresp_public_low_level_response) to strings and files.  It also includes a utility
 * routine that displays a channel (@ref evalresp_public_low_level_channel) to a log file.
 * For routines that output multiple files, infer file names, etc, see
 * @ref evalresp_public_high_level.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level_channel evalresp Public Low-Level Channel Interface
 * @ingroup evalresp_public_low_level
 * @brief The evalresp Public Channel Interface describes the contents of an RSEED
 * file for a particular channel.  Instances of these structures are generated by
 * routines in the @ref evalresp_public_low_level_input.
 *
 * For examples see @ref evalresp_public.
 */

/**
 * @defgroup evalresp_public_low_level_response evalresp Public Low-Level Response Interface
 * @ingroup evalresp_public_low_level
 * @brief The evalresp Public Response Interface describes an evaluated response
 * (ie complex values at different frequencies).  Instances of these structures are
 * generated by evaluating channels via the @ref evalresp_public_low_level_evaluation.
 * They can also be passed to routines in the @ref evalresp_public_low_level_output
 * for writing.
 *
 * For examples see @ref evalresp_public.
 *
 * The complex response values are stored in the `rvec` member; corresponding frequencies
 * are stored in the `freqs` member.
 */

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
  EVALRESP_MEM,    /**< Memory error. */
  EVALRESP_IO,     /**< IO Error. */
  EVALRESP_INP,    /**< Bad user input. */
  EVALRESP_ERR,    /**< Internal (coding) error. */
  EVALRESP_PAR,    /**< Parsing error in file. */
  EVALRESP_EOF,    /**< End of input. */
  EVALRESP_VAL     /**< Validation error. */
};

// --- filters

/**
 * @private
 * @ingroup evalresp_public_options
 * @brief Structure used for time comparisons.
 */
typedef struct
{
  int year;  /**< Year. */
  int jday;  /**< Julian day of year. */
  int hour;  /**< Hour. */
  int min;   /**< Minutes. */
  float sec; /**< Seconds. */
} evalresp_datetime;

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Structure used for filtering input data by SNCL and date/time.
 */
typedef struct
{
  struct evalresp_sncls_s *sncls; /**< The SNCLs to match (if set, one must match).  Values can added using @ref evalresp_add_sncl_text and @ref evalresp_add_sncl_all. */
  evalresp_datetime *datetime;    /**< The datetime to use (if set). */
} evalresp_filter;

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[out] filter pointer to be allocated and initialized
 * @brief Allocate and initialize an evalresp_filter with no date or SNCLs.
 * @retval EVALRESP_OK on success
 */
int evalresp_new_filter (evalresp_logger *log, evalresp_filter **filter);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the year
 * @param[in] year
 * @brief Set the year in an evalresp_filter from a string.  Input channels will only be read and
 * evaluated for the year given.
 * @retval EVALRESP_OK on success
 */
int evalresp_set_year (evalresp_logger *log, evalresp_filter *filter, const char *year);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the day of theyear
 * @param[in] julian_day the day of the year
 * @brief Set the Julian day in an evalresp_filter from a string.  Input channels will only be
 * read and evaluated for the day given.
 * @retval EVALRESP_OK on success
 */
int evalresp_set_julian_day (evalresp_logger *log, evalresp_filter *filter, const char *julian_day);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] filter evalresp_filter pointer to set the time
 * @param[in] time
 * @brief Set the time in an evalresp_filter (HH:MM:SS format).  Input channels will only be
 * read and evaluated for the time given.
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
 * @brief Add a SNCL to an evalresp_filter.  If any SNCLs are given then input channels will
 * only be read and evaluated if they match a SNCL.
 * @retval EVALRESP_OK on success
 */
int evalresp_add_sncl_text (evalresp_logger *log, evalresp_filter *filter,
                            const char *net, const char *sta, const char *locid, const char *chan);

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
 * @brief Add SNCL(s) to an evalresp_filter, accepting S, C and L as lists.
 * If any SNCLs are given then input channels will only be read and evaluated if they match a SNCL.
 * @retval EVALRESP_OK on success
 */
int evalresp_add_sncl_all (evalresp_logger *log, evalresp_filter *filter,
                           const char *sta, const char *net, const char *chan, const char *locid);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] filter the filter to free
 * @brief Free an evalresp_filter.
 * @retval EVALRESP_OK on success
 */
void evalresp_free_filter (evalresp_filter **filter);

// --- options

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Enumeration of output formats (FAP, complex, etc - can require multiple files).
 */
typedef enum {
  evalresp_ap_output_format,     /**< Two files, AMP and PHASE. */
  evalresp_fap_output_format,    /**< One file, FAP. */
  evalresp_complex_output_format /**< One file, COMPLEX. */
} evalresp_output_format;

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Enumeration of response units (acceleration, velocity or displacement).
 */
typedef enum {
  evalresp_file_unit,         /**< Use the units in the input file. */
  evalresp_displacement_unit, /**< Displacement units. */
  evalresp_velocity_unit,     /**< Velocity units (the default). */
  evalresp_acceleration_unit  /**< Acceleration units. */
} evalresp_unit;

#define EVALRESP_ALL_STAGES -1 /**< Default for start and stop stage. */
#define EVALRESP_NO_FREQ -1    /**< Default for frequency limits. */

/**
 * @public
 * @ingroup evalresp_public_options
 * @brief Structure containing the options that affect how responses are evaluated and
 * printed.
 */
typedef struct
{
  char *filename;                /**< Input file (if omitted, some routines will scan the current directory for files). */
  double b62_x;                  /**< X value for evaluating blockette 62. */
  double min_freq;               /**< Minimum frequency to evaluate. */
  double max_freq;               /**< Maximum frequency to evaluate. */
  int nfreq;                     /**< Number of frequencies (ie "bins") to evaluate. */
  int lin_freq;                  /**< Linear frequency steps (logarithmic by default)? */
  int start_stage;               /**< First stage to evaluate (all be default). */
  int stop_stage;                /**< Last stage to evaluate (all by default). */
  int use_estimated_delay;       /**< Use the estimated delay (ignore by default)? */
  int unwrap_phase;              /**< Unwrap phase (leave unwrapped by default)? */
  int b55_interpolate;           /**< Interpolate blockette 55 to match min_freq, max_freq, etc (use frequencies given in the blockette, overriding options here, by default)? */
  int use_total_sensitivity;     /**< Use the total sensitivity (ignore by default)? */
  int use_stdio;                 /**< Read from stdin / write to stdout (use files by default)? */
  int station_xml;               /**< Expect StationXML input, use 0 to autodetect (SEED RESP by default)? */
  evalresp_output_format format; /**< Output format (AMP and PHA by default). */
  evalresp_unit unit;            /**< Output unit (displacement by default). */
  int verbose;                   /**< Verbose output? */
} evalresp_options;

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[out] options pointer to options that is allocated
 * @brief Allocate and initialize an evalresp_options with default values.
 * @retval EVALRESP_OK on success
 */
int evalresp_new_options (evalresp_logger *log, evalresp_options **options);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in[ filename filename to set in options
 * @brief Add a filename to the options.  Data will be read from this file.
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
 * @brief Set the minimum and maximum frequency, and the number of bins, from strings.
 * Alternatively the numerical values can be set directly.
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
 * @brief Set the output format from a string.  Alternatively the format can be set directly
 * from @ref evalresp_output_format.
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
 * @brief Set the output units from a string.  Alternatively the units can be set directly
 * from @ref evalresp_unit.
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
 * @brief Set the frequency spacing option from a string.  Alternatively the value can be set
 * directly (1 for linear, 0 for log).
 * @retval EVALRESP_OK on success
 */
int evalresp_set_spacing (evalresp_logger *log, evalresp_options *options,
                          const char *spacing);

// start and stop are separate because it simplifies calling from main routine

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] log logging structure
 * @param[in] options evalresp_option in which the value is to be added
 * @param[in] stage starting stage as a string, "-1" means start at first available stage
 * @brief Set the starting stage option from a string.  Alternatively the numerical value can
 * be set directly.
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
 * @brief Set the stopping stage option from a string.  Alternatively the numerical value can
 * be set directly.
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
 * @brief Set the x value option to be used for blockette 62 from a string.  Alternatively the
 * numerical value can be set directly.
 * @retval EVALRESP_OK on success
 */
int evalresp_set_b62_x (evalresp_logger *log, evalresp_options *options,
                        const char *b62_x);

/**
 * @public
 * @ingroup evalresp_public_options
 * @param[in] options the options that need to be deallocated
 * @brief Free an evalresp_options.
 */
void evalresp_free_options (evalresp_options **options);

// --- low level input

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] seed_or_xml input string
 * @param[in] options file format and unit options are used
 * @param[in] filter filter to use when getting the channels
 * @param[out] channels collection of channels that gets allocated and returned
 * @brief Read channels (@ref evalresp_public_low_level_channel) from a string.  Options (including
 * RSEED or station.xml format) are set via @ref evalresp_options and the channels
 * read are selected via @ref evalresp_filter.
 * @retval EVALRESP_OK on success
 */
int evalresp_char_to_channels (evalresp_logger *log, const char *seed_or_xml,
                               evalresp_options const *const options,
                               const evalresp_filter *filter, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] file input stream
 * @param[in] options file format and unit options are used
 * @param[in] filter filter to use when getting the channels
 * @param[out] channels collection of channels that gets allocated and returned
 * @brief Read channels (@ref evalresp_public_low_level_channel) from a file (stream).  Options (including
 * RSEED or station.xml format) are set via @ref evalresp_options and the channels
 * read are selected via @ref evalresp_filter.
 * @retval EVALRESP_OK on success
 */
int evalresp_file_to_channels (evalresp_logger *log, FILE *file,
                               evalresp_options const *const options,
                               const evalresp_filter *filter, evalresp_channels **channels);

/**
 * @public
 * @ingroup evalresp_public_low_level_input
 * @param[in] log logging structure
 * @param[in] filename input filename
 * @param[in] options file format and unit options are used
 * @param[in] filter filter to use when getting the channels
 * @param[out] channels collection of channels that gets allocated and returned
 * @brief Read channels (@ref evalresp_public_low_level_channel) from a (named) file.  Options (including
 * RSEED or station.xml format) are set via @ref evalresp_options and the channels
 * read are selected via @ref evalresp_filter.
 * @retval EVALRESP_OK on success
 */
int evalresp_filename_to_channels (evalresp_logger *log, const char *filename, evalresp_options const *const options,
                                   const evalresp_filter *filter, evalresp_channels **channels);

// --- low level evaluation

/**
 * @public
 * @ingroup evalresp_public_low_level_evaluation
 * @param[in] log logging structure
 * @param[in] channel channel object to be converted into a response
 * @param[in] options options control how responses are evaluated
 * @param[out] response an allocated response created from channel
 * @brief Evaluate a channel (@ref evalresp_public_low_level_channel) to a response
 * (@ref evalresp_public_low_level_response).
 * @retval EVALRESP_OK on success
 */
int evalresp_channel_to_response (evalresp_logger *log, evalresp_channel *channel,
                                  evalresp_options *options, evalresp_response **response);

/**
 * @public
 * @ingroup evalresp_public_low_level_evaluation
 * @param[in] log logging structure
 * @param[in] channels channels to be converted into responses
 * @param[in] options options control how responses are evaluated
 * @param[in,out] responses a pointer to an @ref evalresp_responses object (a collection of responses);
 * if *responses == NULL then it will be allocated
 * @brief All the channels are evaluated and the responses added to the collection.
 * @retval EVALRESP_OK on success
 */
int evalresp_channels_to_responses (evalresp_logger *log, evalresp_channels *channels,
                                    evalresp_options *options, evalresp_responses **responses);

// --- low level output

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @brief Enumeration of output file formats (for a single file).
 */
typedef enum {
  evalresp_fap_file_format,       /**< A file containing frequency, amplitude and phase columns. */
  evalresp_amplitude_file_format, /**< A file containing frequency and amplitude columns. */
  evalresp_phase_file_format,     /**< A file containing frequency and phase columns. */
  evalresp_complex_file_format    /**< A file containing frequency and complex response columns. */
} evalresp_file_format;

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] unwrap whether to unwrap phase
 * @param[in] format the output format to generate
 * @param[out] output pointer to the char * that the response will be printed into
 * @brief Format an @ref evalresp_response to a string.
 * @retval EVALRESP_OK on success
 * @post output will be allocated on success and must be free'd by other functions
 */
int evalresp_response_to_char (evalresp_logger *log, const evalresp_response *response,
                               int unwrap, evalresp_file_format format, char **output);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] unwrap whether to unwrap phase
 * @param[in] format the output format to generate
 * @param[out] file stream that the response will be printed into
 * @brief Format an @ref evalresp_response to a file stream.
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_stream (evalresp_logger *log, const evalresp_response *response,
                                 int unwrap, evalresp_file_format format, FILE *const file);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] response the response object created by evalresp
 * @param[in] unwrap whether to unwrap phase
 * @param[in] format the output format to generate
 * @param[out] filename name of the file that the response will be printed into
 * @brief Format an @ref evalresp_response to a named file.
 * @retval EVALRESP_OK on success
 */
int evalresp_response_to_file (evalresp_logger *log, const evalresp_response *response,
                               int unwrap, evalresp_file_format format, const char *filename);

/**
 * @public
 * @ingroup evalresp_public_low_level_output
 * @param[in] log logging structure
 * @param[in] options units, start and stop stages are used; some other options are logged
 * @param[in] channel the channel to print into the log
 * @brief Display information on the channel be processed
 * @retval EVALRESP_OK on success
 */
int evalresp_channel_to_log (evalresp_logger *log, evalresp_options const *const options,
                             evalresp_channel *const channel);

// --- high level

/**
 * @public
 * @ingroup evalresp_public_high_level
 * @param[in] log logging structure
 * @param[in] options controls input, evaluation and output details (see @ref evalresp_public_options)
 * @param[in] filter selects files and channels based on SNCLs and datetime (see @ref evalresp_public_options)
 * @brief Reads files from the current directory (although stdin or a named file can be specified
 * in the options); extracts the channels and evaluates the responses; writes the responses to files
 * in the current directory using names inferred from the output options (although stdout can also
 * be used if specified in the options).
 * @retval EVALRESP_OK on success
 */
int evalresp_cwd_to_cwd (evalresp_logger *log,
                         evalresp_options *options, evalresp_filter *filter);

#endif
