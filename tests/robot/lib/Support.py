
from __future__ import print_function
from os import environ, chdir, makedirs, getcwd, listdir, symlink
from os.path import join, exists, relpath, realpath, isfile
#from shutil import copyfile
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

    def _assert_equal_floats(self, a, b, location):
        magnitude = max(abs(a), abs(b))
        if magnitude < TINY:
            if abs(a - b) > TINY:
                raise Exception('%f and %f differ at %s' % (a, b, location))
        else:
            if (abs(a - b) / magnitude) > TINY:
                raise Exception('%f and %f differ at %s' % (a, b, location))

    def prepare(self, dir, files):
        """Call this method before running evalresp.  It creates the
        working directory (under 'run') and copies across the required
        data files (which should be a comma-separated list with no 
        spaces)."""
        run = join(RUN, dir)
        self._assert_missing_dir(run)
        makedirs(run)
        for file in files.split(','):
            src = join(DATA, file)
            dest = join(RUN, dir, file)
            # link instead of copy to avoid using disk space with duplicates
            #copyfile(src, dest)
            symlink(src, dest)
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

    def compare_two_float_cols(self, target_dir, files):
        """Call this method after running evalresp.  It checks the given
        files (a comma-separated list with no spaces) between the working
        directory and the target directory."""
        run = join(RUN, getcwd())
        self._assert_present_dir(run)
        target = join(TARGET, target_dir)
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
                            location = '%s and %s at line %d' % (result_path, target_path, index)
                            self._assert_equal_floats(r, t, location)
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_target_files_two_float_cols(self, target_dir=None):
        """Call this method after running evalresp.  It checks all files
        in the target directory against those in the run directory
        (the target directory can be inferred of both have the same 
        relative paths)."""
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        target = join(TARGET, target_dir)
        files = ','.join(listdir(target))
        self.compare_two_float_cols(target_dir, files)

    def check_number_of_files(self, n):
        """Check the number of files in the run directory."""
        n = int(n)
        found = 0
        for file in listdir(getcwd()):
            if isfile(join(getcwd(), file)):
                found += 1
        if found != n:
            raise Exception('Found  %d files in %s, but expected %d' % (found, getcwd(), n))
