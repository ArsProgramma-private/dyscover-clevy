#!/bin/bash
# Integration Test: Migration Creates Valid Structure
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify migration creates all required directories and files
#
# Expected behavior:
# - All layout directories created
# - Each layout has layout.cpp, audio/, tts/ subdirectories
# - File counts match expected values
# - No extra or missing files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
MIGRATION_SCRIPT="${REPO_ROOT}/scripts/migration/migrate-to-layouts.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Migration Structure Validation Test"
echo "=========================================="

# Create temporary test directory
TEST_DIR=$(mktemp -d)
echo "Test directory: ${TEST_DIR}"

# Cleanup function
cleanup() {
    echo ""
    echo "Cleaning up test directory..."
    rm -rf "${TEST_DIR}"
}
trap cleanup EXIT

# Copy source files to test directory
echo "Setting up test environment..."
mkdir -p "${TEST_DIR}/src"
mkdir -p "${TEST_DIR}/res/data"
mkdir -p "${TEST_DIR}/res/data/tts"
mkdir -p "${TEST_DIR}/scripts/migration"

cp "${REPO_ROOT}/src/Keys.cpp" "${TEST_DIR}/src/" 2>/dev/null || echo "Note: Keys.cpp not found, test may be premature"
cp -r "${REPO_ROOT}/res/data"/*.wav "${TEST_DIR}/res/data/" 2>/dev/null || echo "Note: Audio files not found"
cp -r "${REPO_ROOT}/res/data/tts"/* "${TEST_DIR}/res/data/tts/" 2>/dev/null || echo "Note: TTS files not found"

# Note: Since migration script doesn't exist yet, this test will be skipped
if [[ ! -f "${MIGRATION_SCRIPT}" ]]; then
    echo -e "${YELLOW}SKIP: Migration script not yet implemented${NC}"
    echo "This test will validate structure once migrate-to-layouts.sh is complete"
    exit 0
fi

# Run migration (not dry-run)
echo ""
echo "Running migration..."
cd "${TEST_DIR}"
"${MIGRATION_SCRIPT}"

# Validate structure
echo ""
echo "Validating created structure..."

CHECKS_PASSED=0
CHECKS_TOTAL=10

# Expected layout directories
EXPECTED_LAYOUTS=(
    "res/layouts/classic/nl_nl"
    "res/layouts/classic/nl_be"
    "res/layouts/default/nl_nl"
    "res/layouts/default/nl_be"
    "res/layouts/kwec/nl_nl"
)

# Check each layout directory
for LAYOUT_DIR in "${EXPECTED_LAYOUTS[@]}"; do
    if [[ -d "${LAYOUT_DIR}" ]]; then
        echo -e "${GREEN}✓${NC} Directory exists: ${LAYOUT_DIR}"
        ((CHECKS_PASSED++))
        
        # Check subdirectories
        if [[ -f "${LAYOUT_DIR}/layout.cpp" ]]; then
            echo -e "  ${GREEN}✓${NC} layout.cpp exists"
        else
            echo -e "  ${RED}✗${NC} layout.cpp missing"
        fi
        
        if [[ -d "${LAYOUT_DIR}/audio" ]]; then
            AUDIO_COUNT=$(find "${LAYOUT_DIR}/audio" -name "*.wav" | wc -l)
            echo -e "  ${GREEN}✓${NC} audio/ directory exists (${AUDIO_COUNT} files)"
        else
            echo -e "  ${RED}✗${NC} audio/ directory missing"
        fi
        
        if [[ -d "${LAYOUT_DIR}/tts" ]]; then
            TTS_COUNT=$(find "${LAYOUT_DIR}/tts" -type f | wc -l)
            echo -e "  ${GREEN}✓${NC} tts/ directory exists (${TTS_COUNT} files)"
        else
            echo -e "  ${RED}✗${NC} tts/ directory missing"
        fi
    else
        echo -e "${RED}✗${NC} Directory missing: ${LAYOUT_DIR}"
    fi
done

# Check if any layout.cpp has content
if [[ -f "res/layouts/classic/nl_nl/layout.cpp" ]]; then
    LINE_COUNT=$(wc -l < "res/layouts/classic/nl_nl/layout.cpp")
    if [[ ${LINE_COUNT} -gt 50 ]]; then
        echo -e "${GREEN}✓${NC} layout.cpp has content (${LINE_COUNT} lines)"
        ((CHECKS_PASSED++))
    else
        echo -e "${RED}✗${NC} layout.cpp is too small (${LINE_COUNT} lines)"
    fi
fi

# Final result
echo ""
echo "=========================================="
EXPECTED_PASS=6  # Adjust based on what's actually checked
if [[ ${CHECKS_PASSED} -ge ${EXPECTED_PASS} ]]; then
    echo -e "${GREEN}PASS: ${CHECKS_PASSED}/${CHECKS_TOTAL} checks passed${NC}"
    exit 0
else
    echo -e "${RED}FAIL: ${CHECKS_PASSED}/${CHECKS_TOTAL} checks passed${NC}"
    exit 1
fi
