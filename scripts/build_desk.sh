#!/bin/bash

# get build mode
BUILD_MODE="release"

for arg in "$@"; do
  if [[ "$arg" == "--debug" ]]; then
    BUILD_MODE="debug"
    break
  fi
done

# get parent dir (project root)
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
PARENT_DIR=$(dirname "$SCRIPT_DIR")

# get xmake path
XMAKE_PATH=$(command -v xmake)

if [ -z "$XMAKE_PATH" ]; then
    echo "'xmake' not found in PATH."
    exit 1
fi

echo "configured as:"
echo "   build mode: $BUILD_MODE"
echo "   parent dir: $PARENT_DIR"
echo "   xmake path: $XMAKE_PATH"
echo

pushd $PARENT_DIR > /dev/null

$XMAKE_PATH f -c -m $BUILD_MODE
echo
$XMAKE_PATH b

popd > /dev/null