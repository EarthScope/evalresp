
This directory contains example code that calls evalresp from Fortran.
The code has changed significantly since the previous release.  These
changes were made to simplify the maintenance of the code (by making
the called routine more "C like") and to bring the Fortran code
up-to-date with Fortran 95 practices.

I apologise for any extra work this may imply when adapting to this
release, but, as far as I can tell, the existing Fortran code had no
worked correctly for some time (in particular, new parameters seemed
to have been added after the implicit string length arguments used
with f77).

Hopefully the example is sufficiently detailed to explain the new
approach.

Andrew Cooke (a.cooke@isti.com), release 4.0.0

Note: To run the example, edit and then execute run.sh
