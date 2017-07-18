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

//int curr_seq_no;

/* and set a global variable to contain the environment for the setjmp/longjmp
 combination for error handling */
//jmp_buf jump_buffer;

//char myLabel[20];
