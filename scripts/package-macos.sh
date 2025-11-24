#!/usr/bin/env bash
set -euo pipefail

CONFIG=Release
LANGUAGE=nl
LICENSING=demo
UNIVERSAL=0
SIGN_ID="" # e.g. "Developer ID Application: Example Corp (TEAMID)"
NOTARIZE=0
BUNDLE_ROOT="Dyscover.app"
DIST_DIR="dist-macos-${CONFIG}"

usage() {
  cat <<EOF
Usage: $0 [-c Release|Debug] [-l nl|nl_be] [-L demo|full|none] [--universal] [--sign "IDENTITY"] [--notarize]
Creates a macOS .app bundle and optional signed DMG.
Environment variables APPLE_API_KEY / APPLE_API_ISSUER can be used for notarization (if implemented externally).
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -c) CONFIG="$2"; shift 2;;
    -l) LANGUAGE="$2"; shift 2;;
    -L) LICENSING="$2"; shift 2;;
    --universal) UNIVERSAL=1; shift;;
    --sign) SIGN_ID="$2"; shift 2;;
    --notarize) NOTARIZE=1; shift;;
    -h|--help) usage; exit 0;;
    *) echo "Unknown arg: $1"; usage; exit 1;;
  esac
done

BUILD_DIR="build-macos-${CONFIG}";
GEN_ARGS=""
if [[ "$UNIVERSAL" == "1" ]]; then
  GEN_ARGS="-DCMAKE_OSX_ARCHITECTURES=\"x86_64;arm64\""
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="${CONFIG}" -DLANGUAGE="${LANGUAGE}" -DLICENSING="${LICENSING}" ${GEN_ARGS}
cmake --build "$BUILD_DIR" --target Dyscover -j$(sysctl -n hw.ncpu)

# Prepare bundle structure
rm -rf "$DIST_DIR/$BUNDLE_ROOT" || true
mkdir -p "$DIST_DIR/$BUNDLE_ROOT/Contents/MacOS" "$DIST_DIR/$BUNDLE_ROOT/Contents/Resources"

# Copy binary & resources
cp "$BUILD_DIR/Dyscover" "$DIST_DIR/$BUNDLE_ROOT/Contents/MacOS/Dyscover"
cp res/Info.plist "$DIST_DIR/$BUNDLE_ROOT/Contents/Info.plist"
# Icons (choose primary icon)
cp "$BUILD_DIR/Icon.icns" "$DIST_DIR/$BUNDLE_ROOT/Contents/Resources/Icon.icns" || true
# Audio assets
mkdir -p "$DIST_DIR/$BUNDLE_ROOT/Contents/Resources/audio"
cp "$BUILD_DIR"/audio/*.wav "$DIST_DIR/$BUNDLE_ROOT/Contents/Resources/audio/" || true

# TTS data (if present)
if [[ -d "$BUILD_DIR/tts" ]]; then
  cp -R "$BUILD_DIR/tts" "$DIST_DIR/$BUNDLE_ROOT/Contents/Resources/"
fi

# Optional codesign
if [[ -n "$SIGN_ID" ]]; then
  echo "[INFO] Codesigning bundle with identity: $SIGN_ID"
  codesign --force --options runtime --timestamp --sign "$SIGN_ID" "$DIST_DIR/$BUNDLE_ROOT"
fi

# Create DMG
DMG_NAME="Dyscover-${LANGUAGE}-${LICENSING}-${CONFIG}.dmg"
rm -f "$DIST_DIR/$DMG_NAME"
mkdir -p "$DIST_DIR/.dmg-root"
cp -R "$DIST_DIR/$BUNDLE_ROOT" "$DIST_DIR/.dmg-root/"
hdiutil create -volname "Dyscover" -srcfolder "$DIST_DIR/.dmg-root" -ov -format UDZO "$DIST_DIR/$DMG_NAME"
rm -rf "$DIST_DIR/.dmg-root"

if [[ -n "$SIGN_ID" ]]; then
  echo "[INFO] Codesigning DMG"
  codesign --force --sign "$SIGN_ID" "$DIST_DIR/$DMG_NAME"
fi

if [[ "$NOTARIZE" == "1" && -n "$SIGN_ID" ]]; then
  echo "[INFO] (Template) Notarization step NOT implemented; please run:
    xcrun notarytool submit $DIST_DIR/$DMG_NAME --key $APPLE_API_KEY --issuer $APPLE_API_ISSUER --wait
    xcrun stapler staple $DIST_DIR/$DMG_NAME" >&2
fi

echo "[OK] macOS bundle + DMG ready in $DIST_DIR"