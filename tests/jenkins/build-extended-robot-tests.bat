REM @ECHO OFF
IF NOT EXIST src (
    echo "Run this script in the top level directory"
    EXIT /B 1
)

REM TODO Get number of input

SETLOCAL

set year=%1
set day=%2

set DATE=%year%-%day%


set EXTENDED_INPUT=RESP-testset.zip
CALL:download tests\robot\data %EXTENDED_INPUT% http://ds.iris.edu/files/staff/chad/%EXTENDED_INPUT%
PUSHD tests\robot\data
IF EXIST extended (
    echo "Wipping existing extended data"
    rmdir extended /s /q
)
md extended
PUSHD extended
echo "Extracting Data from %EXTENDED_INPUT%"
powershell -Command 'Expand-Archive -Path %EXTENDED_INPUT% -DestinationPath .'

POPD
POPD
set EXTENDED_TARGET=RESP-targets-%DATE%.zip
CALL:download "tests\robot\target" "%EXTENDED_TARGET%" "http://isti.com/~andrew/%EXTENDED_TARGET%"
IF NOT EXIST tests\robot\target\extended (
    md tests\robot\target\extended
)
PUSHD tests\robot\target\extended 
powershell -Command 'Expand-Archive -Path "..\%EXTENDED_TARGET%" -DestinationPath .'
POPD

ENDLOCAL
:download
SETLOCAL
SET DIR=%~t1
SET ZIP=%~t2
SET URL=%~t3
PUSHD %DIR%
IF NOT EXIST %ZIP (
    echo "%ZIP% does not exist"
    IF NOT DEFINED ROBOT_ARCHIVE_CACHE (
        echo "To avoid multiple downloads, place %ZIP% in a cache directory and set"
        echo "ROBOT_ARCHIVE_CACHE to the absolute file location."

        powershell -Command "$script=new-object System.Net.WebClient; $script.DownloadFile($Env:URL, $Env:ZIP);"
    ) ELSE (
        IF NOT EXIST %ROBOT_ARCHIVE_CACHE%\%ZIP% (
            ECHO "WARNING: %ZIP% not found in %ROBOT_ARCHIVE_CACHE%"
        ) ELSE (
	        echo "Copying data from cache at %ROBOT_ARCHIVE_CACHE%"
            copy %ROBOT_ARCHIVE_CACHE%\%ZIP% %ZIP%
        )
    )
)
POPD
ENDLOCAL
EXIT /B 0
