#!/bin/bash

echo ${WORKSPACE}
rm -fr ${WORKSPACE}/env
virtualenv ${WORKSPACE}/env
