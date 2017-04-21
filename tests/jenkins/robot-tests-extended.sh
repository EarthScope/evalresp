#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

if [ ! -d env ]; then
    echo "Run robot-tests-basic.sh first"
    exit 1
fi

PATTERN=${1:-*}
echo "Will generate tests for files matching the pattern '$PATTERN'"
echo "You can give this as an initial argument:"
echo "  $0 '$PATTERN'"


# copy and expand raw data

ARCHIVE="RESP-testset.zip"

pushd tests/robot/data
if [ ! -f "$ARCHIVE" ]; then
    echo "$ARCHIVE does not exist"
    if [ -z ${ROBOT_ARCHIVE_CACHE+x} ]; then
	echo "ROBOT_ARCHIVE_CACHE is unset so downloading data."
	echo "To avoid multiple downloads, place $ARCHIVE in a cache directory and set"
	echo "ROBOT_ARCHIVE_CACHE to the absolute file location (eg /var/data/$ARCHIVE)."
	wget "http://ds.iris.edu/files/staff/chad/$ARCHIVE"
    else
	echo "Copying data from cache at $ROBOT_ARCHIVE_CACHE"
	cp "$ROBOT_ARCHIVE_CACHE" "$ARCHIVE"
    fi
fi
if [ -d extended ]; then
    echo "Wiping existing extended data"
    rm -fr extended
fi
mkdir extended
pushd extended
# note -j below so that everything is in main directory (no subdirs)
unzip -j "../$ARCHIVE"
popd
popd


# generate tests (we cannot do this inside robot and have N tests for
# N files - it would be a single test, which makes reporting ugly)

pushd tests/robot/all
if [ -d extended ]; then
    echo "Wiping existing extended tests"
    rm -fr extended
fi
mkdir extended


exit 0

. ./env/bin/activate

# under jenkins, WORKSPACE is set automatically, but otherwise
# this script will be run there anyway...
pwd=`pwd`; WORKSPACE=${WORKSPACE:-$pwd}
TMP=${TMP:-/tmp}
PATH="${PATH}:${WORKSPACE}/install/bin"

cd tests/robot
rm -fr run/extended; mkdir run/extended
WORKSPACE=${WORKSPACE} robot --loglevel DEBUG all/extended
