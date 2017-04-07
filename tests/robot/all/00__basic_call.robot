
*** Settings ***

Library  Process
Library  Support


*** Test Cases ***

A simple call to evalresp
    Prepare  basic_call/simple  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Compare target files two float cols
    Check number of files  3

Bad date gives no data
    Prepare  basic_call/bad_date   RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2009  12  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Bad site gives no data
    Prepare  basic_call/bad_site   RESP.Z.CGV..HYZ
    Run process  evalresp  XXX  HYZ  2009  12  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Bad channel gives no data
    Prepare  basic_call/bad_channel   RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  XXX  2009  12  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Check number of files  1

Wildcards match all data
    Prepare  basic_call/wildcards  RESP.Z.CGV..HYZ
    Run process  evalresp  *  *  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ
    Compare target files two float cols  basic_call/simple
    Check number of files  3

