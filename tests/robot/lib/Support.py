
from __future__ import print_function
from os import environ, chdir, mkdir, getcwd, listdir
from os.path import join, exists, relpath, realpath
from shutil import copyfile
from robot.api import logger

WORKSPACE = environ['WORKSPACE']
TARGET = join(WORKSPACE, 'tests/robot/target')
DATA = join(WORKSPACE, 'tests/robot/data')
RUN = join(WORKSPACE, 'tests/robot/run')

TINY = 1e-8


class Support:

    """A Robot plugin to support running evalresp tests"""

    def _assert_missing_dir(self, dir):
        if exists(dir):
            raise Exception('Directory %s already exists' % dir)

    def _assert_present_dir(self, dir):
        if not exists(dir):
            raise Exception('Directory %s does not exist' % dir)

    def prepare(self, dir, files):
        """Call this method before running evalresp.  It creates the
        working directory (under 'run') and copies across the required
        data files (which should be a comma-separated list with no 
        spaces)."""
        run = join(RUN, dir)
        self._assert_missing_dir(run)
        mkdir(run)
        for file in files.split(','):
            src = join(DATA, file)
            dest = join(RUN, dir, file)
            copyfile(src, dest)
        chdir(run)

    def _extract_floats(self, n, line, path, index):
        if not line:
            raise Exception('Data missing from %s at line %d' % (path, index))
        try:
            data = map(float, line.split())
        except FormatException:
            raise Exception('Bad data in %s at line %d' % (path, index))
        if len(data) != n:
            raise Exception('Missing data in %s at line %d' % (path, index))
        return data

    def compare_two_float_cols(self, dir, files):
        """Call this method after running evalresp.  It checks the given
        files (a comma-separated list with no spaces) between the working
        directory and the target directory (both should have the same
        relative paths)."""
        run = join(RUN, dir)
        self._assert_present_dir(run)
        target = join(TARGET, dir)
        self._assert_present_dir(target)
        for file in files.split(','):
            result_path = join(run, file)
            target_path = join(target, file)
            logger.info('Comparing %s with %s' % (result_path, target_path))
            with open(target_path, 'r') as target_file:
                with open(result_path, 'r') as result_file:
                    for (index, result_line) in enumerate(result_file.readlines()):
                        target_line = target_file.readline()
                        result_data = self._extract_floats(2, result_line, result_path, index)
                        target_data = self._extract_floats(2, target_line, target_path, index)
                        for (r, t) in zip(result_data, target_data):
                            a = max(abs(r), abs(t))
                            if a < TINY:
                                if abs(r - t) > TINY:
                                    raise Exception('%s and %s differ at line %d' % (result_path, target_path, index))
                                    
                            else:
                                if (abs(r-t)/a) > TINY:
                                    raise Exception('%s and %s differ at line %d' % (result_path, target_path, index))
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_target_files_two_float_cols(self):
        """As 'compare_two_float_cols', but inferring all arguments
        automatically."""
        # infer target path
        dir = relpath(realpath(getcwd()), realpath(RUN))
        target = join(TARGET, dir)
        files = ','.join(listdir(target))
        self.compare_two_float_cols(dir, files)
