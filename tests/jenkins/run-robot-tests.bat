@ECHO OFF
IF NOT EXIST src (
    echo "Run this script in the top level directory"
    EXIT /B 1
)

REM Need to do everything in a sanitary environment
setlocal

CALL tests\jenkins\install-robot.bat
CALL env\Scripts\activate.bat

IF NOT DEFINED WORKSPACE (
    set WORKSPACE=%cd%
)

IF NOT DEFINED TMP (
    set TMP="C:\TEMP\"
)

set BUILD=%WORKSPACE%

set PATH=%BUILD%\install\bin;%PATH%

PUSHD tests\robot

IF NOT EXIST run (
    md run
)

REM ( dir /b /a "run"|findstr . ) > nul && (
FOR /D %%p IN ("run\*.*") DO rmdir "%%p" /s /q 
REM ) || echo "run/ is clean"

robot --loglevel DEBUG all

POPD

CALL env\Scripts\deactivate.bat

REM end of sanitary environment
endlocal
