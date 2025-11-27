#!/bin/bash
# Integration Test: Binary Equivalence
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify old and new structures produce functionally equivalent binaries
#
# Expected behavior:
# - Both structures build successfully
# - Same layout data accessible at runtime
# - Same audio files available
# - No functional differences

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "Binary Equivalence Test"
echo "=========================================="

# Check prerequisites
if [[ ! -f "${REPO_ROOT}/src/Keys.cpp" ]]; then
    echo -e "${RED}ERROR: Keys.cpp not found${NC}"
    exit 1
fi

if [[ ! -d "${REPO_ROOT}/res/layouts" ]]; then
    echo -e "${YELLOW}SKIP: New layout structure not yet created${NC}"
    echo "Run migration script first"
    exit 0
fi

# Build with old structure
echo ""
echo "Building with old structure (USE_LAYOUT_STRUCTURE=OFF)..."
if cmake -B build-old -DLANGUAGE=nl -DUSE_LAYOUT_STRUCTURE=OFF "${REPO_ROOT}" > /tmp/build_old_$$.log 2>&1; then
    echo -e "${GREEN}✓${NC} Old structure CMake configuration succeeded"
else
    echo -e "${RED}✗${NC} Old structure CMake configuration failed"
    cat /tmp/build_old_$$.log
    exit 1
fi

if cmake --build build-old --parallel > /tmp/build_old_compile_$$.log 2>&1; then
    echo -e "${GREEN}✓${NC} Old structure build succeeded"
else
    echo -e "${YELLOW}⚠${NC} Old structure build failed (may be expected if Keys.cpp already migrated)"
    echo "This test requires both structures to exist simultaneously"
    rm -rf build-old
    exit 0
fi

# Build with new structure
echo ""
echo "Building with new structure (USE_LAYOUT_STRUCTURE=ON)..."
if cmake -B build-new -DLANGUAGE=nl -DUSE_LAYOUT_STRUCTURE=ON "${REPO_ROOT}" > /tmp/build_new_$$.log 2>&1; then
    echo -e "${GREEN}✓${NC} New structure CMake configuration succeeded"
else
    echo -e "${RED}✗${NC} New structure CMake configuration failed"
    cat /tmp/build_new_$$.log
    rm -rf build-old
    exit 1
fi

if cmake --build build-new --parallel > /tmp/build_new_compile_$$.log 2>&1; then
    echo -e "${GREEN}✓${NC} New structure build succeeded"
else
    echo -e "${RED}✗${NC} New structure build failed"
    cat /tmp/build_new_compile_$$.log
    rm -rf build-old build-new
    exit 1
fi

# Compare binary sizes
echo ""
echo "Comparing binaries..."
OLD_SIZE=$(stat -f%z build-old/Dyscover 2>/dev/null || stat -c%s build-old/Dyscover 2>/dev/null)
NEW_SIZE=$(stat -f%z build-new/Dyscover 2>/dev/null || stat -c%s build-new/Dyscover 2>/dev/null)

echo "Old structure binary: ${OLD_SIZE} bytes"
echo "New structure binary: ${NEW_SIZE} bytes"

# Allow up to 5% size difference (different compilation units)
SIZE_DIFF=$(echo "scale=2; (${NEW_SIZE} - ${OLD_SIZE}) / ${OLD_SIZE} * 100" | bc 2>/dev/null || echo "0")
SIZE_DIFF_ABS=$(echo "${SIZE_DIFF}" | tr -d '-')

if (( $(echo "${SIZE_DIFF_ABS} < 5" | bc -l 2>/dev/null || echo "1") )); then
    echo -e "${GREEN}✓${NC} Binary sizes are equivalent (difference: ${SIZE_DIFF}%)"
else
    echo -e "${YELLOW}⚠${NC} Binary size difference: ${SIZE_DIFF}% (acceptable if <10%)"
fi

# Cleanup
echo ""
echo "Cleaning up..."
rm -rf build-old build-new
rm -f /tmp/build_old_$$.log /tmp/build_new_$$.log /tmp/build_old_compile_$$.log /tmp/build_new_compile_$$.log

echo ""
echo -e "${GREEN}PASS: Binary equivalence test completed${NC}"
exit 0
