#!/usr/bin/env bash
set -euo pipefail

# Quick, repeatable debug build script for Linux.
# Usage:
#   ./scripts/quick-build-linux.sh configure   # configure build-debug (Ninja, Debug)
#   ./scripts/quick-build-linux.sh build       # incremental build (parallel)
#   ./scripts/quick-build-linux.sh test        # run ctest (all tests)
#   ./scripts/quick-build-linux.sh run         # run the built Dyscover binary
#   ./scripts/quick-build-linux.sh clean       # remove build-debug dir

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build-debug"
JOBS=${JOBS:-$(nproc || echo 4)}

# Use ccache if available for faster incremental builds
if command -v ccache &>/dev/null; then
  CCACHE=1
else
  CCACHE=0
fi

# Global flags passed to script (e.g. --force)
FORCE=0
for arg in "$@"; do
  if [ "$arg" = "--force" ]; then
    FORCE=1
  fi
done

configure() {
  echo "Configuring (Ninja generator, Debug) into: $BUILD_DIR"
  # If a CMake cache exists, detect if it was created on a different platform/path.
  if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    prev_src=$(grep -m1 '^CMAKE_HOME_DIRECTORY:INTERNAL=' "$BUILD_DIR/CMakeCache.txt" | cut -d= -f2- || true)
    if [ -n "$prev_src" ]; then
      # detect Windows-style paths (e.g. "C:/path" or "C:\path" or a prefixed U:/)
      if echo "$prev_src" | grep -qE '^[A-Za-z]:[\\/]|^[A-Za-z]:$' || echo "$prev_src" | grep -qE '^[A-Za-z]:'; then
        echo "\nWarning: existing CMake cache in '$BUILD_DIR' was generated for a different path: $prev_src"
        echo "This commonly happens when the same build directory was used on Windows (drive-letter) and Linux." 
        echo "Keeping that cache will cause path mismatches and failed configure/builds." 
        if [ "${QUICK_BUILD_FORCE:-0}" = "1" ] || [ "${FORCE:-0}" = "1" ]; then
          echo "Removing stale build directory: $BUILD_DIR (forced)"
          rm -rf "$BUILD_DIR"
        else
          echo "To proceed, either remove $BUILD_DIR yourself or re-run this command with '--force' or set QUICK_BUILD_FORCE=1 to auto-delete."
          exit 1
        fi
      fi
    fi
  fi
  mkdir -p "$BUILD_DIR"
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    $( [ "$CCACHE" -eq 1 ] && echo "-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache" )
}

build() {
  echo "Building target(s) (ninja -C $BUILD_DIR -j$JOBS)"
  # Ninja is extremely fast for incremental builds
  cmake --build "$BUILD_DIR" -- -j"$JOBS"
}

test_all() {
  echo "Running ctest (in: $BUILD_DIR)"
  cmake --build "$BUILD_DIR" --target test -- -j"$JOBS" || true
}

run() {
  if [ ! -x "$BUILD_DIR/Dyscover" ]; then
    echo "Executable not found in $BUILD_DIR. Build first: ./scripts/quick-build-linux.sh build"
    exit 1
  fi

  echo "Running built binary: $BUILD_DIR/Dyscover"
  # Allow library lookups from the build directory if librstts etc are placed there.
  LD_LIBRARY_PATH="$BUILD_DIR":${LD_LIBRARY_PATH-}
  (cd "$BUILD_DIR" && LD_LIBRARY_PATH="$LD_LIBRARY_PATH" ./Dyscover)
}

clean() {
  echo "Removing $BUILD_DIR"
  rm -rf "$BUILD_DIR"
}

case "${1:-}" in
  configure) configure ;;
  build) build ;;
  test) test_all ;;
  run) run ;;
  clean) clean ;;
  "") echo "No subcommand provided; configure, build, test, run, clean"; exit 1 ;;
  *) echo "Unknown subcommand: $1"; exit 1 ;;
esac
