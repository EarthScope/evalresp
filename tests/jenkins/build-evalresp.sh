#!/bin/bash -ex

# previously (in jenkins directly)
# git clone git@github.com-evalresp:earthscope/evalresp.git
# where github.com-evalresp is defined in ~/.ssh/config according to
# https://gist.github.com/jexchan/2351996

if [ ! -d src ]; then
    echo "Run this script in the top level directory"
    exit 1
fi

rm -fr install

# Pass INSTALL_DIR on the command line (overrides Build.config) rather than
# editing Build.config with sed -i, which is not portable (leaves
# Build.config-e on macOS) and dirties a tracked file.
INSTALL_DIR="$(pwd)/install"
make -f Makefile INSTALL_DIR="${INSTALL_DIR}"
make -f Makefile INSTALL_DIR="${INSTALL_DIR}" install
