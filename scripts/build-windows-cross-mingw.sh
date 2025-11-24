#!/usr/bin/env bash
set -euo pipefail
# build-windows-cross-mingw.sh - Cross-compile Dyscover for Windows (x86_64) from Linux using MinGW-w64.
# Usage:
#   ./scripts/build-windows-cross-mingw.sh [-c Debug|Release] [-l nl] [-L demo|full] [--no-tests] [--package]
# Example:
#   ./scripts/build-windows-cross-mingw.sh -c Release -l nl -L demo --package
# Notes:
#   Requires packages: mingw-w64, x86_64-w64-mingw32-gcc/g++, cmake.
#   wxWidgets must be cross-built or replaced with native Windows build (recommended to build on Windows instead).
#   This path is optional and may not support all runtime functionality if dependencies absent.

CONFIG=Debug
LANGUAGE=nl
LICENSING=demo
BUILD_TESTS=ON
PACKAGE=OFF

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--config) CONFIG="$2"; shift 2;;
    -l|--language) LANGUAGE="$2"; shift 2;;
    -L|--licensing) LICENSING="$2"; shift 2;;
    --no-tests) BUILD_TESTS=OFF; shift;;
    --package) PACKAGE=ON; shift;;
    -h|--help) grep '^#' "$0" | sed 's/^# //' ; exit 0;;
    *) echo "Unknown arg: $1"; exit 1;;
  esac
done

BUILD_DIR="build-cross-win64-$CONFIG"
TOOLCHAIN="scripts/toolchains/mingw-w64-x86_64.cmake"

mkdir -p "$BUILD_DIR"

echo "[Cross] Generating ($CONFIG)";
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
  -DCMAKE_BUILD_TYPE=$CONFIG \
  -DLANGUAGE=$LANGUAGE \
  -DLICENSING=$LICENSING \
  -DBUILD_TESTS=$BUILD_TESTS || {
    echo "Generation failed (check toolchain and dependencies)"; exit 1;
  }

echo "[Cross] Building Dyscover";
cmake --build "$BUILD_DIR" --target Dyscover -j$(nproc) || {
  echo "Build failed"; exit 1;
}

if [[ $BUILD_TESTS == ON ]]; then
  echo "[Cross] Running unit tests (may fail if tests rely on platform specifics)";
  ctest --test-dir "$BUILD_DIR" --output-on-failure || true
fi

if [[ $PACKAGE == ON ]]; then
  OUT_DIR="dist-cross-win64-$CONFIG"
  rm -rf "$OUT_DIR" && mkdir -p "$OUT_DIR"
  cp "$BUILD_DIR/Dyscover.exe" "$OUT_DIR"/ || true
  echo "[Cross] Created $OUT_DIR (DLL dependency resolution not automated)"
fi

echo "[Done] Cross-build complete."