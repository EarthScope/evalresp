#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi


# install robot in a virtualenv
# this directory is always removed on jenkins (if configured correctly)

# remove manually locally if you want to rebuild the env
#rm -fr env

if [ ! -d env ]; then
    virtualenv env
    . ./env/bin/activate
    pip install --upgrade robotframework
    pushd tests/robot/lib
    python setup.py install
    popd
else
    echo "Reusing existing robot virtualenv"
    . ./env/bin/activate
fi    


# under jenkins, WORKSPACE is set automatically, but otherwise
# this script will be run there anyway...

pwd=`pwd`; WORKSPACE=${WORKSPACE:-$pwd}
TMP=${TMP:-/tmp}
export PATH="${WORKSPACE}/install/bin:${PATH}
export LD_LIBRARY_PATH="${WORKSPACE}/install/lib"
which evalresp

cd tests/robot
mkdir -p run
rm -fr run/*
WORKSPACE=${WORKSPACE} robot --loglevel DEBUG all
