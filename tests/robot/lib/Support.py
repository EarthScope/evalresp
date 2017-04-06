
from __future__ import print_function
from os import environ, chdir, mkdir
from os.path import join, exists
from shutil import copyfile
from robot.api import logger

WORKSPACE = environ['WORKSPACE']
DATA = join(WORKSPACE, 'tests/robot/data')
RUN = join(WORKSPACE, 'tests/robot/run')

class Support:

    """A Robot plugin to support running evalresp tests"""

    def _assert_missing_dir(self, dir):
        if exists(dir):
            raise Exception('Directory %s already exists' % dir)

    def prepare(self, dir, files):
        run = join(RUN, dir)
        self._assert_missing_dir(run)
        mkdir(run)
        for file in files.split(','):
            src = join(DATA, file)
            dest = join(RUN, dir, file)
            copyfile(src, dest)
        chdir(run)
