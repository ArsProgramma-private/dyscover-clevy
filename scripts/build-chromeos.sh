#!/usr/bin/env bash
set -euo pipefail
# build-chromeos.sh - ChromeOS (Crostini) build wrapper.
# Reuses Linux build parameters; adds sandbox capability notes.
# Usage: ./scripts/build-chromeos.sh [same flags as build-linux.sh]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Delegate to linux script
"$SCRIPT_DIR/build-linux.sh" "$@"

echo "[ChromeOS] NOTE: If udev or libpulse access is restricted, device detection may fallback to polling and volume control may be unsupported."