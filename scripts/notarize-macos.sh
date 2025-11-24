#!/usr/bin/env bash
# macOS notarization helper (requires prior codesign)
# Usage: APPLE_NOTARY_KEY_ID=... APPLE_NOTARY_ISSUER=... APPLE_NOTARY_KEY_FILE=AuthKey.p8 \
#        ./scripts/notarize-macos.sh dist-macos-Release/Dyscover-*.dmg
set -euo pipefail
if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <dmg-or-zip-path>" >&2; exit 1; fi
ARTIFACT="$1"
if [[ ! -f "$ARTIFACT" ]]; then echo "Artifact not found: $ARTIFACT" >&2; exit 1; fi
: "${APPLE_NOTARY_KEY_ID:?APPLE_NOTARY_KEY_ID env required}" 
: "${APPLE_NOTARY_ISSUER:?APPLE_NOTARY_ISSUER env required}" 
: "${APPLE_NOTARY_KEY_FILE:?APPLE_NOTARY_KEY_FILE env required}" 

echo "[INFO] Submitting for notarization: $ARTIFACT"
REQUEST_ID=$(xcrun notarytool submit "$ARTIFACT" \
  --key "$APPLE_NOTARY_KEY_FILE" \
  --key-id "$APPLE_NOTARY_KEY_ID" \
  --issuer "$APPLE_NOTARY_ISSUER" --wait --output-format json | jq -r '.id')

if [[ -z "$REQUEST_ID" || "$REQUEST_ID" == "null" ]]; then
  echo "[ERROR] Failed to parse notarization request id" >&2; exit 1; fi

echo "[INFO] Stapling: $ARTIFACT"
xcrun stapler staple "$ARTIFACT" || { echo "[ERROR] Staple failed" >&2; exit 1; }

echo "[OK] Notarization + staple complete (Request ID: $REQUEST_ID)"