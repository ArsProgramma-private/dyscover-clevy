#!/usr/bin/env bash
# Generate CycloneDX SBOM + license matrix using syft (if available)
# Usage: ./scripts/generate-sbom.sh build/Dyscover
set -euo pipefail
TARGET=${1:-build/Dyscover}
OUT_DIR="sbom"
mkdir -p "$OUT_DIR"
if ! command -v syft >/dev/null 2>&1; then
  echo "[WARN] syft not found. Install with: curl -sSfL https://raw.githubusercontent.com/anchore/syft/main/install.sh | sh" >&2
  exit 1
fi
if [[ ! -f "$TARGET" ]]; then echo "Binary not found: $TARGET" >&2; exit 1; fi

# CycloneDX JSON
syft packages "$TARGET" -o cyclonedx-json > "$OUT_DIR/cyclonedx.json"
# Table output includes license info; extract into CSV
syft packages "$TARGET" -o table > "$OUT_DIR/table.txt"
awk 'BEGIN{FS="|";OFS=","} NR>2 {gsub(/^ +| +$/,"",$0); if($2!="" && $4!="") print $2,$4,$6}' "$OUT_DIR/table.txt" > "$OUT_DIR/licenses.csv" || true

echo "[OK] SBOM written to $OUT_DIR/cyclonedx.json and license matrix $OUT_DIR/licenses.csv"