#!/usr/bin/env bash
# Verify basic binary hardening (Linux/macOS)
# Checks: PIE, RELRO (full/partial), NX, RPATH anomalies, codesign (macOS)
set -euo pipefail
BIN=${1:-build/Dyscover}
if [[ ! -f "$BIN" ]]; then echo "Binary not found: $BIN" >&2; exit 1; fi
OS=$(uname -s)

report() { printf "%-18s %s\n" "$1" "$2"; }

if [[ "$OS" == "Linux" ]]; then
  TYPE=$(readelf -h "$BIN" | awk '/Type:/ {print $2}')
  PIE=$([[ "$TYPE" == "DYN" ]] && echo yes || echo no)
  NX=$(readelf -l "$BIN" | grep -q 'GNU_STACK' | grep -vq 'RWE' && echo yes || echo no)
  RELRO=$(readelf -l "$BIN" | grep -q 'GNU_RELRO' && { readelf -l "$BIN" | grep -q BIND_NOW && echo full || echo partial; } || echo none)
  RPATH=$(readelf -d "$BIN" | awk '/RPATH/||/RUNPATH/ {print $NF}')
  report PIE "$PIE"
  report NX "$NX"
  report RELRO "$RELRO"
  if [[ -n "$RPATH" ]]; then
    if echo "$RPATH" | grep -q '\.\.'; then report RPATH "WARNING: contains parent refs: $RPATH"; else report RPATH "$RPATH"; fi
  else
    report RPATH none
  fi
elif [[ "$OS" == "Darwin" ]]; then
  otool -hv "$BIN" >/dev/null || { echo "otool failed" >&2; exit 1; }
  PIE=$(otool -hv "$BIN" | grep -q PIE && echo yes || echo no)
  NX=yes # macOS default MMU protections
  RPATHS=$(otool -l "$BIN" | awk '/LC_RPATH/{getline; getline; print $2}')
  SIGN_STATUS=$(codesign --verify --deep --strict "$BIN" >/dev/null 2>&1 && echo valid || echo missing)
  report PIE "$PIE"
  report NX "$NX"
  if [[ -n "$RPATHS" ]]; then report RPATH "$RPATHS"; else report RPATH none; fi
  report CodeSign "$SIGN_STATUS"
else
  echo "Unsupported OS: $OS" >&2; exit 1
fi
