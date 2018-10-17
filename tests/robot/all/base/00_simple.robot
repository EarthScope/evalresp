
*** Settings ***

Library  Process
Library  Support
Library  OperatingSystem


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
    Compare target files two float cols  base/simple/direct  tol=.00001
    Check number of files  3

A simple call to evalresp (alternative data file)
    Prepare  base/simple/direct_2  base  RESP.IM.ATTU..BHE
    Run process  evalresp  ATTU  BHE  1998  57  0.001  10  100  -f  RESP.IM.ATTU..BHE
    Compare target files two float cols
    Check number of files  3

Filter duplicates (warning message in log)
    Prepare  base/simple/filter  base  RESP.H2.H2O.00.HHZ
    Run process  evalresp  H2O  HHZ  2001  235  1.0E-5  160.0  200  -f  RESP.H2.H2O.00.HHZ  -u  def  -r  fap  -n  H2  -l  00  -t  08:16:02  -s  log  -v  stdout=stdout  stderr=stderr
    Cut Fields  stderr  temp  [  2-
    Clear Version  temp
    Move File  temp  stderr
    Compare text  base/simple/filter  stdout,stderr

Filter duplicates (get expected result)
    Prepare  base/simple/filter_2  base  RESP.CT.SAVY..BHE
    Run process  evalresp  SAVY  BHE  1995  265  1.0E-5  20.0  200  -f  RESP.CT.SAVY..BHE  -u  def  -r  ap  -n  CT  -l  \ \   -t  04:25:27  -s  log  -v
    Count and compare target files two float cols

Open date in XML
    Prepare  base/simple/open  base  TITA1.HGZ
    Run process  evalresp  TITA1  HGZ  2019  1  0.001  10  100  -f  TITA1.HGZ  -x
    Count and compare target files two float cols
