# Layout Discovery Module
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Discover and validate layout-based resource directories
#
# This module provides functions for:
# - Discovering layout.cpp files in res/layouts/*/* structure
# - Validating layout directory structure (audio/, tts/, layout.cpp)
# - Extracting audio references from layout files
# - Generating layout installation manifests
#
# See: specs/006-language-resource-optimization/contracts/layout-api.md

# Include build tools
include("${CMAKE_SOURCE_DIR}/scripts/build-tools/discover-layouts.cmake")
include("${CMAKE_SOURCE_DIR}/scripts/build-tools/validate-layout-structure.cmake")
include("${CMAKE_SOURCE_DIR}/scripts/build-tools/extract-audio-references.cmake")

# Function: generate_layout_manifest
# Create install manifest for a specific layout
#
# Arguments:
#   LAYOUT_DIR - Path to layout directory
#   LANGUAGE - Language code
#   LAYOUT_TYPE - Layout type (classic/default/kwec)
#   OUTPUT_FILE - Path to output manifest file
function(generate_layout_manifest LAYOUT_DIR LANGUAGE LAYOUT_TYPE OUTPUT_FILE)
  # Validate inputs
  if(NOT EXISTS "${LAYOUT_DIR}")
    message(FATAL_ERROR "Layout directory does not exist: ${LAYOUT_DIR}")
  endif()

  # Collect all files to include in manifest
  set(MANIFEST_ENTRIES "")

  # Add layout.cpp
  if(EXISTS "${LAYOUT_DIR}/layout.cpp")
    list(APPEND MANIFEST_ENTRIES "layout.cpp")
  endif()

  # Add all audio files
  file(GLOB AUDIO_FILES RELATIVE "${LAYOUT_DIR}" "${LAYOUT_DIR}/audio/*.wav")
  foreach(AUDIO ${AUDIO_FILES})
    list(APPEND MANIFEST_ENTRIES "${AUDIO}")
  endforeach()

  # Add all TTS files
  file(GLOB TTS_FILES RELATIVE "${LAYOUT_DIR}" "${LAYOUT_DIR}/tts/*")
  foreach(TTS ${TTS_FILES})
    # Only add files, not directories
    if(NOT IS_DIRECTORY "${LAYOUT_DIR}/${TTS}")
      list(APPEND MANIFEST_ENTRIES "${TTS}")
    endif()
  endforeach()

  # Write manifest to file
  file(WRITE "${OUTPUT_FILE}" "")
  foreach(ENTRY ${MANIFEST_ENTRIES})
    file(APPEND "${OUTPUT_FILE}" "${ENTRY}\n")
  endforeach()

  message(STATUS "Generated layout manifest: ${OUTPUT_FILE}")
endfunction()

message(STATUS "LayoutDiscovery module loaded")

