@ECHO OFF
IF NOT EXIST src (
    echo "Run this script in the top level directory"
    EXIT /B 1
)

RMDIR install

SETLOCAL

set CWD=%CD%
CALL "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\Tools\\VsDevCmd.bat" -arch=x64
CALL "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\Tools\\vsdevcmd\\ext\\vcvars.bat"
cd %CD%
nmake /F Makefile.nmake
nmake /F Makefile.nmake install-for-robot

ENDLOCAL
