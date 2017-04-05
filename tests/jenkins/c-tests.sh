#!/bin/bash -ex

if [ ! -d src ]; then
    echo "run this script in the top level directory"
    exit 1
fi

make check
