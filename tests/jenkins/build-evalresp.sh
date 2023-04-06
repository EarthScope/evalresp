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

# autoconf approach
#rm -fr m4; mkdir m4  # needed by ancient aclocal on jenkins machine
#autoreconf -fi
#./configure --prefix `pwd`/install --enable-check
#make
#make install

# standalone makefile approach
sed -i -e 's|^INSTALL_DIR.*|INSTALL_DIR = $(PWD)/install|' Build.config
make -f Makefile
make -f Makefile install
