#!/bin/bash -e

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

rm -fr install
autoreconf -fi
./configure -prefix `pwd`/install --enable-check
make

# this is where test output will be go
rm -fr tests/run
mkdir tests/run

# these will be run separately in jenkins, if needed
#make check
#make install
