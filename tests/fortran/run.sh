#!/bin/bash

# compile
gfortran evtest.f -g -o evtest -levresp -lx2r -lxml2

# run (puts output in evtest.out)
LD_LIBRARY_PATH=/usr/local/lib ./evtest

# run equivalent evalresp (to AMP.IU.ANMO.00.VMZ and PHASE.IU.ANMO.00.VMZ)
evalresp "*" VMZ 2010 260 0.0001 100 100 -x -f ../data/station-1.xml -b62_x 3 -stage 1 1

# paste together for comparison
sed -i -e 's/^\s*//' evtest.out
sed -i -e 's/\s\s*/ /g' evtest.out
sed -i -e 's/\s\s*/ /g' AMP.IU.ANMO.00.VMZ
sed -i -e 's/\s\s*/ /g' PHASE.IU.ANMO.00.VMZ

cut -d ' ' -f 1 evtest.out > tmp-f
cut -d ' ' -f 2 evtest.out > tmp-a1
cut -d ' ' -f 3 evtest.out > tmp-p1
cut -d ' ' -f 2 AMP.IU.ANMO.00.VMZ > tmp-a2
cut -d ' ' -f 2 PHASE.IU.ANMO.00.VMZ > tmp-p2
paste tmp-f tmp-a1 tmp-a2 tmp-p1 tmp-p2 > tmp-all
echo "freq            amp (f)         amp (c)         phase (f)       phase (c)" > comparison
cat tmp-all >> comparison
rm tmp*

# not a great choice of responses to compare, but it seems to be working
cat comparison
