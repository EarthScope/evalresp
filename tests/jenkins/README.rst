
To build and run these tests outside of Jenkins do something like:

  tar xvfz evalresp.tgz
  cd evalresp
  ./tests/jenkins/build.sh
  ./tests/jenkins/c-tests.sh
  ./tests/jenkins/robot-tests.sh

Inside Jenkins, call the scripts in the required order during a build
setp.
