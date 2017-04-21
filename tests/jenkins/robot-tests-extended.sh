#!/bin/bash -ex

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

if [ ! -d env ]; then
    echo "run robot-tests-basic.sh first"
    exit 1
fi

. ./env/bin/activate

# under jenkins, WORKSPACE is set automatically, but otherwise
# this script will be run there anyway...
pwd=`pwd`; WORKSPACE=${WORKSPACE:-$pwd}
TMP=${TMP:-/tmp}
PATH="${PATH}:${WORKSPACE}/install/bin"

cd tests/robot
rm -fr run/extended; mkdir run/extended
WORKSPACE=${WORKSPACE} robot --loglevel DEBUG all/extended
