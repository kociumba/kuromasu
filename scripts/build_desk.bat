@echo off

:: get build mode
set "BUILD_MODE=release"
set "XMAKE_YES="

for %%a in (%*) do (
    if "%%~a"=="--debug" set "BUILD_MODE=debug"
    if "%%~a"=="--yes" set "XMAKE_YES=-y"
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
    echo %XMAKE_PATH% f -c %XMAKE_YES% -m %BUILD_MODE%
    %XMAKE_PATH% f -c %XMAKE_YES% -m %BUILD_MODE%
    echo.
    echo %XMAKE_PATH% b
    %XMAKE_PATH% b
    popd
)