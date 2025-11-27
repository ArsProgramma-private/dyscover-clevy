#!/bin/bash
# Integration Test: CMake Layout Validation
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Verify validate_layout_structure() catches invalid layouts
#
# Expected behavior:
# - Valid layouts pass validation
# - Invalid layouts (missing files/dirs) fail validation
# - Error messages are descriptive

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "CMake Layout Validation Test"
echo "=========================================="

# Create temporary test directory
TEST_DIR=$(mktemp -d)
echo "Test directory: ${TEST_DIR}"

cleanup() {
    rm -rf "${TEST_DIR}"
}
trap cleanup EXIT

# Create test layout structures
mkdir -p "${TEST_DIR}/valid_layout"
mkdir -p "${TEST_DIR}/valid_layout/audio"
mkdir -p "${TEST_DIR}/valid_layout/tts"
touch "${TEST_DIR}/valid_layout/layout.cpp"
touch "${TEST_DIR}/valid_layout/audio/test.wav"
touch "${TEST_DIR}/valid_layout/tts/nl_nl.db"
touch "${TEST_DIR}/valid_layout/tts/nl_nl.fsa"
touch "${TEST_DIR}/valid_layout/tts/nl_nl.fst"

mkdir -p "${TEST_DIR}/missing_layoutcpp/audio"
mkdir -p "${TEST_DIR}/missing_layoutcpp/tts"
touch "${TEST_DIR}/missing_layoutcpp/audio/test.wav"

mkdir -p "${TEST_DIR}/missing_audio"
mkdir -p "${TEST_DIR}/missing_audio/tts"
touch "${TEST_DIR}/missing_audio/layout.cpp"

mkdir -p "${TEST_DIR}/empty_audio"
mkdir -p "${TEST_DIR}/empty_audio/audio"
mkdir -p "${TEST_DIR}/empty_audio/tts"
touch "${TEST_DIR}/empty_audio/layout.cpp"

# Create test CMake script
TEST_CMAKE=$(mktemp)
cat > "${TEST_CMAKE}" << EOF
cmake_minimum_required(VERSION 3.15)
project(ValidationTest)

include(${REPO_ROOT}/scripts/build-tools/validate-layout-structure.cmake)

# Test 1: Valid layout
validate_layout_structure("${TEST_DIR}/valid_layout" RESULT ERROR)
if(NOT RESULT)
    message(FATAL_ERROR "Test 1 FAILED: Valid layout rejected: \${ERROR}")
endif()
message(STATUS "✓ Test 1 PASSED: Valid layout accepted")

# Test 2: Missing layout.cpp
validate_layout_structure("${TEST_DIR}/missing_layoutcpp" RESULT ERROR)
if(RESULT)
    message(FATAL_ERROR "Test 2 FAILED: Missing layout.cpp not detected")
endif()
message(STATUS "✓ Test 2 PASSED: Missing layout.cpp detected: \${ERROR}")

# Test 3: Missing audio directory
validate_layout_structure("${TEST_DIR}/missing_audio" RESULT ERROR)
if(RESULT)
    message(FATAL_ERROR "Test 3 FAILED: Missing audio/ not detected")
endif()
message(STATUS "✓ Test 3 PASSED: Missing audio/ detected: \${ERROR}")

# Test 4: Empty audio directory
validate_layout_structure("${TEST_DIR}/empty_audio" RESULT ERROR)
if(RESULT)
    message(FATAL_ERROR "Test 4 FAILED: Empty audio/ not detected")
endif()
message(STATUS "✓ Test 4 PASSED: Empty audio/ detected: \${ERROR}")

message(STATUS "All validation tests PASSED")
EOF

# Run CMake test
echo ""
echo "Running CMake validation tests..."
if cmake -P "${TEST_CMAKE}" 2>&1 | tee /tmp/cmake_validation_test_$$.log; then
    echo ""
    echo -e "${GREEN}PASS: All validation tests succeeded${NC}"
    rm -f "${TEST_CMAKE}" /tmp/cmake_validation_test_$$.log
    exit 0
else
    echo ""
    echo -e "${RED}FAIL: Validation tests failed${NC}"
    cat /tmp/cmake_validation_test_$$.log
    rm -f "${TEST_CMAKE}" /tmp/cmake_validation_test_$$.log
    exit 1
fi
