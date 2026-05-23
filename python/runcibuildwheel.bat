@echo off
if "%1"=="" (
    echo Error: Solver name required
    echo Usage: runcibuildwheel.bat ^<solver^>
    echo Example: runcibuildwheel.bat copt
    exit /b 1
)

set SOLVER=%1


if not exist "%SOLVER%" (
    echo Error: Directory '%SOLVER%' does not exist
    exit /b 1
)


set CIBW_SKIP=pp* *_i686 *-win32 *musllinux* cp38* cp39* cp310* cp311* cp314* cp312-* cp314t-*
set CIBW_ARCHS_WINDOWS=AMD64
set CIBW_TEST_REQUIRES=--index-url https://pypi.ampl.com --extra-index-url https://pypi.org/simple amplpy ampl_module_base pandas
set  CIBW_TEST_COMMAND=python -m amplpy_%SOLVER%.tests

echo Building wheels for %SOLVER%...
cd %SOLVER%
cibuildwheel --platform windows --archs AMD64 --output-dir tmp .