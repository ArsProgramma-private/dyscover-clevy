#!/bin/bash
# Integration Test: Migration Script Dry-Run
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify --dry-run produces correct file move plan without modifying files
#
# Expected behavior:
# - Script runs without errors
# - No files are actually moved
# - Output shows planned moves for all layouts
# - Validation report is generated

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
echo "Migration Dry-Run Integration Test"
echo "=========================================="

# Check prerequisites
if [[ ! -f "${MIGRATION_SCRIPT}" ]]; then
    echo -e "${RED}FAIL: Migration script not found at ${MIGRATION_SCRIPT}${NC}"
    exit 1
fi

if [[ ! -x "${MIGRATION_SCRIPT}" ]]; then
    echo -e "${YELLOW}Making migration script executable${NC}"
    chmod +x "${MIGRATION_SCRIPT}"
fi

# Run dry-run
echo ""
echo "Running migration script with --dry-run..."
OUTPUT=$("${MIGRATION_SCRIPT}" --dry-run 2>&1)
EXIT_CODE=$?

# Check exit code
if [[ ${EXIT_CODE} -ne 0 ]]; then
    echo -e "${RED}FAIL: Script exited with code ${EXIT_CODE}${NC}"
    echo "${OUTPUT}"
    exit 1
fi

# Verify expected output patterns
echo ""
echo "Verifying dry-run output..."

CHECKS_PASSED=0
CHECKS_TOTAL=7

# Check 1: Output contains "DRY RUN" marker
if echo "${OUTPUT}" | grep -q "\[DRY RUN\]"; then
    echo -e "${GREEN}✓${NC} Output contains [DRY RUN] markers"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} Missing [DRY RUN] markers"
fi

# Check 2: Plans to create layout directories
if echo "${OUTPUT}" | grep -q "Would create:"; then
    echo -e "${GREEN}✓${NC} Plans to create layout directories"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} No directory creation plan found"
fi

# Check 3: Plans to move layout definitions
if echo "${OUTPUT}" | grep -q "Would move.*layout definition"; then
    echo -e "${GREEN}✓${NC} Plans to move layout definitions"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} No layout definition move plan found"
fi

# Check 4: Plans to copy audio files
if echo "${OUTPUT}" | grep -q "Would copy.*audio file"; then
    echo -e "${GREEN}✓${NC} Plans to copy audio files"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} No audio file copy plan found"
fi

# Check 5: Plans to copy TTS files
if echo "${OUTPUT}" | grep -q "Would copy.*TTS file"; then
    echo -e "${GREEN}✓${NC} Plans to copy TTS files"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} No TTS file copy plan found"
fi

# Check 6: No actual files were created
if [[ -d "${REPO_ROOT}/res/layouts" ]]; then
    echo -e "${RED}✗${NC} res/layouts/ directory exists (should not in dry-run)"
else
    echo -e "${GREEN}✓${NC} No files were actually created"
    ((CHECKS_PASSED++))
fi

# Check 7: Summary is generated
if echo "${OUTPUT}" | grep -q "Run without --dry-run to execute migration"; then
    echo -e "${GREEN}✓${NC} Summary with execution instructions present"
    ((CHECKS_PASSED++))
else
    echo -e "${RED}✗${NC} Missing execution instructions"
fi

# Final result
echo ""
echo "=========================================="
if [[ ${CHECKS_PASSED} -eq ${CHECKS_TOTAL} ]]; then
    echo -e "${GREEN}PASS: ${CHECKS_PASSED}/${CHECKS_TOTAL} checks passed${NC}"
    exit 0
else
    echo -e "${RED}FAIL: ${CHECKS_PASSED}/${CHECKS_TOTAL} checks passed${NC}"
    echo ""
    echo "Full output:"
    echo "${OUTPUT}"
    exit 1
fi
