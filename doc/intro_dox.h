/**

@mainpage Introduction
 @anchor introduction
  <b>evalresp</b> is a program, an application programming interface
  (API) and a set of libraries in C that calculate the complex spectral
  response, or the frequency-amplitude-phase response, of a station or
  set of stations, channel or channels, and network, for a specified date,
  time and set of frequencies.  Responses are read from FDSN StationXML,
  as produced by the FDSN station web service, or from the legacy SEED
  ASCII response (RESP) files produced by the program rdseed.

@section intro_audience Audience
The target audience for this document is the evalresp software
developers, software integrators that plan to utilize the evalresp API
in their projects, project management, and curious users.

@section intro_start Getting started
Programs that use evalresp include <tt>\<evalresp/public_api.h\></tt> and link
with the libraries installed by <tt>make install</tt>, in this order:
<tt>-levalresp -levalresp_log -lspline -lmxmlev -lm</tt>.

The API is described in @ref evalresp_public.  Its high-level interface
(@ref evalresp_public_high_level) does everything the command-line program does
in one call.  Its low-level interface (@ref evalresp_public_low_level) reads
channels, evaluates them and writes them in separate steps, so that a program
can keep responses in memory.  Programs written against the earlier
interface can keep using @ref evalresp_public_compat.

The private interface (@ref evalresp_private) is documented for developers
of evalresp itself and may change without notice.

The command-line programs are described in the evalresp(1) and xml2resp(1)
manual pages.

*/
