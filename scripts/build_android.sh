#!/bin/bash

# get build mode
BUILD_MODE="Release"
GRADLE_TASK="assemble"
INSTALL_REQUESTED=false

for arg in "$@"; do
  case $arg in
    --debug)
      BUILD_MODE="Debug"
      ;;
    --install)
      INSTALL_REQUESTED=true
      GRADLE_TASK="install"
      ;;
  esac
done

# get directories (android root)
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
PARENT_DIR=$(dirname "$SCRIPT_DIR")
ANDROID_DIR="$PARENT_DIR/android"

# get env
XMAKE_PATH=$(command -v xmake)
if [ -z "$XMAKE_PATH" ]; then echo "'xmake' not found in PATH"; exit 1; fi

if [ "$INSTALL_REQUESTED" = true ]; then
    ADB_PATH=$(command -v adb)
    if [ -z "$ADB_PATH" ]; then 
        echo "--install requested but 'adb' not found in PATH";
        exit 1; 
    fi
fi

: "${ANDROID_SDK_HOME:=$ANDROID_SDK_ROOT}"
: "${ANDROID_NDK_HOME:=$ANDROID_NDK_ROOT}"

if [ -z "$ANDROID_SDK_HOME" ] || [ ! -d "$ANDROID_SDK_HOME" ]; then
    echo "ANDROID_HOME (or ANDROID_SDK_ROOT) not set or not a directory" >&2
    exit 1
fi

if [ -z "$ANDROID_NDK_HOME" ] || [ ! -d "$ANDROID_NDK_HOME" ]; then
    echo "ANDROID_NDK_HOME (or ANDROID_NDK_ROOT) not set or not a directory" >&2
    exit 1
fi

echo "configured as:"
echo "   build task : $GRADLE_TASK$BUILD_MODE"
echo "   android dir: $ANDROID_DIR"
echo "   xmake path : $XMAKE_PATH"
if [ "$INSTALL_REQUESTED" = true ]; then echo "   adb path   : $ADB_PATH"; fi
echo "   android ndk: $ANDROID_NDK_HOME"
echo "   android sdk: $ANDROID_SDK_HOME"
echo ""

pushd "$ANDROID_DIR" > /dev/null
set -x

./gradlew "$GRADLE_TASK$BUILD_MODE"

{ set +x; } 2>/dev/null
popd > /dev/null