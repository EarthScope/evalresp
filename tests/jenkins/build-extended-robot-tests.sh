#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

PATTERN=${1:-*}
echo
echo "Will generate tests for files matching the pattern '$PATTERN'"
echo "You can give this as an initial argument:"
echo "  $0 '$PATTERN'"
echo

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
echo "Extracting data from $ARCHIVE"
# note -j below so that everything is in main directory (no subdirs)
unzip -j "../$ARCHIVE" > /dev/null
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
    Run process  evalresp  *  *  2017  96  0.001  10  100  -f  $file
    Check number of files  3

EOF
    ) > "$file.robot"
    echo "$path -> $file"
done
popd
popd
