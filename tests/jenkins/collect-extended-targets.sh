#!/bin/bash -e

NAME=$0

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

if [ $# -ne 2 ]; then
    echo
    echo "This script collects the extended results for a given year and day"
    echo "  $NAME year day"
    echo "Where"
    echo "  year is a year, eg 2017"
    echo "  day is a Julian day, eg 182"
    echo
    exit 1
fi

YEAR=$1
DAY=$2
RUN_DIR="tests/robot/run/extended/$YEAR/$DAY"

if [ ! -d "$RUN_DIR" ]; then
    echo "ERROR: $RUN_DIR does not exist.  Run tests first."
    exit 1
fi

# delete input files as not wanted in zip
find "$RUN_DIR" -type l -delete

# build zip
ZIPFILE="RESP-targets-$YEAR-$DAY.zip"
pushd tests/robot/run/extended
zip -r "$ZIPFILE" "$YEAR/$DAY"
mv "$ZIPFILE" ../../target
popd
