
To build and run these tests outside of Jenkins do something like:

  tar xvfz evalresp.tgz
  cd evalresp
  ./tests/jenkins/build-evalresp.sh
  ./tests/jenkins/clean-test-dirs.sh
  # uncomment this line for automated tests (large amounts of data)
  # ./tests/jenkins/build-extended-robot-tests.sh 2017 1
  ./tests/jenkins/run-c-tests.sh
  ./tests/jenkins/run-robot-tests.sh

Inside Jenkins, call the scripts in the required order during a build
step.
