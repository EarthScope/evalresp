#!/bin/bash

if [ ! -f IRIS-WS-2.0.12.jar ]; then
    echo "IRIS-WS-2.0.12.jar must be present"
    echo "http://ds.iris.edu/ds/nodes/dmc/software/downloads/IRIS-WS/2-0-12/"
    exit 1
fi

javac -cp IRIS-WS-2.0.12.jar Convert.java
java -cp IRIS-WS-2.0.12.jar:. Convert < ../data/station-1.xml > ../data/response-1
java -cp IRIS-WS-2.0.12.jar:. Convert < ../data/station-2.xml > ../data/response-2


