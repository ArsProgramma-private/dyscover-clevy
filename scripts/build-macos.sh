#!/usr/bin/env bash
set -euo pipefail
# build-macos.sh - Build Dyscover on macOS.
# Usage:
#   ./scripts/build-macos.sh [-c Debug|Release] [-l nl] [-L demo|full] [--no-tests] [--integration] [--universal] [--package]
# Notes:
#   Requires: Xcode command line tools, Homebrew: wxwidgets portaudio gettext (as needed).
#   Universal build adds arm64; initial plan targets x86_64 only unless --universal given.

CONFIG=Debug
LANGUAGE=nl
LICENSING=demo
BUILD_TESTS=ON
INTEGRATION=OFF
UNIVERSAL=OFF
PACKAGE=OFF

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--config) CONFIG="$2"; shift 2;;
    -l|--language) LANGUAGE="$2"; shift 2;;
    -L|--licensing) LICENSING="$2"; shift 2;;
    --no-tests) BUILD_TESTS=OFF; shift;;
    --integration) INTEGRATION=ON; shift;;
    --universal) UNIVERSAL=ON; shift;;
    --package) PACKAGE=ON; shift;;
    -h|--help) grep '^#' "$0" | sed 's/^# //' ; exit 0;;
    *) echo "Unknown arg: $1"; exit 1;;
  esac
done

BUILD_DIR="build-macos-$CONFIG"
mkdir -p "$BUILD_DIR"

ARCHES="x86_64"
if [[ $UNIVERSAL == ON ]]; then
  ARCHES="x86_64;arm64"
fi

echo "[Build] Generating ($CONFIG) arches=$ARCHES";
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=$CONFIG \
  -DLANGUAGE=$LANGUAGE \
  -DLICENSING=$LICENSING \
  -DBUILD_TESTS=$BUILD_TESTS \
  -DBUILD_INTEGRATION_TESTS=$INTEGRATION \
  -DCMAKE_OSX_ARCHITECTURES="$ARCHES"

echo "[Build] Compiling Dyscover";
cmake --build "$BUILD_DIR" --target Dyscover -j$(sysctl -n hw.ncpu)

if [[ $BUILD_TESTS == ON ]]; then
  echo "[Test] Running unit tests";
  ctest --test-dir "$BUILD_DIR" --output-on-failure -R unit-
fi

if [[ $PACKAGE == ON ]]; then
  OUT_DIR="dist-macos-$CONFIG"
  rm -rf "$OUT_DIR" && mkdir -p "$OUT_DIR"
  cp "$BUILD_DIR/Dyscover" "$OUT_DIR"/
  echo "[Package] Created $OUT_DIR (bundle packaging not yet implemented)"
fi

echo "[Done] Build complete."