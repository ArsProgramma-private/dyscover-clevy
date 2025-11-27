# ResourceManifest.cmake
# CMake module for language-specific resource manifest generation
# Part of Feature 006: Language-Specific Resource Optimization

#[=======================================================================[.rst:
ResourceManifest
----------------

Provides CMake functions for generating language-specific resource manifests
by extracting audio file requirements from Keys.cpp based on the selected LANGUAGE.

Functions
^^^^^^^^^

.. cmake:command:: generate_resource_manifest

  ::

    generate_resource_manifest(
      <OUTPUT_MANIFEST_FILE>
      LANGUAGE <language_code>
      KEYS_SOURCE_FILE <path_to_Keys.cpp>
      RESOURCE_DIR <path_to_res/data>
    )

  Generates a manifest file listing all audio resources required for the specified language.
  
  Parameters:
    OUTPUT_MANIFEST_FILE - Path where the manifest file will be created
    LANGUAGE - Language code (e.g., nl, nl_be)
    KEYS_SOURCE_FILE - Path to Keys.cpp containing keyboard layout definitions
    RESOURCE_DIR - Path to res/data/ directory containing audio files

#]=======================================================================]

function(generate_resource_manifest OUTPUT_MANIFEST LANGUAGE KEYS_SOURCE RESOURCE_DIR)
  # Validate inputs
  if(NOT LANGUAGE)
    message(FATAL_ERROR "generate_resource_manifest: LANGUAGE parameter is required")
  endif()
  
  if(NOT EXISTS "${KEYS_SOURCE}")
    message(FATAL_ERROR "generate_resource_manifest: Keys.cpp not found at ${KEYS_SOURCE}")
  endif()
  
  if(NOT EXISTS "${RESOURCE_DIR}")
    message(FATAL_ERROR "generate_resource_manifest: Resource directory not found at ${RESOURCE_DIR}")
  endif()
  
  # Convert OUTPUT_MANIFEST to absolute path
  if(NOT IS_ABSOLUTE "${OUTPUT_MANIFEST}")
    set(OUTPUT_MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_MANIFEST}")
  endif()
  
  # Set extraction script path
  set(EXTRACTION_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/build-tools/extract-audio-resources.cmake")
  
  if(NOT EXISTS "${EXTRACTION_SCRIPT}")
    message(FATAL_ERROR "generate_resource_manifest: Extraction script not found at ${EXTRACTION_SCRIPT}")
  endif()
  
  # Execute extraction script
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -DLANGUAGE=${LANGUAGE}
      -DKEYS_SOURCE=${KEYS_SOURCE}
      -DRESOURCE_DIR=${RESOURCE_DIR}
      -DOUTPUT_MANIFEST=${OUTPUT_MANIFEST}
      -P ${EXTRACTION_SCRIPT}
    RESULT_VARIABLE EXTRACT_RESULT
    OUTPUT_VARIABLE EXTRACT_OUTPUT
    ERROR_VARIABLE EXTRACT_ERROR
  )
  
  if(NOT EXTRACT_RESULT EQUAL 0)
    message(FATAL_ERROR "Resource extraction failed:\n${EXTRACT_OUTPUT}\n${EXTRACT_ERROR}")
  endif()
  
  # Verify manifest was created
  if(NOT EXISTS "${OUTPUT_MANIFEST}")
    message(FATAL_ERROR "Resource manifest was not created at ${OUTPUT_MANIFEST}")
  endif()
  
  message(STATUS "Generated resource manifest for language '${LANGUAGE}': ${OUTPUT_MANIFEST}")
  
  # Return the manifest path to parent scope
  set(${OUTPUT_MANIFEST} "${OUTPUT_MANIFEST}" PARENT_SCOPE)
endfunction()

# Utility function to read manifest and return list of files
function(read_resource_manifest MANIFEST_FILE OUTPUT_VAR)
  if(NOT EXISTS "${MANIFEST_FILE}")
    message(FATAL_ERROR "read_resource_manifest: Manifest file not found at ${MANIFEST_FILE}")
  endif()
  
  file(STRINGS "${MANIFEST_FILE}" RESOURCE_LIST)
  set(${OUTPUT_VAR} "${RESOURCE_LIST}" PARENT_SCOPE)
endfunction()
