# Discover Layouts Build Tool
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Find all layout.cpp files matching selected LANGUAGE
#
# Function: discover_layouts(OUTPUT_VAR LANGUAGE [LAYOUT_ROOT])
#
# Returns list of absolute paths to layout.cpp files
#
# See: specs/006-language-resource-optimization/contracts/layout-api.md

# Include validation function
include("${CMAKE_CURRENT_LIST_DIR}/validate-layout-structure.cmake")

function(discover_layouts OUTPUT_VAR LANGUAGE)
  # Optional third argument: LAYOUT_ROOT (defaults to CMAKE_SOURCE_DIR/res/layouts)
  if(ARGC GREATER 2)
    set(LAYOUT_ROOT "${ARGV2}")
  else()
    set(LAYOUT_ROOT "${CMAKE_SOURCE_DIR}/res/layouts")
  endif()

  # Check if layout root exists
  if(NOT EXISTS "${LAYOUT_ROOT}")
    message(STATUS "Layout root not found: ${LAYOUT_ROOT}")
    set(${OUTPUT_VAR} "" PARENT_SCOPE)
    return()
  endif()

  # Cache key based on language and layout root
  string(MD5 CACHE_KEY "${LANGUAGE}_${LAYOUT_ROOT}")
  
  # Check if we have cached results for this language
  if(DEFINED LAYOUT_DISCOVERY_CACHE_${CACHE_KEY} AND DEFINED LAYOUT_DISCOVERY_LANG_${CACHE_KEY})
    if("${LAYOUT_DISCOVERY_LANG_${CACHE_KEY}}" STREQUAL "${LANGUAGE}")
      message(STATUS "Using cached layout discovery for language: ${LANGUAGE}")
      set(${OUTPUT_VAR} ${LAYOUT_DISCOVERY_CACHE_${CACHE_KEY}} PARENT_SCOPE)
      return()
    endif()
  endif()

  message(STATUS "Discovering layouts for language: ${LANGUAGE}...")

  # Find all layout.cpp files matching pattern: res/layouts/*/*/${LANGUAGE}/layout.cpp
  # Supports language codes like: nl, nl_nl, nl_be, en_us, etc.
  file(GLOB_RECURSE ALL_LAYOUT_FILES "${LAYOUT_ROOT}/*/*/layout.cpp")

  # Filter by language
  set(VALID_LAYOUTS "")
  foreach(LAYOUT_FILE ${ALL_LAYOUT_FILES})
    # Extract language code from path
    # Expected: .../res/layouts/{type}/{lang}/layout.cpp
    get_filename_component(LAYOUT_DIR "${LAYOUT_FILE}" DIRECTORY)
    get_filename_component(LANG_CODE "${LAYOUT_DIR}" NAME)

    # Check if language matches
    # Strategy:
    # - Exact match: LANGUAGE=nl_be matches nl_be
    # - For base languages without region: LANGUAGE=nl matches nl_nl, nl_be
    # - For languages with region: LANGUAGE=nl_be matches ONLY nl_be (exact match only)
    set(LANG_MATCH FALSE)
    if("${LANG_CODE}" STREQUAL "${LANGUAGE}")
      # Exact match
      set(LANG_MATCH TRUE)
    elseif(NOT "${LANGUAGE}" MATCHES "_" AND "${LANG_CODE}" MATCHES "^${LANGUAGE}_")
      # Only do prefix matching if LANGUAGE is a base code (no underscore)
      # e.g., LANGUAGE=nl matches nl_nl, nl_be
      # but LANGUAGE=nl_be does NOT match nl_nl
      set(LANG_MATCH TRUE)
    endif()

    if(LANG_MATCH)
      # Validate layout structure
      validate_layout_structure("${LAYOUT_DIR}" IS_VALID ERROR_MSG)
      
      if(IS_VALID)
        list(APPEND VALID_LAYOUTS "${LAYOUT_FILE}")
        message(STATUS "Found valid layout: ${LAYOUT_FILE}")
      else()
        message(WARNING "Skipping invalid layout ${LAYOUT_DIR}: ${ERROR_MSG}")
      endif()
    endif()
  endforeach()

  # Cache the results
  set(LAYOUT_DISCOVERY_CACHE_${CACHE_KEY} ${VALID_LAYOUTS} CACHE INTERNAL "Cached layout discovery results")
  set(LAYOUT_DISCOVERY_LANG_${CACHE_KEY} "${LANGUAGE}" CACHE INTERNAL "Cached language for layout discovery")
  
  message(STATUS "Layout discovery complete: found ${CMAKE_MATCH_COUNT} layouts (cached for future runs)")

  # Return list of valid layout.cpp paths
  set(${OUTPUT_VAR} ${VALID_LAYOUTS} PARENT_SCOPE)
endfunction()

message(STATUS "discover-layouts.cmake loaded")

