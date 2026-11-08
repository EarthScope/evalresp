# evalresp

[![CI](https://github.com/EarthScope/evalresp/actions/workflows/ci.yml/badge.svg)](https://github.com/EarthScope/evalresp/actions/workflows/ci.yml)

[![DOI](https://zenodo.org/badge/84248894.svg)](https://doi.org/10.5281/zenodo.24062696)

evalresp calculates the frequency response of seismic, pressure, temperature
and magnetic channels.  Given a station, channel, network and location, a date
and time, and a range of frequencies, it evaluates the channel's instrument
response and writes it as amplitude and phase, frequency/amplitude/phase, or
complex spectra.

Responses are read from FDSN StationXML files or legacy SEED RESP files (as written
by `rdseed`).

evalresp is a C program, a set of libraries and a C API, so the same
evaluation can be built into other software.

## What is included

| Item | Description |
| --- | --- |
| `evalresp` | Command-line program that evaluates responses |
| `xml2resp` | Converts StationXML to RESP |
| `libevalresp.a` | Library and C API |
| `libevalresp_log.a`, `libspline.a`, `libmxmlev.a` | Support libraries: logging, spline interpolation, and a bundled copy of [Mini-XML](https://www.msweet.org/mxml/) |

## Building

### Unix

A C compiler and GNU make (`gmake` on some systems) are all that is needed.

    make                                  # build programs and libraries
    make CFLAGS="-O0 -g"                  # build with other flags (default is -O2)
    make install INSTALL_DIR=/usr/local   # install bin/, lib/ and include/

`INSTALL_DIR` defaults to the build directory.  Set `DESTDIR` as well to stage
an install for packaging.  Other settings are in `Build.config`.  See
[INSTALL.unix](INSTALL.unix) for details.

### Windows

From a Visual Studio developer command prompt:

    nmake /F Makefile.nmake

See [INSTALL.win](INSTALL.win) for details.

## Using evalresp

    evalresp STA_LIST CHA_LIST YYYY DAY MIN_FREQ MAX_FREQ NFREQS [options]

For example, to evaluate the response of channel BH1 of station ANMO on day 100
of 2016 at 50 log-spaced frequencies from 0.01 to 10 Hz, using a StationXML file
from this repository:

    evalresp ANMO BH1 2016 100 0.01 10 50 -f tests/data/station-1.xml

The file holds this channel at two locations, so this writes `AMP.IU.ANMO.00.BH1`
and `PHASE.IU.ANMO.00.BH1`, and the same two files for location 10.  Each file
has a column of frequencies (Hz) and a column of amplitudes or phases (degrees):

    1.000000E-02 2.849763E+09
    1.151395E-02 3.077383E+09
    ...

The input format is detected from the file, or can be forced with `-x`.

### Legacy SEED RESP input

evalresp also reads SEED RESP files, as written by `rdseed`.  Using the same
options with a RESP file:

    evalresp ANMO BHZ 2010 1 0.001 10 100 -f tests/data/RESP.IU.ANMO.00.BHZ

`xml2resp` converts StationXML to RESP text:

    xml2resp -o RESP.IU.ANMO tests/data/station-1.xml

Without `-f`, the current directory and then the directory named by the
`SEEDRESP` environment variable are searched for RESP files.

### Selecting channels

Stations, channels and locations can be given as lists or with wildcards
(`-l '??'`, `-n '*'`), and every matching response is written to its own
files.

### Common options

| Option | Meaning |
| --- | --- |
| `-f file` | Input file, or a directory of legacy `RESP.NET.STA.LOC.CHA` files |
| `-u dis\|vel\|acc\|def` | Units of the response (default `vel`); `def` is required for responses that are not ground motion |
| `-r ap\|fap\|cs` | Output: amplitude/phase, frequency/amplitude/phase, or complex spectra |
| `-s log\|lin` | Frequency spacing (default `log`) |
| `-t HH:MM:SS` | Time of day, to choose between responses effective on the same day |
| `-n`, `-l` | Network and location codes |
| `-stage start [stop]` | Evaluate only these stages |
| `-stdio` | Read input from stdin and write output to stdout |
| `-v` | Verbose |

The [man page](doc/evalresp.1) (`man doc/evalresp.1`) lists every option and
describes how filters, decimation, List blockettes and non-linear responses
are handled.

## Using the library

Include `<evalresp/public_api.h>` and link with the libraries installed by
`make install`.  This example reads a StationXML file and writes amplitude
and phase files for one channel:

```c
#include <evalresp/public_api.h>

int
main (void)
{
  int status;
  evalresp_options *options = NULL;
  evalresp_filter *filter = NULL;

  if ((status = evalresp_new_options (NULL, &options)) ||
      (status = evalresp_new_filter (NULL, &filter)))
    return status;

  /* 50 log-spaced frequencies from 0.01 to 10 Hz */
  options->min_freq = 0.01;
  options->max_freq = 10;
  options->nfreq = 50;
  evalresp_set_filename (NULL, options, "station-1.xml");

  /* Evaluate on day 100 of 2016 */
  filter->datetime->year = 2016;
  filter->datetime->jday = 100;
  evalresp_add_sncl_text (NULL, filter, "IU", "ANMO", "00", "BH1");

  /* Reads the file, writes AMP.* and PHASE.* files in the current directory */
  status = evalresp_cwd_to_cwd (NULL, options, filter);

  evalresp_free_options (&options);
  evalresp_free_filter (&filter);
  return status;
}
```

Build it with:

    cc example.c -I$PREFIX/include -L$PREFIX/lib \
       -levalresp -levalresp_log -lspline -lmxmlev -lm

`evalresp_cwd_to_cwd` does everything in one call.  The lower-level functions
read channels, evaluate them and return the responses in memory; see
[highlevel.c](libsrc/evalresp/examples/highlevel.c) and
[lowlevel.c](libsrc/evalresp/examples/lowlevel.c) in `libsrc/evalresp/examples`.

The complete API reference is built from the source with Doxygen and LaTeX:

    cd doc
    make docs        # writes doc/evalresp.pdf
    make html        # writes doc/html/index.html (needs only Doxygen)

## Testing

    make check                              # Unix
    nmake /F Makefile.nmake check           # Windows

The tests need nothing beyond the C compiler.  They run on every push on
Linux (gcc and clang), macOS and Windows (MSVC).  See
[tests/README.md](tests/README.md) for running individual tests and adding new
ones.

## Changes, license and contributing

- Changes in each release: [ChangeLog](ChangeLog)
- Source, issues and pull requests: <https://github.com/EarthScope/evalresp>
- Citing: each release is archived on Zenodo with a DOI; the citation
  metadata is in [CITATION.cff](CITATION.cff).
- evalresp is distributed under the GNU Lesser General Public License,
  version 3: see [LICENSE](LICENSE).
