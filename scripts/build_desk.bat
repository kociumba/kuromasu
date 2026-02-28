@echo off

:: get build mode
set "BUILD_MODE=release"

for %%a in (%*) do (
    if "%%~a"=="--debug" set "BUILD_MODE=debug"
)

:: get parent dir (project root)
for %%i in ("%~dp0..") do set "PARENT_DIR=%%~fi"

:: get xmake path
for /f "delims=" %%i in ('where xmake 2^>nul') do (
    set "XMAKE_PATH=%%i"
    goto :found
)

echo 'xmake' not found in PATH.
exit /b 1

:found
echo configured as:
echo    build mode: %BUILD_MODE%
echo    parent dir: %PARENT_DIR%
echo    xmake path: %XMAKE_PATH%
echo.

pushd %PARENT_DIR% && (
    %XMAKE_PATH% f -c -m %BUILD_MODE%
    echo.
    %XMAKE_PATH% b
    popd
)