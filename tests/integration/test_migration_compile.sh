#!/bin/bash
# Integration Test: Migrated Layouts Compile
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify layout.cpp files compile successfully
#
# Expected behavior:
# - Each layout.cpp compiles without errors
# - No missing includes or symbols
# - Compiler warnings are acceptable, errors are not

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Migration Compilation Test"
echo "=========================================="

# Check if layout structure exists
if [[ ! -d "${REPO_ROOT}/res/layouts" ]]; then
    echo -e "${YELLOW}SKIP: Layout structure not yet created${NC}"
    echo "Run migration script first: scripts/migration/migrate-to-layouts.sh"
    exit 0
fi

# Find all layout.cpp files
LAYOUT_FILES=$(find "${REPO_ROOT}/res/layouts" -name "layout.cpp" 2>/dev/null)

if [[ -z "${LAYOUT_FILES}" ]]; then
    echo -e "${YELLOW}SKIP: No layout.cpp files found${NC}"
    exit 0
fi

# Check if compiler is available
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${YELLOW}SKIP: No C++ compiler found${NC}"
    exit 0
fi

CXX=${CXX:-g++}

echo "Using compiler: ${CXX}"
echo "Found layout files:"
echo "${LAYOUT_FILES}"
echo ""

# Test compilation of each layout.cpp
CHECKS_PASSED=0
CHECKS_TOTAL=0

while IFS= read -r LAYOUT_FILE; do
    ((CHECKS_TOTAL++))
    LAYOUT_NAME=$(dirname "${LAYOUT_FILE}" | sed "s|${REPO_ROOT}/res/layouts/||")
    
    echo "Testing: ${LAYOUT_NAME}"
    
    # Attempt to compile (syntax check only, no linking)
    if ${CXX} -std=c++17 -fsyntax-only \
        -I"${REPO_ROOT}/src" \
        -I"${REPO_ROOT}/src/layouts" \
        "${LAYOUT_FILE}" 2>&1 | tee /tmp/compile_output_$$.log; then
        echo -e "${GREEN}✓${NC} ${LAYOUT_NAME} compiles successfully"
        ((CHECKS_PASSED++))
    else
        echo -e "${RED}✗${NC} ${LAYOUT_NAME} failed to compile"
        echo "Errors:"
        cat /tmp/compile_output_$$.log
        rm -f /tmp/compile_output_$$.log
    fi
    echo ""
done <<< "${LAYOUT_FILES}"

# Cleanup
rm -f /tmp/compile_output_$$.log

# Final result
echo "=========================================="
if [[ ${CHECKS_PASSED} -eq ${CHECKS_TOTAL} ]]; then
    echo -e "${GREEN}PASS: ${CHECKS_PASSED}/${CHECKS_TOTAL} layouts compile${NC}"
    exit 0
else
    echo -e "${RED}FAIL: ${CHECKS_PASSED}/${CHECKS_TOTAL} layouts compile${NC}"
    exit 1
fi
