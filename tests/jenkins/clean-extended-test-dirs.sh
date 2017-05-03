#!/bin/bash -e

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

rm -fr tests/robot/all/extended/*
rm -fr tests/robot/target/extended/*

