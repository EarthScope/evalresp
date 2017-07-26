
/* NOTE - this is here only to check for compilation errors so we don't have an
 * example that won't compile.
 *
 * Also, we need to use C++/C99 comments so we can cut+paste into a comment in the docs.
 */

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
