
*** Settings ***

Library  Process
Library  Support


*** Test Cases ***

A simple call to evalresp
    Prepare  base/simple/direct  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Compare target files two float cols
    Check number of files  3

Bad date gives no data
    Prepare  base/simple/bad_date  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2009  365  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Bad site gives no data
    Prepare  base/simple/bad_site  base  RESP.Z.CGV..HYZ
    Run process  evalresp  XXX  HYZ  2009  365  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Bad channel gives no data
    Prepare  base/simple/bad_channel  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  XXX  2009  365  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Wildcards match all data
    Prepare  base/simple/wildcards  base  RESP.Z.CGV..HYZ
    Run process  evalresp  *  *  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Compare target files two float cols  base/simple/direct
    Check number of files  3

