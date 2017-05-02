#!/bin/bash -e


NAME=$0

function usage {
    echo
    echo "This script generates tests for the given year, day and pattern:"
    echo "  $NAME year day [pattern]"
    echo "Where"
    echo "  year is a year, eg 2017"
    echo "  day is a Julian day, eg 182"
    echo "  pattern is an optional pattern that matches the input file, eg *"
    echo "Multiple dates can be tested at once (re-run this script),"
    echo "but the corresponding results must exist (see extract-targets.sh)"
    echo
    exit 1
}

function download {
    DIR="$1"
    TARBALL="$2"
    URL="$3"
    pushd "$DIR"
    if [ ! -f "$TARBALL" ]; then
	echo "$TARBALL does not exist"
	if [ -z ${ROBOT_ARCHIVE_CACHE+x} ]; then
	    echo "ROBOT_ARCHIVE_CACHE is unset so downloading data."
	    echo "To avoid multiple downloads, place $TARBALL in a cache directory and set"
	    echo "ROBOT_ARCHIVE_CACHE to the absolute file location (eg /var/data/$ARCHIVE)."
	    wget "$URL"
	else
	    echo "Copying data from cache at $ROBOT_ARCHIVE_CACHE"
	    cp "$ROBOT_ARCHIVE_CACHE/$ARCHIVE" "$ARCHIVE"
	fi
    fi
    popd
}


# read and store inputs

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    usage
fi

if [ $# -lt 2 ]; then
    usage
fi

YEAR=$1
DAY=$2
PATTERN=${3:-*}
DATE="$YEAR-$DAY"
echo
echo "Will generate tests for $YEAR $DAY using files matching '$PATTERN'"
echo "  $0 $YEAR $DAY '$PATTERN'"
echo


# copy and expand input data

EXTENDED_INPUT="RESP-testset.zip"
download tests/robot/data "$EXTENDED_INPUT" "http://ds.iris.edu/files/staff/chad/$EXTENDED_INPUT"
pushd tests/robot/data
if [ -d extended ]; then
    echo "Wiping existing extended data"
    rm -fr extended
fi
mkdir -p extended
pushd extended
echo "Extracting data from $EXTENDED_INPUT"
# note -j below so that everything is in main directory (no subdirs)
unzip -j "../$EXTENDED_INPUT" > /dev/null
popd
popd


# copy and expand target data

EXTENDED_TARGET="RESP-targets-$YEAR-$DAY.zip"
download tests/robot/target "$EXTENDED_TARGET" "http://isti.com/~andrew/$EXTENDED_TARGET"
TARGET_DIR="tests/robot/target/$YEAR/$DAY"
mkdir -p "$TARGET_DIR"
pushd "$TARGET_DIR"
echo "Extracting data from $EXTENDED_TARGET"
unzip "../../$EXTENDED_TARGET" > /dev/null
popd


# generate tests (we cannot do this inside robot and have N tests for
# N files - it would be a single test, which makes reporting ugly)

pushd tests/robot/all
mkdir -p extended
pushd extended
for path in `ls ../../data/extended/$PATTERN`; do
    file=`basename $path`
    (
	cat <<EOF

*** Settings ***

Library  Process
Library  Support


*** Test Cases ***

Automated call to evalresp
    Prepare  extended/$file  $file  extended
    Run process  evalresp  *  *  $YEAR  $DAY  0.001  10  100  -f  $file
    Check number of files  3

EOF
    ) > "$file.robot"
    echo "$path -> $file"
done
popd
popd
