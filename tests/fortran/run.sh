#!/bin/bash

# these variables may need modifying - see Build.config in the build dir
INSTALL_DIR=../../install
#INSTALL_DIR=/usr/local
BIN_DIR=$INSTALL_DIR/bin
LIB_DIR=$INSTALL_DIR/lib

echo "compiling evtest"
gfortran evtest.f -L $LIB_DIR -g -o evtest -levalresp -levalresp_log -lmxmlev -lspline

echo "running evtest"
LD_LIBRARY_PATH=$LIB_DIR ./evtest

echo "running evalresp"
PATH=$BIN_DIR:$PATH
evalresp "*" VMZ 2010 260 0.0001 100 100 -x -f ../c/data/station-1.xml -b62_x 3 -stage 1 1

echo "generating comparison"
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

echo "comparison:"
cat comparison
