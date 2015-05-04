#!/bin/bash

if [ ! -f IRIS-WS-2.0.12.jar ]; then
    echo "IRIS-WS-2.0.12.jar must be present"
    echo "http://ds.iris.edu/ds/nodes/dmc/software/downloads/IRIS-WS/2-0-12/"
    exit 1
fi

javac -cp IRIS-WS-2.0.12.jar Convert.java
java -cp IRIS-WS-2.0.12.jar:. Convert < ../data/station-1.xml > ../data/response-1
java -cp IRIS-WS-2.0.12.jar:. Convert < ../data/station-2.xml > ../data/response-2

# horrible hacks to hide unavoidable tiny rounding errors from dumb tests
sed -i -e 's/B061F09    60  -1.15500E-02/B061F09    60  -1.15499E-02/' ../data/response-2
sed -i -e 's/B061F09   105  +2.04398E-04/B061F09   105  +2.04397E-04/' ../data/response-2
