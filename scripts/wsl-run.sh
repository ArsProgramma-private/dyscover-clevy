#!/usr/bin/env bash
set -euo pipefail

# Small helper to compute reasonable DISPLAY and PULSE_SERVER values when running inside WSL
# Usage: scripts/wsl-run.sh [--no-build]

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build-debug"

if [ "${1:-}" = "--no-build" ]; then
  NO_BUILD=1
else
  NO_BUILD=0
fi

# Compute DISPLAY fallback (use existing DISPLAY if present, otherwise attempt host IP):
if [ -n "${DISPLAY-}" ]; then
  DISPLAY_VAL="$DISPLAY"
else
  # Use the first nameserver from /etc/resolv.conf which is typically the Windows host IP in WSL
  host_ip=$(grep -m1 nameserver /etc/resolv.conf | awk '{print $2}') || host_ip=""
  if [ -n "$host_ip" ]; then
    DISPLAY_VAL="$host_ip:0"
  else
    DISPLAY_VAL=":0"
  fi
fi

# Compute PULSE_SERVER fallback: prefer existing, then runtime socket under /run/user/<uid>/pulse/native
if [ -n "${PULSE_SERVER-}" ]; then
  PULSE_VAL="$PULSE_SERVER"
elif [ -S "/run/user/$(id -u)/pulse/native" ]; then
  PULSE_VAL="/run/user/$(id -u)/pulse/native"
else
  PULSE_VAL=""
fi

echo "[wsl-run] DISPLAY=$DISPLAY_VAL PULSE_SERVER=${PULSE_VAL:-<none>}"

export DISPLAY="$DISPLAY_VAL"
if [ -n "$PULSE_VAL" ]; then
  export PULSE_SERVER="$PULSE_VAL"
fi

if [ "$NO_BUILD" -eq 0 ]; then
  # Ensure build exists and is up-to-date
  "$ROOT_DIR/scripts/quick-build-linux.sh" build
fi

# Run the binary from build tree
if [ -x "$BUILD_DIR/Dyscover" ]; then
  LD_LIBRARY_PATH="$BUILD_DIR":${LD_LIBRARY_PATH-} (cd "$BUILD_DIR" && LD_LIBRARY_PATH="$LD_LIBRARY_PATH" ./Dyscover)
else
  echo "Executable not found in $BUILD_DIR. Build first: ./scripts/quick-build-linux.sh configure && ./scripts/quick-build-linux.sh build"
  exit 1
fi
