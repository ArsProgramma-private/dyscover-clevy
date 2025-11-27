#!/bin/bash
# Integration Test: CMake Layout Discovery
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify discover_layouts() finds all layout.cpp files
#
# Expected behavior:
# - Discovers all layout directories for selected language
# - Returns valid layout.cpp paths
# - Validates each discovered layout

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "CMake Layout Discovery Test"
echo "=========================================="

# Check if layout structure exists
if [[ ! -d "${REPO_ROOT}/res/layouts" ]]; then
    echo -e "${YELLOW}SKIP: Layout structure not yet created${NC}"
    echo "Run migration script first: scripts/migration/migrate-to-layouts.sh"
    exit 0
fi

# Create test CMake script
TEST_CMAKE=$(mktemp)
cat > "${TEST_CMAKE}" << 'EOF'
cmake_minimum_required(VERSION 3.15)
project(LayoutDiscoveryTest)

# Include discovery module
include(${CMAKE_SOURCE_DIR}/cmake/LayoutDiscovery.cmake)

# Test discovery for Dutch
discover_layouts(NL_LAYOUTS "nl")
message(STATUS "Dutch layouts found: ${NL_LAYOUTS}")

list(LENGTH NL_LAYOUTS NL_COUNT)
message(STATUS "Dutch layout count: ${NL_COUNT}")

# Test discovery for Flemish
discover_layouts(NL_BE_LAYOUTS "nl_be")
message(STATUS "Flemish layouts found: ${NL_BE_LAYOUTS}")

list(LENGTH NL_BE_LAYOUTS NL_BE_COUNT)
message(STATUS "Flemish layout count: ${NL_BE_COUNT}")

# Expected counts
set(EXPECTED_NL_COUNT 3)  # classic, default, kwec
set(EXPECTED_NL_BE_COUNT 2)  # classic, default

if(NOT NL_COUNT EQUAL EXPECTED_NL_COUNT)
    message(FATAL_ERROR "Expected ${EXPECTED_NL_COUNT} Dutch layouts, found ${NL_COUNT}")
endif()

if(NOT NL_BE_COUNT EQUAL EXPECTED_NL_BE_COUNT)
    message(FATAL_ERROR "Expected ${EXPECTED_NL_BE_COUNT} Flemish layouts, found ${NL_BE_COUNT}")
endif()

message(STATUS "✓ Discovery test PASSED")
EOF

# Run CMake test
echo ""
echo "Running CMake discovery test..."
cd "${REPO_ROOT}"
if cmake -P "${TEST_CMAKE}" 2>&1 | tee /tmp/cmake_discovery_test_$$.log; then
    echo ""
    echo -e "${GREEN}PASS: Layout discovery test succeeded${NC}"
    rm -f "${TEST_CMAKE}" /tmp/cmake_discovery_test_$$.log
    exit 0
else
    echo ""
    echo -e "${RED}FAIL: Layout discovery test failed${NC}"
    cat /tmp/cmake_discovery_test_$$.log
    rm -f "${TEST_CMAKE}" /tmp/cmake_discovery_test_$$.log
    exit 1
fi
