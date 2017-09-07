
*** Settings ***

Library  Process
Library  Support
Library  OperatingSystem


*** Test Cases ***

'dis' units
    Prepare  base/args/units_dis  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -u  dis
    Count and compare target files two float cols

'vel' units
    Prepare  base/args/units_vel  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -u  vel
    Count and compare target files two float cols

'acc' units
    Prepare  base/args/units_acc  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -u  acc
    Count and compare target files two float cols

'def' units
    Prepare  base/args/units_def  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -u  def
    Count and compare target files two float cols

Time of day
    Prepare  base/args/time_of_day  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -t  06:54:32
    Count and compare target files two float cols

Log spacing
    Prepare  base/args/log_space  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -s  log
    Count and compare target files two float cols

Linear
    Prepare  base/args/linear_space  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -s  lin
    Count and compare target files two float cols

Amp/Phase
    Prepare  base/args/amp_phase  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -r  ap
    Count and compare target files two float cols

Freq/Amp/Phase
    Prepare  base/args/freq_amp_phase  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -r  fap
    Count and compare target files n float cols  3

Complex
    Prepare  base/args/complex  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -r  cs
    Count and compare target files n float cols  3

Net ID
    Prepare  base/args/net_id  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -n  Z
    Count and compare target files two float cols

Bad Net ID
    Prepare  base/args/bad_net_id  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -n  XXX
    Check number of files  1

Location ID
    Prepare  base/args/location_id  base  RESP.IU.ANMO.00.BHZ
    Run process  evalresp  ANMO  BHZ  2010  1  0.001  10  100  -f  RESP.IU.ANMO.00.BHZ  -l  00
    Count and compare target files two float cols

Location ID wildcard 1
    Prepare  base/args/location_id_wild_1  base  RESP.IU.ANMO.00.BHZ
    Run process  evalresp  ANMO  BHZ  2010  1  0.001  10  100  -f  RESP.IU.ANMO.00.BHZ  -l  ??
    Count and compare target files two float cols

Location ID wildcard 2
    Prepare  base/args/location_id_wild_2  base  RESP.IU.ANMO.00.BHZ
    Run process  evalresp  ANMO  BHZ  2010  1  0.001  10  100  -f  RESP.IU.ANMO.00.BHZ  -l  *
    Count and compare target files two float cols

Bad Location ID
    Prepare  base/args/bad_location_id  base  RESP.IU.ANMO.00.BHZ
    Run process  evalresp  ANMO  BHZ  2010  1  0.001  10  100  -f  RESP.IU.ANMO.00.BHZ  -l  01
    Check number of files  1

Stage 3 -
    Prepare  base/args/stage_3  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -stage  3
    Count and compare target files two float cols

Stage 3 - 5
    Prepare  base/args/stage_3_5  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -stage  3  5
    Count and compare target files two float cols

Stdio
    Prepare  base/args/stdio  base  RESP.Z.CGV..HYZ
    Run process  evalresp CGV HYZ 2010 1 0.001 10 100 -stdio < RESP.Z.CGV..HYZ  shell=True  stdout=stdout  stderr=stderr
    Count and compare target files n float cols  3

Estimated delay
    Prepare  base/args/delay  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -use-estimated-delay
    Count and compare target files two float cols

# the following two tests don't test anything - waiting on better data from
# ilya

Interpolate output
    Prepare  base/args/interp_out  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -il
    Count and compare target files two float cols

Interpolate input
    Prepare  base/args/interp_in  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -ii
    Count and compare target files two float cols

Tension
    Prepare  base/args/tension  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -ii  -il  -it  100
    Count and compare target files two float cols

Unwrap
    Prepare  base/args/unwrap  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -unwrap
    Count and compare target files two float cols

Unwrap (IRIS no flag)
    Prepare  base/args/unwrap-iris-no-flag  base  RESP.CT.SAVY..BHE
    Run process  evalresp  SAVY  BHE  1995  265  1.0E-5  20.0  200  -f  RESP.CT.SAVY..BHE  -u  def  -r  ap  -n  CT  -l  \ \   -t  04:25:27  -s  log  -v  stdout=stdout  stderr=stderr
    Compare n float cols absolute  base/args/unwrap-iris-no-flag  2  360  PHASE.CT.SAVY..BHE

Unwrap (IRIS flag)
    Prepare  base/args/unwrap-iris-flag  base  RESP.CT.SAVY..BHE
    Run process  evalresp  SAVY  BHE  1995  265  1.0E-5  20.0  200  -f  RESP.CT.SAVY..BHE  -u  def  -r  ap  -n  CT  -l  \ \   -t  04:25:27  -s  log  -v  -unwrap  stdout=stdout  stderr=stderr
    Compare n float cols absolute  base/args/unwrap-iris-flag  2  360  PHASE.CT.SAVY..BHE

Total sensitivity
    Prepare  base/args/total_sens  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -ts
    Count and compare target files two float cols

Verbose
    Prepare  base/args/verbose  base  RESP.Z.CGV..HYZ
    Run process  evalresp  CGV  HYZ  2010  1  0.001  10  100  -f  RESP.Z.CGV..HYZ  -v  stdout=stdout  stderr=stderr
    Cut Fields  stderr  temp  [  2-
#    Run process  cut  -d  [  -f  2-  stderr  stdout=temp
    Clear Version  temp
    Move File  temp  stderr
    Count and compare target files two float cols

XML
    Prepare  base/args/xml  base  station-1.xml
    Run process  evalresp  ANMO  BHZ  2015  1  0.001  10  100  -f  station-1.xml  -x  -l  10
    Count and compare target files two float cols

# waiting on b62 test from ilya

# the following, by happy mistake, tested spline data, so we are leaving
# it here, even though it does not test the b62 flag as originally intended.

B55 interpolation
    Prepare  base/args/b55  base  RESP.IM.ATTU..BHE
    Run process  evalresp  ATTU  BHE  1998  57  0.001  10  100  -f  RESP.IM.ATTU..BHE  -il
    # 50% difference is high, but leave this in as it checks the file count
    Count and compare target files two float cols  base/args/b55  0.5
    Compare target files two float cols average  base/args/b55  0.015
