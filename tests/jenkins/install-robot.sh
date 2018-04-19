#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi


# install robot in a virtualenv
# this directory is always removed on jenkins (if configured correctly)

# remove manually locally if you want to rebuild the env
#rm -fr env

if [ ! -d "env/bin" ]; then
    virtualenv env
    . ./env/bin/activate
    pip install --upgrade robotframework
else
    echo "Reusing existing robot virtualenv"
    . ./env/bin/activate
fi    

# install this every time in case we change it
echo "(Re-)Installing support library"
pushd tests/robot/lib
python setup.py install
popd
