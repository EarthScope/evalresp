#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*===================================================================
Name:   evresp_1
Purpose:
        FORTRAN callable interface to the evresp routine (below)
Reference:
        SEED. Standard for the Exchange of Earthquake Data
        Reference Manual
        SEED Format Version 2.3 or later
        ??? 1995
Author:    Thomas J. McSweeney, Andrew Cooke

Usage (from FORTRAN):
        See tests/fortran/evresp.f

Notes:
        This routine was updated in release 4.0.0 to support Fortran
        95.  Previous versions were clearly broken (parameters had
        been added with no respect to the implicit lengths from
        Fortran character arrays), so backwards compatibility is not
        provided.

        Given the cleaner interface supported by Fortran 95, this
        routine can also be called from C.

        Whereas the other function returns a linked list of responses
        (one for each response that matched the user's request), this
        routine returns the response for one (1)
        station-channel-network for one (1) effective time.  If more
        than one match is found for a given
        station-channel-network-time, an error condition is raised
        (and a value of -1 is returned to the calling routine to
        indicate failure).  Likewise, a value of 1 is returned if no
        match is found for the given station-channel-network-time.  If
        a unique match is found, a value of 0 is returned to the
        calling routine

 *=================================================================*/
/*
    8/28/2001 -- [ET]  Moved several variable definitions from 'evresp.h'
                       into this file.
   10/19/2005 -- [ET]  Added 'evresp_itp()' function with support for
                       List blockette interpolation; made 'evresp()'
                       call 'evresp_itp()' function with default
                       values for List blockette interpolation parameters.
    2/13/2006 -- [ET]  Moved 'use_delay()' function from 'evalresp.c'
                       to 'evresp.c'; modified to close input file
                       when a single response file is specified.
    3/28/2006 -- [ET]  Added "free(freqs_orig)" at end of 'evresp_itp()'
                       function; added "free_matched_files(output_files)"
                       in 'evresp_itp()' function.
    8/21/2006 -- [IGD] Version 3.2.36: Added support for TESLA units.
    8/23/2006 -- [ET]  Improved detection of pressure ("PA") input units
                       for in/out units check.
   10/16/2006 -- [ET]  Modified to free array allocated in 'evresp_itp()'
                       function.
   02/27/2007 -- [IGD] Added return (#ifdef LIB_MODE) if the input file is not
                       found
   2015-05-04 -- [AC]  Simplifed / fixed for fortran 95
*/

#include <stdlib.h>
#include <string.h>

#include "evalresp/public.h"
#include "evalresp/stationxml2resp/wrappers.h"
#include "evalresp_log/log.h"
#include <evalresp/private.h>

/* define a global flag to use if using "default" units */
int def_units_flag;

/* define a pointer to a channel structure to use in determining the input and
 output units if using "default" units and for use in error output*/
evalresp_channel *GblChanPtr;
float unitScaleFact;

/* define global variables for use in printing error messages */
char *curr_file;
int curr_seq_no;

/* and set a global variable to contain the environment for the setjmp/longjmp
 combination for error handling */
jmp_buf jump_buffer;

char myLabel[20];

int
evresp_1 (char *sta, char *cha, char *net, char *locid, char *datime,
          char *units, char *file, double *freqs, int nfreqs, double *resp,
          char *rtype, char *verbose, int start_stage, int stop_stage,
          int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
          int xml_flag)
{
  evalresp_response *first = (evalresp_response *)NULL;
  int i, j;
  evalresp_log_t *log = NULL;

  // some eyeball checks to make sure fortran is passing things ok
  // printf("freqs: %f-%f\n", freqs[0], freqs[nfreqs-1]);
  // printf("x_for_b62: %f\n", x_for_b62);

  first = evresp (sta, cha, net, locid, datime, units, file, freqs, nfreqs,
                  rtype, verbose, start_stage, stop_stage, stdio_flag, useTotalSensitivityFlag,
                  x_for_b62, xml_flag, log);

  /* check the output.  If no response found, return 1, else if more than one response
     found, return -1 */

  if (first == (evalresp_response *)NULL)
  {
    return (1);
  }
  else if (first->next != (evalresp_response *)NULL)
  {
    evalresp_free_response (first);
    return (-1);
  }

  /* if only one response found, convert from complex output vector into multiplexed
     real output for FORTRAN (real1, imag1, real2, imag2, ..., realN, imagN) */

  for (i = 0, j = 0; i < nfreqs; i++)
  {
    resp[j++] = (float)first->rvec[i].real;
    resp[j++] = (float)first->rvec[i].imag;
  }

  /* free up dynamically allocated space */

  evalresp_free_response (first);

  /* and return to FORTRAN program */

  return (0);
}

/* 2/6/2006 -- [ET]  Moved from 'evalresp.c' to 'evresp.c' */
int
use_estimated_delay (int flag)
{
  /* WE USE THOSE WEIRD magic numbers here because
     * there is a chance that use_delay_flag is not
     * defined: in user program which uses evresp()
     * when use_estimated_delay() is not used before evresp().
     */
  int magic_use_delay = 35443647;
  int magic_dont_use_delay = -90934324;
  static int use_delay_flag = FALSE;
  if (TRUE == flag)
    use_delay_flag = magic_use_delay;
  if (FALSE == flag)
    use_delay_flag = magic_dont_use_delay;

  if (use_delay_flag == magic_use_delay)
    return TRUE;
  return FALSE;
}

/*===================================================================

 Name:      evresp

 Purpose:
        Extract channel response parameters from either ASCII files
        produced by rdseed -r ("response" file) or rdseed -d
        ("sta-cha" files) and calculate the complex response.

 Reference:
        SEED. Standard for the Exchange of Earthquake Data Reference
        Manual SEED Format Version 2.3 or later ??? 1995

 Author:
        Thomas J. McSweeney

 Modifications:
        Ilya Dricker (i.dricker@isti.com) IGD for versions of evalresp 3.2.17

 Notes:

         ???. Version 3.0

        - modified to parse "new" rdseed RESP file output (includes a
        new field that contains the blockette and field numbers for
        each of the items in the RESP file)

        - is a very substantial change over the previous releases of
        evresp.  The code has been completely rewritten from the
        original form authored by Jean-Francios Fels to support
        several new features.  among them are:

        (a) a "new" RESP file format that contains the blockette and
        field numbers as prefixes to each line.  This allows for quick
        determination of whether or not the program is parsing the
        correct information without relying on searching for
        non-standardized character strings in the RESP file

        (b) support for the blockette [61] responses

        (c) support for the response-reference style responses (i.e.
        a blockette [60] followed by a series of blockette [41] or
        blockette [43] through blockette [48] responses)

        - the code has been rewritten so that the calculations are all
        confined to this function and the functions that it calls.
        All the user has to do us supply the appropriate control
        parameters to this function

        - the parsing has been entirely reworked so that each
        blockette style is parsed in a seperate.  This should make the
        code easier to maintain and allow for changes in the output
        from RDSEED (either in number of fields on a line or in which
        fields are output from a given blockette)

        - the code has been converted to ANSI standard C, rather than
          K&R style C

 Thomas J. McSweeney:  tjm@iris.washington.edu

 *=================================================================*/

/* IGD 08/21/06 Added Tesla */


