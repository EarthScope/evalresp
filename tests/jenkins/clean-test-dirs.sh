#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

tests/jenkins/clean-base-test-dirs.sh
tests/jenkins/clean-extended-test-dirs.sh

