#!/usr/bin/env bash
# Example helper to sign an existing macOS Dyscover.app bundle.
# Replace TEAMID and identity with your actual Developer ID Application cert.
# Usage: ./scripts/macos-codesign-example.sh path/to/Dyscover.app "Developer ID Application: Example Corp (TEAMID)"
set -euo pipefail
if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <bundle-path> <identity>" >&2
  exit 1
fi
BUNDLE="$1"
IDENTITY="$2"

if [[ ! -d "$BUNDLE" ]]; then
  echo "Bundle not found: $BUNDLE" >&2
  exit 1
fi

echo "Signing: $BUNDLE"
# Sign nested binaries first if any
find "$BUNDLE" -type f -perm +111 -print0 2>/dev/null | while IFS= read -r -d '' f; do
  echo "  codesign: $f"
  codesign --force --options runtime --sign "$IDENTITY" "$f"
done
# Sign the bundle root
codesign --force --options runtime --sign "$IDENTITY" "$BUNDLE"
# Verify
codesign --verify --deep --strict "$BUNDLE" && echo "Verification OK"
