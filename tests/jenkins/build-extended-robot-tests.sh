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
    ZIP="$2"
    URL="$3"
    pushd "$DIR" >> /dev/null
    if [ ! -f "$ZIP" ]; then
	echo "$ZIP does not exist"
	if [ -z ${ROBOT_ARCHIVE_CACHE+x} ]; then
	    echo "To avoid multiple downloads, place $ZIP in a cache directory and set"
	    echo "ROBOT_ARCHIVE_CACHE to the absolute file location (eg /var/data/$ARCHIVE)."
	elif [ ! -e "$ROBOT_ARCHIVE_CACHE/$ZIP" ]; then
	    echo "WARNING: $ZIP not found in $ROBOT_ARCHIVE_CACHE"
	fi
	if [ "$ROBOT_ARCHIVE_CACHE" ] && [ -e "$ROBOT_ARCHIVE_CACHE/$ZIP" ]; then
	    echo "Copying data from cache at $ROBOT_ARCHIVE_CACHE"
	    cp "$ROBOT_ARCHIVE_CACHE/$ZIP" "$ZIP"
	else
	    wget "$URL" || {
		echo "WARNING: Could not download $URL"
		echo "If this file was not optional, script may fail later"
	    }
	fi
    fi
    popd >> /dev/null
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
pushd tests/robot/data >> /dev/null
if [ -d extended ]; then
    echo "Wiping existing extended data"
    rm -fr extended
fi
mkdir -p extended
pushd extended >> /dev/null
echo "Extracting data from $EXTENDED_INPUT"
# note -j below so that everything is in main directory (no subdirs)
unzip -j "../$EXTENDED_INPUT" > /dev/null
if ! compgen -G "$PATTERN"; then
    echo "ERROR: Bad pattern '$PATTERN'"
    usage
fi
popd >> /dev/null
popd >> /dev/null


# copy and expand target data

EXTENDED_TARGET="RESP-targets-$YEAR-$DAY.zip"
download tests/robot/target "$EXTENDED_TARGET" "http://isti.com/~andrew/$EXTENDED_TARGET"
mkdir -p tests/robot/target/extended
pushd tests/robot/target/extended >> /dev/null
echo "Extracting data from $EXTENDED_TARGET"
unzip "../$EXTENDED_TARGET" > /dev/null || {
    echo "WARNING: No target data for $YEAR $DAY.  Tests will fail."
    echo "Use collect-extended-targets.sh to build data after initial run."
}
popd >> /dev/null


# generate tests (we cannot do this inside robot and have N tests for
# N files - it would be a single test, which makes reporting ugly)

RUN_DIR="tests/robot/all/extended/$YEAR/$DAY"
mkdir -p "$RUN_DIR"
pushd "$RUN_DIR" >> /dev/null
echo "Generating tests.  This can take some time."
tick=0
for path in `ls ../../../../data/extended/$PATTERN`; do
    file=`basename $path`
    (
	cat <<EOF

*** Settings ***

Library  Process
Library  Support


*** Test Cases ***

Automated call to evalresp
    Prepare  extended/$YEAR/$DAY/$file  extended  $file
    Run process  evalresp  *  *  $YEAR  $DAY  0.001  10  100  -f  $file
    Count and compare target files two float cols

EOF
    ) > "$file.robot"
    tick=$(( ( tick + 1 ) % 100 ))
    if [ $tick -eq 0 ]; then
	echo -n "."
    fi
done
echo
echo "Tests generated"
popd >> /dev/null

