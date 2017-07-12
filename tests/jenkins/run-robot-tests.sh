#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

tests/jenkins/install-robot.sh
. ./env/bin/activate

# under jenkins, WORKSPACE is set automatically, but otherwise
# this script will be run there anyway...

pwd=`pwd`; WORKSPACE=${WORKSPACE:-$pwd}
TMP=${TMP:-/tmp}

# this will use the current evalresp
BUILD=${WORKSPACE}
# this will use an alternative evalresp for regression testing
#BUILD=/home/ssi/evalresp-trunk

export PATH="${BUILD}/install/bin:${PATH}"
export LD_LIBRARY_PATH="${BUILD}/install/lib"
which evalresp

cd tests/robot
mkdir -p run
rm -fr run/*

# this will run all tests
WORKSPACE=${WORKSPACE} robot --loglevel DEBUG all

# this will run a single suite (all tests in the given file)
#WORKSPACE=${WORKSPACE} robot --loglevel DEBUG all/extended/2017/1/RESP.CI.BFS..ACE.robot
