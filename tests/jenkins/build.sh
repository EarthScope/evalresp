#!/bin/bash -e

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

rm -fr install
autoreconf -fi
./configure -prefix `pwd`/install --enable-check
make

# these will be run separately in jenkins, if needed
#make check
#make install
