
from __future__ import print_function
from os import environ, chdir, makedirs, getcwd, listdir
from os.path import join, exists, relpath, realpath, isfile
#from shutil import copyfile
from robot.api import logger
try:
    from os import symlink
except Exception:
    def symlink(source, link_name):
        import ctypes
        from os.path import isdir
        csl = ctypes.windll.kernel32.CreateSymbolicLinkW
        csl.argtypes = (ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_uint32)
        csl.restype = ctypes.c_ubyte
        flags = 1 if isdir(source) else 0
        if csl(link_name, source.replace('/', '\\'), flags) == 0:
            raise ctypes.WinError()

try:
    WORKSPACE = environ['WORKSPACE']
    TARGET = join(WORKSPACE, 'tests/robot/target')
    DATA = join(WORKSPACE, 'tests/robot/data')
    RUN = join(WORKSPACE, 'tests/robot/run')
except Exception:
    WORKSPACE = getcwd()
    TARGET = join(WORKSPACE, 'target')
    DATA = join(WORKSPACE, 'data')
    RUN = join(WORKSPACE, 'run')

TINY = 1e-5


class Support:

    """A Robot plugin to support running evalresp tests"""

    def _assert_missing_dir(self, dir):
        if exists(dir):
            raise Exception('Directory %s already exists' % dir)

    def _assert_present_dir(self, dir):
        if not exists(dir):
            raise Exception('Directory %s does not exist' % dir)

    def _assert_equal_floats(self, a, b, tol, location):
        magnitude = max(abs(a), abs(b))
        if magnitude < tol:
            if abs(a - b) > tol:
                raise Exception('%f and %f differ at %s' % (a, b, location))
        else:
            if (abs(a - b) / magnitude) > tol:
                raise Exception('%f and %f differ at %s' % (a, b, location))

    def _assert_equal_floats_absolute(self, a, b, tol, location):
        if abs(a - b) > tol:
            raise Exception('%f and %f differ at %s' % (a, b, location))

    def prepare(self, dest_dir, data_dir, files):
        """Call this method before running evalresp on a single set of files.
        It creates the working directory (under 'run') and copies
        across the required data files (which should be a
        comma-separated list with no spaces)."""
        run = join(RUN, dest_dir)
        self._assert_missing_dir(run)
        makedirs(run)
        for file in files.split(','):
            src = join(DATA, data_dir, file)
            dest = join(run, file)
            # link instead of copy to avoid using disk space with duplicates
            #copyfile(src, dest)
            symlink(src, dest)
        chdir(run)

    def _extract_floats(self, n, line, path, index):
        if not line:
            raise Exception('Data missing from %s at line %d' % (path, index))
        try:
            data = map(float, line.split())
        except ValueError:
            raise Exception('Bad data in %s at line %d' % (path, index))
        if len(data) != int(n):
            raise Exception('Missing data in %s at line %d' % (path, index))
        return data

    def compare_n_float_cols_absolute(self, target_dir, ncols, tol, files):
        """Call this method after running evalresp on a single set of files.
        It checks the given files (a comma-separated list with no
        spaces) between the working directory and the target
        directory.  The test here is absolute (not relative)"""
        ncols = int(ncols)
        tol = float(tol)
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
                        try:
                            result_data = self._extract_floats(ncols, result_line, result_path, index)
                            target_data = self._extract_floats(ncols, target_line, target_path, index)
                            for (r, t) in zip(result_data, target_data):
                                location = '%s and %s at line %d' % (result_path, target_path, index)
                                self._assert_equal_floats_absolute(r, t, tol, location)
                        except Exception, e:
                            # try comparing as text (may be titles etc)
                            if result_line != target_line:
                                raise e
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_n_float_cols(self, target_dir, ncols, tol, files):
        """Call this method after running evalresp on a single set of files.
        It checks the given files (a comma-separated list with no
        spaces) between the working directory and the target
        directory."""
        ncols = int(ncols)
        tol = float(tol)
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
                        try:
                            result_data = self._extract_floats(ncols, result_line, result_path, index)
                            target_data = self._extract_floats(ncols, target_line, target_path, index)
                            for (r, t) in zip(result_data, target_data):
                                location = '%s and %s at line %d' % (result_path, target_path, index)
                                self._assert_equal_floats(r, t, tol, location)
                        except Exception, e:
                            # try comparing as text (may be titles etc)
                            if result_line != target_line:
                                raise e
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_two_float_cols(self, target_dir, tol, files):
        """Call this method after running evalresp on a single set of files.
        It checks the given files (a comma-separated list with no
        spaces) between the working directory and the target
        directory."""
        self.compare_n_float_cols(target_dir, 2, float(tol), files)

    def compare_target_files_two_float_cols(self, target_dir=None, tol=None):
        """Call this method after running evalresp on a single set of files.
        It checks all files in the target directory against those in
        the run directory (the target directory can be inferred if
        both have the same relative paths)."""
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        if tol:
            tol = float(tol)
        else:
            tol = TINY
        target = join(TARGET, target_dir)
        files = ','.join(listdir(target))
        self.compare_two_float_cols(target_dir, tol, files)

    def count_and_compare_target_files_n_float_cols(
            self, ncols, target_dir=None, tol=None):
        """Call this method after running evalresp on a single set of files.
        It checks all files in the target directory against those in
        the run directory (the target directory can be inferred if
        both have the same relative paths).  No additional files can
        be present."""
        ncols = int(ncols)
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        if tol:
            tol = float(tol)
        else:
            tol = TINY
        target = join(TARGET, target_dir)
        files = listdir(target)
        if files:
            filelist = ','.join(files)
            self.compare_n_float_cols(target_dir, ncols, tol, filelist)
        self.check_number_of_files(len(files)+1)

    def count_and_compare_target_files_two_float_cols(
            self, target_dir=None, tol=None):
        """Call this method after running evalresp on a single set of files.
        It checks all files in the target directory against those in
        the run directory (the target directory can be inferred if
        both have the same relative paths).  No additional files can
        be present."""
        self.count_and_compare_target_files_n_float_cols(2, target_dir, tol)

    def check_number_of_files(self, n):
        """Check the number of files in the run directory."""
        n = int(n)
        found = 0
        run = getcwd()
        for file in listdir(run):
            if isfile(join(run, file)):
                found += 1
        if found != n:
            raise Exception('Found %d files in %s, but expected %d' % (found, run, n))

    def compare_text(self, target_dir, files):
        """Call this method to check the given files (a comma-separated
        list with no spaces) between the working directory and the target
        directory."""
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
                        if result_line != target_line:
                            raise Exception('"%s" and "%s" differ at %s and %s at line %d' %
                                            (result_line, target_line,
                                            result_path, target_path, index))
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_target_files_text(self, target_dir=None):
        """Call this method to check the content of all files from
        the target directory."""
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        target = join(TARGET, target_dir)
        files = ','.join(listdir(target))
        self.compare_text(target_dir, files)

    def count_and_compare_target_files_text(self, target_dir=None):
        """Call this method to check the content and number of all files
        from the target directory."""
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        target = join(TARGET, target_dir)
        files = listdir(target)
        if files:
            filelist = ','.join(files)
            self.compare_text(target_dir, filelist)
        self.check_number_of_files(len(files)+1)

    def compare_n_float_cols_average(self, target_dir, ncols, tol, files):
        """Call this method after running evalresp on a single set of files.
        It checks the given files (a comma-separated list with no
        spaces) between the working directory and the target
        directory, using the average relative deviation."""
        ncols = int(ncols)
        tol = float(tol)
        run = join(RUN, getcwd())
        self._assert_present_dir(run)
        target = join(TARGET, target_dir)
        self._assert_present_dir(target)
        for file in files.split(','):
            result_path = join(run, file)
            target_path = join(target, file)
            logger.info('Comparing %s with %s' % (result_path, target_path))
            total, n = 0, 0
            with open(target_path, 'r') as target_file:
                with open(result_path, 'r') as result_file:
                    for (index, result_line) in enumerate(result_file.readlines()):
                        target_line = target_file.readline()
                        try:
                            result_data = self._extract_floats(ncols, result_line, result_path, index)
                            target_data = self._extract_floats(ncols, target_line, target_path, index)
                            for (a, b) in zip(result_data, target_data):
                                magnitude = max(abs(a), abs(b))
                                if magnitude < tol:
                                    delta = abs(a - b)
                                else:
                                    delta = abs(a - b) / magnitude
                            total += delta
                            n += 1
                        except Exception, e:
                            # try comparing as text (may be titles etc)
                            if result_line != target_line:
                                raise e
                average = total / n
                if average > tol:
                    raise Exception('%s and %s have an average (relative) difference of %f > %f' %
                                    (result_path, target_path, average, tol))
                if target_file.readline():
                    raise Exception('Missing data at end of %s' % result_path)

    def compare_target_files_two_float_cols_average(
            self, target_dir=None, tol=None):
        """Call this method after running evalresp on a single set of files.
        It checks all files in the target directory against those in
        the run directory (the target directory can be inferred if
        both have the same relative paths) using the average relative
        deviation."""
        if not target_dir:
            target_dir = relpath(realpath(getcwd()), realpath(RUN))
        if tol:
            tol = float(tol)
        else:
            tol = TINY
        target = join(TARGET, target_dir)
        files = ','.join(listdir(target))
        self.compare_n_float_cols_average(target_dir, 2, tol, files)

    def cut_fields(self, source, destination, delim=',', fields='1-'):
        """ this method is equivelent to cut -d delim -f fields source > destination."""
        field_stop = fields.split('-')
        if len(field_stop) > 2:
            raise Exception('The fields \'%s\' has to many ranges' % (fields))
        with open(source, 'r') as src_file:
            with open(destination, 'w') as dest_file:
                for src_line in src_file.readlines():
                    split_src_line = src_line.split(delim)
                    start = 0
                    end = len(split_src_line) 
                    if field_stop[0]:
                        start = int(field_stop[0])-1
                        if start >= len(split_src_line):
                            start = len(split_src_line) - 1
                    if len(field_stop) < 2:
                        end = int(field_stop[0])-1
                    elif field_stop[1]:
                        end = int(field_stop[1])-1
                        if end >= len(split_src_line):
                            end = len(split_src_line)

                    out_line = delim.join(split_src_line[start:end])
                    dest_file.write("%s" % (out_line))
            dest_file.close()
        src_file.close()

    def clear_version(self, target_file_name):
        """Use this method to clean the version number from verbose output"""
        import re
        from tempfile import mkstemp
        from os import fdopen
        from robot.libraries.BuiltIn import BuiltIn
        ROS = BuiltIn().get_library_instance('OperatingSystem')
        fh, path = mkstemp()
        with fdopen(fh, 'w') as temp_file:
            with open(target_file_name) as target_file:
                #get the line with the version string
                for line in target_file:
                    temp_file.write(re.sub(r'(EVALRESP RESPONSE OUTPUT V)\d.+( >>)',r'\1ROBOT\2',line))
        ROS.remove_file(target_file_name)
        ROS.move_file(path, target_file_name)
