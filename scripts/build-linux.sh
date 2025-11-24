#!/usr/bin/env bash
set -euo pipefail
# build-linux.sh - Build Dyscover on Linux.
# Usage:
#   ./scripts/build-linux.sh [-c Debug|Release] [-l nl] [-L demo|full] [--no-tests] [--integration] [--bench] [--package]
# Examples:
#   ./scripts/build-linux.sh -c Release -l nl -L demo --bench --package
# Notes:
#   Requires: gcc/clang, cmake >=3.15, wxWidgets dev, libudev dev, (libpulse|alsa) optional, gettext, librstts.

CONFIG=Debug
LANGUAGE=nl
LICENSING=demo
BUILD_TESTS=ON
INTEGRATION=OFF
BENCH=OFF
PACKAGE=OFF

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--config) CONFIG="$2"; shift 2;;
    -l|--language) LANGUAGE="$2"; shift 2;;
    -L|--licensing) LICENSING="$2"; shift 2;;
    --no-tests) BUILD_TESTS=OFF; shift;;
    --integration) INTEGRATION=ON; shift;;
    --bench) BENCH=ON; shift;;
    --package) PACKAGE=ON; shift;;
    -h|--help)
      grep '^#' "$0" | sed 's/^# //' ; exit 0;;
    *) echo "Unknown arg: $1"; exit 1;;
  esac
done

BUILD_DIR="build-linux-$CONFIG"
mkdir -p "$BUILD_DIR"

echo "[Build] Generating ($CONFIG)";
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=$CONFIG \
  -DLANGUAGE=$LANGUAGE \
  -DLICENSING=$LICENSING \
  -DBUILD_TESTS=$BUILD_TESTS \
  -DBUILD_INTEGRATION_TESTS=$INTEGRATION

echo "[Build] Compiling Dyscover";
cmake --build "$BUILD_DIR" --target Dyscover -j$(nproc)

if [[ $BUILD_TESTS == ON ]]; then
  echo "[Test] Running unit tests";
  ctest --test-dir "$BUILD_DIR" --output-on-failure -R unit-
  if [[ $BENCH == ON ]]; then
    echo "[Bench] Device detection benchmark";
    "$BUILD_DIR/tests/unit/DeviceDetectorBenchmark" || true
    echo "[Bench] Performance smoke test";
    "$BUILD_DIR/tests/unit/PerformanceSmokeTest" || true
  fi
fi

if [[ $PACKAGE == ON ]]; then
  OUT_DIR="dist-linux-$CONFIG"
  rm -rf "$OUT_DIR" && mkdir -p "$OUT_DIR"
  cp "$BUILD_DIR/Dyscover" "$OUT_DIR"/
  # Copy .mo and resource files as needed.
  echo "[Package] Created $OUT_DIR (add resources & .mo files as required)"
fi

echo "[Done] Build complete."