#!/bin/bash -e

# previously (in jenkins directly)
# git clone git@github.com-evalresp:iris-edu/evalresp.git
# where github.com-evalresp is defined in ~/.ssh/config according to
# https://gist.github.com/jexchan/2351996

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

rm -fr install
mkdir m4  # needed by ancient aclocal on jenkins machine
autoreconf -fi
./configure -prefix `pwd`/install --enable-check
make

# this is where test output will be go
rm -fr tests/run
mkdir tests/run

# these will be run separately in jenkins, if needed
#make check
#make install
