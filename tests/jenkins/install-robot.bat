@ECHO OFF
IF NOT EXIST src (
    echo "Run this script in the top level directory"
    EXIT /B 1
)
REM Need to do everything in a sanitary environment

IF NOT EXIST env (
    python -m virtualenv env
    CALL env\Scripts\activate.bat
    pip install --upgrade robotframework
) ELSE (
    CALL env\Scripts\activate.bat
)

REM install this every time in case we change it
echo "(Re-)Installing support library"
pushd tests\robot\lib
python setup.py install
popd
CALL env\Scripts\deactivate.bat
