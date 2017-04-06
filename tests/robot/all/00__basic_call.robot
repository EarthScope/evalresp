
*** Settings ***

Library  Process
Library  Support


*** Test Cases ***

A Basic Call To Evalresp
    Prepare  basic  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ

