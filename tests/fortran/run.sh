#!/bin/bash

gfortran evtest.f -o evtest -levresp -lx2r -lxml2
LD_LIBRARY_PATH=/usr/local/lib ./evtest
cat evtest.out
