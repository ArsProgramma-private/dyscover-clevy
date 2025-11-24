#!/usr/bin/env bash
# Dependency audit script (Linux/macOS)
# Lists dynamic library dependencies and flags missing ones.
set -euo pipefail
TARGET=${1:-Dyscover}
BUILD_DIR=${2:-build}
BIN="$BUILD_DIR/$TARGET"
if [[ ! -f "$BIN" ]]; then
  echo "Binary not found: $BIN" >&2
  exit 1
fi
OS=$(uname -s)
sha256() { shasum -a 256 "$1" 2>/dev/null | awk '{print $1}'; }
if [[ "$OS" == "Linux" ]]; then
  echo "[INFO] Auditing (ldd): $BIN"
  ldd "$BIN" | sed 's/^\t//' | while read -r line; do
    if echo "$line" | grep -q 'not found'; then
      echo "[MISSING] $line"
    else
      path=$(echo "$line" | awk '{print $3}')
      if [[ -f "$path" ]]; then
        sum=$(sha256 "$path")
        echo "[OK] $line SHA256=$sum" | sed 's/ (0x[0-9a-f]*)//' 
      else
        echo "[OK] $line"
      fi
    fi
  done
  # RPATH/RUNPATH check
  RP=$(readelf -d "$BIN" 2>/dev/null | awk '/RPATH/||/RUNPATH/ {print $NF}')
  if [[ -n "$RP" ]]; then echo "[RPATH] $RP"; fi
elif [[ "$OS" == "Darwin" ]]; then
  echo "[INFO] Auditing (otool -L): $BIN"
  otool -L "$BIN" | tail -n +2 | while read -r line; do
    lib=$(echo "$line" | awk '{print $1}')
    if [[ -f "$lib" || "$lib" == @* ]]; then
      if [[ -f "$lib" ]]; then sum=$(sha256 "$lib"); fi
      echo "[OK] $lib SHA256=${sum:-n/a}" 
    else
      echo "[CHECK] $lib (verify exists in runtime path)"
    fi
  done
  otool -l "$BIN" | awk '/LC_RPATH/{getline;getline;print "[LC_RPATH] "$2}'
else
  echo "Unsupported OS: $OS" >&2
  exit 1
fi
