
To run these tests outside of jenkins define the WORKSPACE variable to
be wherever evalresp is unpacked to.

So something like:

  tar xvfz evalresp.tgz
  cd evalresp
  WORKSPACE=`pwd` ./tests/jenkins/build.sh
  WORKSPACE=`pwd` ./tests/jenkins/c-tests.sh
  WORKSPACE=`pwd` ./tests/jenkins/robot-tests.sh

