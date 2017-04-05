#!/bin/bash -ex

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

rm -fr env
virtualenv env
. ./env/bin/activate
pip install --upgrade robotframework

# install the library we use to extend robot
#pushd tests/robot/lib
#python setup.py install
#popd

export PATH="${PATH}:${WORKSPACE}/install/bin"
cd tests/robot
rm -fr run
mkdir run
cd run
robot ../all
