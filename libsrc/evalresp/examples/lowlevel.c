
/* NOTE - this is here only to check for compilation errors so we don't have an
 * example that won't compile.
 *
 * Also, we need to use C++/C99 comments so we can cut+paste into a comment in the docs.
 */

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
