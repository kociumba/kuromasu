@echo off
setlocal enabledelayedexpansion

:: get build mode
set "BUILD_MODE=Release"
set "GRADLE_TASK=assemble"
set "INSTALL_REQUESTED=false"

for %%a in (%*) do (
    if "%%~a"=="--debug" (
        set "BUILD_MODE=Debug"
    )
    if "%%~a"=="--install" (
        set "INSTALL_REQUESTED=true"
        set "GRADLE_TASK=install"
    )
)

:: get directories (android root)
for %%i in ("%~dp0..") do set "PARENT_DIR=%%~fi"
set "ANDROID_DIR=%PARENT_DIR%\android"

:: get env
for /f "delims=" %%i in ('where xmake 2^>nul') do set "XMAKE_PATH=%%i"
if "%XMAKE_PATH%"=="" (
    echo 'xmake' not found in PATH.
    exit /b 1
)

if "%INSTALL_REQUESTED%"=="true" (
    for /f "delims=" %%i in ('where adb 2^>nul') do set "ADB_PATH=%%i"
    if "!ADB_PATH!"=="" (
        echo --install requested but 'adb' not found in PATH.
        exit /b 1
    )
)

if "%ANDROID_SDK_HOME%"=="" set "ANDROID_SDK_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK_HOME%"=="" set "ANDROID_NDK_HOME=%ANDROID_NDK_ROOT%"

if "%ANDROID_SDK_HOME%"=="" (
    echo ANDROID_SDK_HOME (or ANDROID_SDK_ROOT) not set.
    exit /b 1
)
if not exist "%ANDROID_SDK_HOME%\" (
    echo ANDROID_SDK_HOME directory does not exist: %ANDROID_SDK_HOME%
    exit /b 1
)

if "%ANDROID_NDK_HOME%"=="" (
    echo ANDROID_NDK_HOME (or ANDROID_NDK_ROOT) not set.
    exit /b 1
)
if not exist "%ANDROID_NDK_HOME%\" (
    echo ANDROID_NDK_HOME directory does not exist: %ANDROID_NDK_HOME%
    exit /b 1
)

echo configured as:
echo    build task : %GRADLE_TASK%%BUILD_MODE%
echo    android dir: %ANDROID_DIR%
echo    xmake path : %XMAKE_PATH%
if "%INSTALL_REQUESTED%"=="true" echo    adb path   : %ADB_PATH%
echo    android ndk: %ANDROID_NDK_HOME%
echo    android sdk: %ANDROID_SDK_HOME%
echo.

pushd "%ANDROID_DIR%" && (
    call gradlew.bat %GRADLE_TASK%%BUILD_MODE%
    popd
)

endlocal