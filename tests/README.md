# Tests

One test program, `run-tests`, covers the library and the `evalresp` and
`xml2resp` programs.  It is plain C and needs nothing beyond the compiler
that builds evalresp: no test library, no scripting language, no shell
scripts.

## Running

From the top of the source tree:

    make check                              # Unix (GNU make)
    nmake /F Makefile.nmake check           # Windows (Visual Studio)

This builds evalresp, then the tests, then runs them.  The exit status is
non-zero if any test fails.

To run by hand (from this directory, after building evalresp):

    make run-tests
    ./run-tests                       # everything
    ./run-tests cli.args              # tests whose name contains "cli.args"
    ./run-tests -v input spline       # several patterns; -v names each test first
    ./run-tests --list                # list the test names

Test names are `suite.test`.  The options `--data`, `--expected`, `--bin` and
`--work` select the fixture directory, expected-output directory, directory
holding the `evalresp` programs and scratch directory; the defaults are
`data`, `expected`, `../src` and `work`, relative to the current directory.

Every test runs in its own empty directory, `work/<suite>/<test>/`, and
whatever it wrote is left there for inspection.  `make clean` removes it.

## Layout

    harness/        test runner, assertions, file comparison, OS-specific code
    suites.c        the list of suites
    unit/           tests that call the library directly
    unit/legacy/    the original file-based RESP parser, kept so that
                    unit/test_legacy.c can check the current parser against it
    cli/test_cli.c  runs evalresp and xml2resp and checks their output
    data/           input files (RESP, StationXML, reference output)
    expected/cli/   what each command-line case must produce

## Writing a unit test

Add a function to a file in `unit/` and list it in that file's table:

    static void
    test_something (void)
    {
      ER_CHECK (1 + 1 == 2);
      ER_CHECK_MSG (x > 0, "x is %d", x);
      ER_CHECK_NEAR (value, 1.0, 1e-6);
      /* er_data_path ("station-1.xml") is an absolute path in data/ */
    }

    const er_test unit_example_tests[] = {
        {"something", test_something, NULL},
        {NULL, NULL, NULL},
    };

A failed check ends that test only.  For a new file, add it to `UNIT` in
`Makefile` and `Makefile.nmake`, and its table to `suites.c`.

Tests must not depend on the directory they are started from (they run in
a scratch directory), must not write outside it, and must use only
standard C or the helpers in `harness/platform.h`, so that they build on
every platform.  Library-internal headers may be used.

## Adding a command-line case

Add a row to `cases[]` in `cli/test_cli.c`: the program, its arguments, the
input file to copy from `data/`, and the directory under `expected/cli/` that
holds the files the command must produce.  The files left in the scratch
directory (other than the input) must be exactly the expected files.  Compared
by name:

* `AMP.*` and `PHASE.*` as two columns of numbers, `FAP.*` and `SPECTRA.*` as
  three; numbers are compared to a relative tolerance of 1e-5 (the row can
  set `.tol`) and any other line must match exactly
* `stdout` and `stderr` as text, with the date before the first `[` removed
  from each `stderr` line and the version number in the banner masked
* anything else (`RESP`, ...) as text

Line endings are ignored.  The exit status is checked as well (`.rc`).

To create the expected files for a new case, run it once, check the output
by hand, and copy the files from `work/cli/<case>/` into `expected/cli/...`.

## Data

See `data/README.rst` for the origin of `RESP.Z.CGV..HYZ`.  Do not let
version control convert line endings in `data/`: `test_input.c` relies on
the exact bytes of the CRLF files (see `.gitattributes`).
