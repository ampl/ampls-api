rem Downloads the latest solver-public-libs universal package from the Azure
rem Artifacts feed into this directory (libs\) and unpacks it, mirroring
rem support\templates\download-solver-libs.yml.
rem
rem Requirements: Azure CLI + "azure-devops" extension, and a prior "az login"
rem (or "az devops login" with a PAT that has Packaging (Read) scope).
rem Usage: download-azure.bat [version]   (default: * = latest)

set "ORG=https://dev.azure.com/ampldev/"
set "FEED=solver-libs-feed"
set "PACKAGE=solver-public-libs"
set "VERSION=%~1"
if "%VERSION%"=="" set "VERSION=*"

cd /d "%~dp0"


where az >nul 2>nul
if errorlevel 1 (
    echo ERROR: Azure CLI ^(az^) not found. Install it with: winget install --exact --id Microsoft.AzureCLI
    exit /b 1
)

echo Downloading %PACKAGE% ^(version %VERSION%^) from %FEED%...
call az artifacts universal download --organization "%ORG%" --feed "%FEED%" --name "%PACKAGE%" --version "%VERSION%" --path .
if errorlevel 1 exit /b 1

echo Unpacking %PACKAGE%.zip...
if exist "%PACKAGE%" rmdir /s /q "%PACKAGE%"
tar -xf "%PACKAGE%.zip"
if errorlevel 1 exit /b 1

rem The zip nests the payload as %PACKAGE%\%PACKAGE%\<solver>\...
set "SRC=%PACKAGE%\%PACKAGE%"
if not exist "%SRC%" set "SRC=%PACKAGE%"
xcopy /e /i /y /q "%SRC%\*" . >nul
if errorlevel 1 exit /b 1
rmdir /s /q "%PACKAGE%"
echo Done.

