
To build and run these tests outside of Jenkins do something like:

```
  git clone https://github.com/earthscope/evalresp.git
  cd evalresp
  autoreconf -fi
  touch libtoolT
  ./configure --enable-check
  ./tests/jenkins/build-evalresp.sh
  ./tests/jenkins/clean-test-dirs.sh
  # uncomment these line for automated tests (large amounts of data)
  # ./tests/jenkins/build-extended-robot-tests.sh 2017 1
  # ./tests/jenkins/build-extended-robot-tests.sh 2010 365
  ./tests/jenkins/run-c-tests.sh
  ./tests/jenkins/run-robot-tests.sh
```

Note that for development everything is installed in `install` inside
the `evalresp` directory.  This allows for developing multiple
versions.

Inside Jenkins, call the scripts in the required order during a build
step.

To collect target data from an initial run (in which extended tests
will have failed):

```
  ./tests/jenkins/collect-extended-targets.sh 2017 1
  ./tests/jenkins/collect-extended-targets.sh 2010 365
```

This will create zip files in the target directory.

To create an export tarball (assuming `build-evalresp.sh` was run):

```
  make dist
```
