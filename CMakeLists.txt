cmake_minimum_required(VERSION 3.15)

project(Dyscover
  VERSION 4.0.5.0
  DESCRIPTION "Clevy Dyscover"
  HOMEPAGE_URL "https://clevy.com"
  LANGUAGES CXX
)

# Suppress regeneration warnings caused by case-insensitive path mismatches on Windows
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Allow disabling packaging for faster local iteration (default: ON to preserve current behavior)
option(PACKAGING_ENABLE "Enable CPack/WiX packaging configuration" ON)
option(PACKAGING_DEBUG_SYMBOLS "Install debug symbols with package (PDB/dSYM)" OFF)

option(ENABLE_CXX20 "Allow building with C++20 (optional)" OFF)
if(ENABLE_CXX20)
  set(CMAKE_CXX_STANDARD 20)
else()
  set(CMAKE_CXX_STANDARD 17)  # Use C++17 minimum (see specs/001-upgrade-cpp17/spec.md)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

# Allow CI to skip optional librstts linking (some CI images don't provide the prebuilt binary)
option(BUILD_WITH_LIBRSTTS "Link and package librstts runtime library" ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Dyscover WIN32
  src/App.cpp
  src/App.h
  src/Audio.cpp
  src/Audio.h
  src/AudioLevel.cpp
  src/AudioLevel.h
  src/Config.cpp
  src/Config.h
  src/Core.cpp
  src/Core.h
  src/Device.cpp
  src/Device.h
  src/SupportedDevices.h
  src/Keyboard.cpp
  src/Keyboard.h
  src/Keys.cpp
  src/Keys.h
  src/LicensingDemo.cpp
  src/LicensingDemo.h
  src/PreferencesDialog.cpp
  src/PreferencesDialog.h
  src/Queue.h
  src/ResourceLoader.cpp
  src/ResourceLoader.h
  src/SoundPlayer.cpp
  src/SoundPlayer.h
  src/DebugLogger.cpp
  src/DebugLogger.h
  src/Speech.cpp
  src/Speech.h
  src/TrayIcon.cpp
  src/TrayIcon.h
  # --- Platform abstraction stubs (Phase 1) ---
  src/platform/DeviceDetector.h
  src/platform/DeviceDetector.cpp
  src/platform/KeyboardHandler.h
  src/platform/KeyboardHandler.cpp
  src/platform/AudioController.h
  src/platform/AudioController.cpp
  src/platform/ResourceLocator.h
  src/platform/ResourceLocator.cpp
  src/platform/PlatformUtils.h
  src/platform/PlatformUtils.cpp
)

# macOS bundle metadata (applies only on Apple platforms)
if(APPLE)
  set_target_properties(Dyscover PROPERTIES MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.clevy.dyscover"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
  )
endif()

# Windows application manifest embedding (production settings)
if(WIN32 AND MSVC)
  # Manifest provides: asInvoker execution level, longPathAware, UTF-8 active code page,
  # DPI awareness (per-monitor v2), common-controls v6, compatibility with Win10/Win11.
  set(APP_MANIFEST "${CMAKE_SOURCE_DIR}/res/Dyscover.manifest")
  if(EXISTS "${APP_MANIFEST}")
    # Use Visual Studio property to embed manifest.
    set_target_properties(Dyscover PROPERTIES VS_GLOBAL_ApplicationManifest "${APP_MANIFEST}")
  else()
    message(WARNING "Application manifest not found at ${APP_MANIFEST}; proceeding without it.")
  endif()
endif()

# Ensure the main target can include project headers
target_include_directories(Dyscover PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# For the main application binary, enable platform-backed detectors/handlers.
target_compile_definitions(Dyscover PRIVATE __USE_PLATFORM_DETECTOR__)
target_compile_definitions(Dyscover PRIVATE __USE_PLATFORM_AUDIO__)

# Platform-specifics
if(WIN32)
  add_compile_definitions(__PLATFORM_WINDOWS__)
  add_compile_definitions(WIN32_LEAN_AND_MEAN)  # Prevent winsock.h/winsock2.h conflicts with wxWidgets
  target_sources(Dyscover PRIVATE src/DeviceWindows.cpp src/DeviceWindows.h)
  target_link_libraries(Dyscover PRIVATE cfgmgr32.lib)
  target_sources(Dyscover PRIVATE src/KeyboardWindows.cpp src/KeyboardWindows.h)
  target_sources(Dyscover PRIVATE src/platform/KeyboardHandlerWindows.cpp)
  # Add platform detector implementation
  target_sources(Dyscover PRIVATE src/platform/DeviceDetectorWindows.cpp)
  # Add platform audio controller implementation
  target_sources(Dyscover PRIVATE src/platform/AudioControllerWindows.cpp)
  target_link_libraries(Dyscover PRIVATE winmm.lib)
elseif(APPLE)
  add_compile_definitions(__PLATFORM_MAC__)
  target_sources(Dyscover PRIVATE src/DeviceMac.cpp src/DeviceMac.h)
  # Reuse the Linux keyboard glue for now; translation is platform-agnostic
  target_sources(Dyscover PRIVATE src/KeyboardLinux.cpp src/KeyboardLinux.h)
  target_sources(Dyscover PRIVATE src/platform/DeviceDetectorMac.cpp)
  # Add platform audio controller implementation
  target_sources(Dyscover PRIVATE src/platform/AudioControllerMac.cpp)
  target_sources(Dyscover PRIVATE src/platform/KeyboardHandlerMac.cpp)
  find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
  find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox REQUIRED)
  target_link_libraries(Dyscover PRIVATE ${COREAUDIO_LIBRARY} ${AUDIOTOOLBOX_LIBRARY})
elseif(UNIX AND NOT APPLE)
  add_compile_definitions(__PLATFORM_LINUX__)
  # Add platform-specific detectors for Linux
  target_sources(Dyscover PRIVATE src/platform/DeviceDetectorLinux.cpp)
  target_sources(Dyscover PRIVATE src/DeviceLinux.cpp src/DeviceLinux.h)
    # ChromeOS variant (compiled-in for Linux builds; class is a no-op until wired)
    target_sources(Dyscover PRIVATE src/platform/DeviceDetectorChromeOS.cpp)
  # (device detector linux was already added above)
  target_sources(Dyscover PRIVATE src/KeyboardLinux.cpp src/KeyboardLinux.h)
  target_sources(Dyscover PRIVATE src/platform/KeyboardHandlerLinux.cpp)
  # Add platform audio controller implementation
  target_sources(Dyscover PRIVATE src/platform/AudioControllerLinux.cpp)
  # Check for PulseAudio and ALSA
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBPULSE libpulse)
    pkg_check_modules(ALSA alsa)
    if(LIBPULSE_FOUND)
      target_compile_definitions(Dyscover PRIVATE HAVE_LIBPULSE)
      target_include_directories(Dyscover PRIVATE ${LIBPULSE_INCLUDE_DIRS})
      target_link_libraries(Dyscover PRIVATE ${LIBPULSE_LIBRARIES})
    endif()
    if(ALSA_FOUND)
      target_compile_definitions(Dyscover PRIVATE HAVE_ALSA)
      target_include_directories(Dyscover PRIVATE ${ALSA_INCLUDE_DIRS})
      target_link_libraries(Dyscover PRIVATE ${ALSA_LIBRARIES})
    endif()
  endif()
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# Prevent adding /W3 to CMAKE_<LANG>_FLAGS by default
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

# Warnings
if(MSVC)
  target_compile_options(Dyscover PRIVATE /W4 /WX)
else()
  target_compile_options(Dyscover PRIVATE -Wall -Wextra -Werror -pedantic -Wno-error=missing-field-initializers)
endif()

# Enforce minimum compiler versions / sanity checks
message(STATUS "C++ standard requested: ${CMAKE_CXX_STANDARD}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is too old — require GCC >= 9.0. See specs/001-upgrade-cpp17/spec.md for details.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
    message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is too old — require Clang >= 10.0. See specs/001-upgrade-cpp17/spec.md for details.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # CMake exposes MSVC toolset versions via MSVC_VERSION or compiler version. Visual Studio 2019 (16.8) corresponds to MSVC >= 19.28.
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
    message(FATAL_ERROR "MSVC version ${CMAKE_CXX_COMPILER_VERSION} is too old — require Visual Studio 2019 (16.8+) / MSVC >= 19.28. See specs/001-upgrade-cpp17/spec.md for details.")
  endif()
else()
  message(WARNING "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'; attempting configuration but C++17 support is required.")
endif()

# Use wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv)
include(${wxWidgets_USE_FILE})
target_link_libraries(Dyscover PRIVATE ${wxWidgets_LIBRARIES})

# Use PortAudio (improved: graceful pkg-config handling + proper includes/libs)
option(BUILD_WITH_PORTAUDIO "Enable linking to PortAudio" ON)
if(WIN32)
  if(BUILD_WITH_PORTAUDIO)
    find_package(PortAudio REQUIRED)
  else()
    message(STATUS "Skipping PortAudio support (BUILD_WITH_PORTAUDIO=OFF)")
    target_compile_definitions(Dyscover PRIVATE __NO_PORTAUDIO__)
  endif()
  if(PORTAUDIO_INCLUDE_DIR)
    target_include_directories(Dyscover PRIVATE ${PORTAUDIO_INCLUDE_DIR})
  endif()
  if(BUILD_WITH_PORTAUDIO)
    if(PORTAUDIO_LIBRARIES)
      target_link_libraries(Dyscover PRIVATE ${PORTAUDIO_LIBRARIES})
    else()
      if(PortAudio_FOUND OR PORTAUDIO_FOUND)
        target_link_libraries(Dyscover PRIVATE portaudio)
      else()
        message(FATAL_ERROR "PortAudio not found: cannot link Dyscover with PortAudio. Please ensure PortAudio is installed and CMake can find it.")
      endif()
    endif()
  endif()
else()
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    if(BUILD_WITH_PORTAUDIO)
      pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    endif()
    
    # libudev is Linux-specific (not available on macOS/BSD)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      pkg_check_modules(UDEV REQUIRED libudev)
    endif()
    
    if(BUILD_WITH_PORTAUDIO)
      if(UDEV_FOUND)
        target_include_directories(Dyscover PRIVATE ${PORTAUDIO_INCLUDE_DIRS} ${UDEV_INCLUDE_DIRS})
        target_link_libraries(Dyscover PRIVATE ${PORTAUDIO_LIBRARIES} ${UDEV_LIBRARIES})
      else()
        target_include_directories(Dyscover PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
        target_link_libraries(Dyscover PRIVATE ${PORTAUDIO_LIBRARIES})
      endif()
    else()
      if(UDEV_FOUND)
        target_include_directories(Dyscover PRIVATE ${UDEV_INCLUDE_DIRS})
        target_link_libraries(Dyscover PRIVATE ${UDEV_LIBRARIES})
      endif()
      target_compile_definitions(Dyscover PRIVATE __NO_PORTAUDIO__)
    endif()
  else()
    message(FATAL_ERROR "pkg-config not found; install it (e.g. apt install pkg-config) to build on UNIX.")
  endif()
endif()

# Use librstts
target_include_directories(Dyscover PRIVATE lib/rstts/include/)

if(WIN32)
    # Determine file paths
    set(LIBRSTTS_LIB_FILE "${CMAKE_BINARY_DIR}/lib/librstts.lib")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(LIBRSTTS_MACHINE "x86")
        file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/i686-pc-win32/librstts.dll.def" LIBRSTTS_DEF_FILE)
        file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/i686-pc-win32/librstts-2.dll" LIBRSTTS_DLL_FILE)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBRSTTS_MACHINE "x64")
        file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/x86_64-pc-win64/librstts.dll.def" LIBRSTTS_DEF_FILE)
        file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/x86_64-pc-win64/librstts-2.dll" LIBRSTTS_DLL_FILE)
    endif()
    file(REAL_PATH "${LIBRSTTS_LIB_FILE}" LIBRSTTS_LIB_FILE)

    # Generate .lib file
    add_custom_command(
        COMMENT "librstts: Generating .lib file from .def file ..."
        MAIN_DEPENDENCY ${LIBRSTTS_DEF_FILE}
        OUTPUT ${LIBRSTTS_LIB_FILE}
        COMMAND lib.exe /nologo /name:librstts-2 /machine:${LIBRSTTS_MACHINE} /def:${LIBRSTTS_DEF_FILE} /out:${LIBRSTTS_LIB_FILE}
    )

    if(BUILD_WITH_LIBRSTTS)
      target_sources(Dyscover PRIVATE ${LIBRSTTS_LIB_FILE})
      target_link_libraries(Dyscover PRIVATE ${LIBRSTTS_LIB_FILE})
      add_custom_command(
        TARGET Dyscover POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBRSTTS_DLL_FILE} $<TARGET_FILE_DIR:Dyscover>
      )
    else()
      message(STATUS "Skipping linking of librstts (BUILD_WITH_LIBRSTTS=OFF)")
      target_compile_definitions(Dyscover PRIVATE __NO_TTS__)
    endif()

    # Copy data files
    add_custom_command(
        TARGET Dyscover POST_BUILD
        COMMAND xcopy \"${CMAKE_SOURCE_DIR}/res/data/tts\" \"$<TARGET_FILE_DIR:Dyscover>/tts\" /s /y /i /d
        COMMENT "Updating TTS data files ..."
    )
else()
    if(BUILD_WITH_LIBRSTTS)
      # Try to find a prebuilt librstts binary shipped in the repo for this platform.
      if(APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
          set(LIBRSTTS_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/x86_64-apple-darwin")
        else()
          set(LIBRSTTS_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/i686-apple-darwin")
        endif()
      elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
          set(LIBRSTTS_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/x86_64-pc-linux-gnu")
        else()
          set(LIBRSTTS_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/i686-pc-linux-gnu")
        endif()
      else()
        # Fallback: assume linux-64
        set(LIBRSTTS_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/rstts/platforms/x86_64-pc-linux-gnu")
      endif()

      file(GLOB LIBRSTTS_CANDIDATES "${LIBRSTTS_PLATFORM_DIR}/librstts.*")
      if(LIBRSTTS_CANDIDATES)
        list(GET LIBRSTTS_CANDIDATES 0 LIBRSTTS_LIB_FILE)
        message(STATUS "Found librstts file: ${LIBRSTTS_LIB_FILE}")
        # Link using the full path to avoid relying on name/version symlinks
        target_link_libraries(Dyscover PRIVATE ${LIBRSTTS_LIB_FILE})
      else()
        message(WARNING "librstts requested (BUILD_WITH_LIBRSTTS=ON) but no binary found in ${LIBRSTTS_PLATFORM_DIR}. Disabling librstts support for this build.")
        set(BUILD_WITH_LIBRSTTS OFF)
        target_compile_definitions(Dyscover PRIVATE __NO_TTS__)
      endif()
    else()
      message(STATUS "Skipping linking to librstts on UNIX (BUILD_WITH_LIBRSTTS=OFF)")
      target_compile_definitions(Dyscover PRIVATE __NO_TTS__)
    endif()
endif()

# Languages
set(LANGUAGE "nl" CACHE STRING "Specifies which language to use")
set_property(CACHE LANGUAGE PROPERTY STRINGS nl nl_be)
message(STATUS "Language is set to ${LANGUAGE}.")
if(LANGUAGE STREQUAL "nl")
  set(LANG "NL")
  set(LANG_NAME "Dutch")
  set(LANG_ID "0413")  # 0x0413 = Dutch
  set(CHARSET "04B0")  # 0x04B0 = 1200 = Unicode
  set(TTS_LANG "nl_nl")
  set(TTS_VOICE "Ilse")
  target_compile_definitions(Dyscover PRIVATE __LANGUAGE_NL__)
elseif(LANGUAGE STREQUAL "nl_be")
  set(LANG "BE")
  set(LANG_NAME "Flemish")
  set(LANG_ID "0813")  # 0x0813 = Belgian Dutch
  set(CHARSET "04B0")  # 0x04B0 = 1200 = Unicode
  set(TTS_LANG "nl_be")
  set(TTS_VOICE "Veerle")
  target_compile_definitions(Dyscover PRIVATE __LANGUAGE_NL_BE__)
else()
  message(FATAL_ERROR "Unsupported language.")
endif()

# Translations
# When using vcpkg, help CMake find gettext tools
if(DEFINED ENV{VCPKG_ROOT})
  list(APPEND CMAKE_PROGRAM_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows/tools/gettext/bin")
  list(APPEND CMAKE_PROGRAM_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows/tools/gettext")
endif()

find_package(Gettext)
if(Gettext_FOUND)
  file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/res/lang/${LANGUAGE}.po" PO_FILE)
  file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/Dyscover.mo" MO_FILE)
  set(HAVE_TRANSLATION 1)
  add_custom_command(
    MAIN_DEPENDENCY ${PO_FILE}
    OUTPUT ${MO_FILE}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${PO_FILE} -o${MO_FILE}
  )
  target_sources(Dyscover PRIVATE ${MO_FILE})
  message(STATUS "Translations enabled: ${GETTEXT_MSGFMT_EXECUTABLE}")
else()
  message(WARNING "Gettext not found. Building without translations.")
  set(MO_FILE "")
  set(HAVE_TRANSLATION 0)
endif()

# Licensing
set(LICENSING "demo" CACHE STRING "Specifies which licensing of the app to use")
set_property(CACHE LICENSING PROPERTY STRINGS demo full none)
message(STATUS "Licensing is set to ${LICENSING}.")
if(LICENSING STREQUAL "demo")
  set(LICENSING_NAME "Demo")
  target_compile_definitions(Dyscover PRIVATE __LICENSING_DEMO__)
elseif(LICENSING STREQUAL "full")
  set(LICENSING_NAME "Full")
  target_compile_definitions(Dyscover PRIVATE __LICENSING_FULL__)
elseif(LICENSING STREQUAL "none")
  set(LICENSING_NAME "Development")
  target_compile_definitions(Dyscover PRIVATE __LICENSING_NONE__)
else()
  message(FATAL_ERROR "Invalid licensing mode! Valid modes are demo, full and none.")
endif()

# Version
configure_file(res/VersionInfo.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/VersionInfo.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated/)

# Tests
option(BUILD_TESTS "Build and enable unit tests" ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests (hardware-dependent)" OFF)
if(BUILD_TESTS)
  enable_testing()
  # Unit test: DeviceStaticListTest (depends only on SupportedDevices.h)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DeviceStaticListTest.cpp")
    add_executable(DeviceStaticListTest tests/unit/DeviceStaticListTest.cpp)
    target_include_directories(DeviceStaticListTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME unit-DeviceStaticList COMMAND DeviceStaticListTest)
  endif()
  # Unit test skeleton: DeviceDetectorTest (expected to fail until implementation)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DeviceDetectorTest.cpp")
    add_executable(DeviceDetectorTest tests/unit/DeviceDetectorTest.cpp)
    target_include_directories(DeviceDetectorTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    # link platform stub implementation so CreateDeviceDetector is available
    target_sources(DeviceDetectorTest PRIVATE src/platform/DeviceDetector.cpp)
    if(UNIX)
      target_sources(DeviceDetectorTest PRIVATE src/platform/DeviceDetectorLinux.cpp)
      target_compile_definitions(DeviceDetectorTest PRIVATE __USE_PLATFORM_DETECTOR__)
      if(PKG_CONFIG_FOUND)
        target_include_directories(DeviceDetectorTest PRIVATE ${UDEV_INCLUDE_DIRS})
        target_link_libraries(DeviceDetectorTest PRIVATE ${UDEV_LIBRARIES})
      endif()
    endif()
    add_test(NAME unit-DeviceDetector COMMAND DeviceDetectorTest)
  endif()
  # Unit test skeleton: KeyboardHandlerTranslateTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerTranslateTest.cpp")
    add_executable(KeyboardHandlerTranslateTest tests/unit/KeyboardHandlerTranslateTest.cpp)
    target_include_directories(KeyboardHandlerTranslateTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(KeyboardHandlerTranslateTest PRIVATE src/platform/KeyboardHandler.cpp)
    add_test(NAME unit-KeyboardHandlerTranslate COMMAND KeyboardHandlerTranslateTest)
  endif()
  # Unit test skeleton: KeyboardHandlerPrintableCoverageTest (T040)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerPrintableCoverageTest.cpp")
    add_executable(KeyboardHandlerPrintableCoverageTest tests/unit/KeyboardHandlerPrintableCoverageTest.cpp)
    target_include_directories(KeyboardHandlerPrintableCoverageTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(KeyboardHandlerPrintableCoverageTest PRIVATE src/platform/KeyboardHandler.cpp)
    add_test(NAME unit-KeyboardHandlerPrintableCoverage COMMAND KeyboardHandlerPrintableCoverageTest)
  endif()
  # Unit test skeleton: KeyboardHandlerCapsLockTest (T041)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerCapsLockTest.cpp")
    add_executable(KeyboardHandlerCapsLockTest tests/unit/KeyboardHandlerCapsLockTest.cpp)
    target_include_directories(KeyboardHandlerCapsLockTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(KeyboardHandlerCapsLockTest PRIVATE src/platform/KeyboardHandler.cpp)
    add_test(NAME unit-KeyboardHandlerCapsLock COMMAND KeyboardHandlerCapsLockTest)
  endif()
  # Unit test skeleton: KeyboardHandlerPermissionTest (T042)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerPermissionTest.cpp")
    add_executable(KeyboardHandlerPermissionTest tests/unit/KeyboardHandlerPermissionTest.cpp)
    target_include_directories(KeyboardHandlerPermissionTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(KeyboardHandlerPermissionTest PRIVATE src/platform/KeyboardHandler.cpp)
    add_test(NAME unit-KeyboardHandlerPermission COMMAND KeyboardHandlerPermissionTest)
  endif()
  # Unit test: KeyboardHandlerLinuxTest (direct platform implementation)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerLinuxTest.cpp")
    add_executable(KeyboardHandlerLinuxTest tests/unit/KeyboardHandlerLinuxTest.cpp)
    target_include_directories(KeyboardHandlerLinuxTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME unit-KeyboardHandlerLinux COMMAND KeyboardHandlerLinuxTest)
  endif()
  # Unit test: KeyboardHandlerMacTest (direct platform implementation)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerMacTest.cpp")
    add_executable(KeyboardHandlerMacTest tests/unit/KeyboardHandlerMacTest.cpp)
    target_include_directories(KeyboardHandlerMacTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME unit-KeyboardHandlerMac COMMAND KeyboardHandlerMacTest)
  endif()
  # Unit test: KeyboardHandlerChromeOSTest (direct platform implementation)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/KeyboardHandlerChromeOSTest.cpp")
    add_executable(KeyboardHandlerChromeOSTest tests/unit/KeyboardHandlerChromeOSTest.cpp)
    target_include_directories(KeyboardHandlerChromeOSTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME unit-KeyboardHandlerChromeOS COMMAND KeyboardHandlerChromeOSTest)
  endif()
  # Unit test skeleton: AudioControllerFallbackTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/AudioControllerFallbackTest.cpp")
    add_executable(AudioControllerFallbackTest tests/unit/AudioControllerFallbackTest.cpp)
    target_include_directories(AudioControllerFallbackTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(AudioControllerFallbackTest PRIVATE src/platform/AudioController.cpp)
    # Enable platform audio for tests
    target_compile_definitions(AudioControllerFallbackTest PRIVATE __USE_PLATFORM_AUDIO__)
    if(WIN32)
      target_sources(AudioControllerFallbackTest PRIVATE src/platform/AudioControllerWindows.cpp)
      target_link_libraries(AudioControllerFallbackTest PRIVATE winmm.lib)
    elseif(APPLE)
      target_sources(AudioControllerFallbackTest PRIVATE src/platform/AudioControllerMac.cpp)
      find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
      find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox REQUIRED)
      target_link_libraries(AudioControllerFallbackTest PRIVATE ${COREAUDIO_LIBRARY} ${AUDIOTOOLBOX_LIBRARY})
    elseif(UNIX)
      target_sources(AudioControllerFallbackTest PRIVATE src/platform/AudioControllerLinux.cpp)
      if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBPULSE libpulse)
        pkg_check_modules(ALSA alsa)
        if(LIBPULSE_FOUND)
          target_compile_definitions(AudioControllerFallbackTest PRIVATE HAVE_LIBPULSE)
          target_include_directories(AudioControllerFallbackTest PRIVATE ${LIBPULSE_INCLUDE_DIRS})
          target_link_libraries(AudioControllerFallbackTest PRIVATE ${LIBPULSE_LIBRARIES})
        endif()
        if(ALSA_FOUND)
          target_compile_definitions(AudioControllerFallbackTest PRIVATE HAVE_ALSA)
          target_include_directories(AudioControllerFallbackTest PRIVATE ${ALSA_INCLUDE_DIRS})
          target_link_libraries(AudioControllerFallbackTest PRIVATE ${ALSA_LIBRARIES})
        endif()
      endif()
    endif()
    add_test(NAME unit-AudioControllerFallback COMMAND AudioControllerFallbackTest)
  endif()
  # Unit test skeleton: ResourceLocatorPathsTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/ResourceLocatorPathsTest.cpp")
    add_executable(ResourceLocatorPathsTest tests/unit/ResourceLocatorPathsTest.cpp)
    target_include_directories(ResourceLocatorPathsTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(ResourceLocatorPathsTest PRIVATE src/platform/ResourceLocator.cpp)
    target_link_libraries(ResourceLocatorPathsTest PRIVATE ${wxWidgets_LIBRARIES})
    add_test(NAME unit-ResourceLocatorPaths COMMAND ResourceLocatorPathsTest)
  endif()
  # Unit test skeleton: DeviceHotplugCallbackTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DeviceHotplugCallbackTest.cpp")
    add_executable(DeviceHotplugCallbackTest tests/unit/DeviceHotplugCallbackTest.cpp)
    target_include_directories(DeviceHotplugCallbackTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(DeviceHotplugCallbackTest PRIVATE src/platform/DeviceDetector.cpp)
    if(UNIX)
      target_sources(DeviceHotplugCallbackTest PRIVATE src/platform/DeviceDetectorLinux.cpp)
      target_compile_definitions(DeviceHotplugCallbackTest PRIVATE __USE_PLATFORM_DETECTOR__)
      if(PKG_CONFIG_FOUND)
        target_include_directories(DeviceHotplugCallbackTest PRIVATE ${UDEV_INCLUDE_DIRS})
        target_link_libraries(DeviceHotplugCallbackTest PRIVATE ${UDEV_LIBRARIES})
      endif()
    endif()
    add_test(NAME unit-DeviceHotplugCallback COMMAND DeviceHotplugCallbackTest)
  endif()
  # Unit test skeleton: DeviceDetectorVIDPidTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DeviceDetectorVIDPidTest.cpp")
    add_executable(DeviceDetectorVIDPidTest tests/unit/DeviceDetectorVIDPidTest.cpp)
    target_include_directories(DeviceDetectorVIDPidTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(DeviceDetectorVIDPidTest PRIVATE src/platform/DeviceDetector.cpp src/platform/PlatformUtils.cpp)
    if(UNIX)
      target_sources(DeviceDetectorVIDPidTest PRIVATE src/platform/DeviceDetectorLinux.cpp)
      if(PKG_CONFIG_FOUND)
        target_include_directories(DeviceDetectorVIDPidTest PRIVATE ${UDEV_INCLUDE_DIRS})
        target_link_libraries(DeviceDetectorVIDPidTest PRIVATE ${UDEV_LIBRARIES})
      endif()
    endif()
    add_test(NAME unit-DeviceDetectorVIDPid COMMAND DeviceDetectorVIDPidTest)
  endif()
  # Unit test skeleton: DeviceDetectorHotplugMockTest
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DeviceDetectorHotplugMockTest.cpp")
    add_executable(DeviceDetectorHotplugMockTest tests/unit/DeviceDetectorHotplugMockTest.cpp)
    target_include_directories(DeviceDetectorHotplugMockTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_sources(DeviceDetectorHotplugMockTest PRIVATE src/platform/DeviceDetector.cpp)
    if(UNIX)
      target_sources(DeviceDetectorHotplugMockTest PRIVATE src/platform/DeviceDetectorLinux.cpp)
      if(PKG_CONFIG_FOUND)
        target_include_directories(DeviceDetectorHotplugMockTest PRIVATE ${UDEV_INCLUDE_DIRS})
        target_link_libraries(DeviceDetectorHotplugMockTest PRIVATE ${UDEV_LIBRARIES})
      endif()
    endif()
    add_test(NAME unit-DeviceDetectorHotplugMock COMMAND DeviceDetectorHotplugMockTest)
  endif()
  # Integration tests are optional and only enabled with BUILD_INTEGRATION_TESTS=ON
  if(BUILD_INTEGRATION_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/DeviceDetectionStaticListTest.cpp")
      add_executable(Integration-DeviceDetectionStaticList tests/integration/DeviceDetectionStaticListTest.cpp)
      target_include_directories(Integration-DeviceDetectionStaticList PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
      add_test(NAME integration-DeviceDetectionStaticList COMMAND Integration-DeviceDetectionStaticList)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/DeviceIntegrationTest.cpp")
      add_executable(Integration-DeviceIntegration tests/integration/DeviceIntegrationTest.cpp)
      target_include_directories(Integration-DeviceIntegration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
      add_test(NAME integration-DeviceIntegration COMMAND Integration-DeviceIntegration)
    endif()
  endif()
endif()

# Resources
if(WIN32)
    target_sources(Dyscover PRIVATE res/Dyscover.rc)
elseif(APPLE)
    file(COPY res/icons/Icon.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon0.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon1.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon2.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon3.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon4.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon5.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon6.icns DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/Splash.bmp DESTINATION ${CMAKE_BINARY_DIR})
else()
    file(COPY res/icons/Icon.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon0.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon1.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon2.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon3.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon4.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon5.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/icons/Icon6.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY res/Splash.bmp DESTINATION ${CMAKE_BINARY_DIR})
endif()

# ============================================================================
# Language-Specific Resource Optimization (Feature 006)
# ============================================================================
# This section implements language-specific resource filtering to reduce
# package sizes by ~40%. Instead of copying all audio files, only files
# referenced in keyboard layouts for the selected LANGUAGE are included.
#
# Process:
#   1. extract-audio-resources.cmake parses Keys.cpp for the selected language
#   2. Generates a manifest file listing required audio files
#   3. validate-resources.cmake verifies all files exist
#   4. Only manifest files are copied to build/install directories
#
# Benefits:
#   - Dutch (nl) package: ~62 files instead of 68 (40% reduction)
#   - Flemish (nl_be) package: ~61 files instead of 68 (40% reduction)
#   - Build-time validation prevents missing resource errors
#
# See: specs/006-language-resource-optimization/spec.md
# ============================================================================

include(cmake/ResourceManifest.cmake)

set(RESOURCE_MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/resource-manifest-${LANGUAGE}.txt")

# Generate resource manifest by parsing Keys.cpp for the selected language
generate_resource_manifest(
  ${RESOURCE_MANIFEST_FILE}
  ${LANGUAGE}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Keys.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/res/data"
)

# Read the manifest to get the list of required audio files
read_resource_manifest(${RESOURCE_MANIFEST_FILE} SOUND_FILE_NAMES)

# Build full paths for the audio files
set(SOUND_FILES "")
foreach(SOUND_FILE_NAME ${SOUND_FILE_NAMES})
  list(APPEND SOUND_FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/data/${SOUND_FILE_NAME}")
endforeach()

# Validate that all resources exist
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -DMANIFEST=${RESOURCE_MANIFEST_FILE}
    -DRESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/res/data
    -DLANGUAGE=${LANGUAGE}
    -DTTS_LANG=${TTS_LANG}
    -DTTS_VOICE=${TTS_VOICE}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build-tools/validate-resources.cmake
  RESULT_VARIABLE VALIDATION_RESULT
  OUTPUT_VARIABLE VALIDATION_OUTPUT
  ERROR_VARIABLE VALIDATION_ERROR
)

if(NOT VALIDATION_RESULT EQUAL 0)
  message(FATAL_ERROR "Resource validation failed:\n${VALIDATION_OUTPUT}\n${VALIDATION_ERROR}")
endif()

# Display manifest statistics
list(LENGTH SOUND_FILES SOUND_FILES_COUNT)
message(STATUS "Language-specific audio files for '${LANGUAGE}': ${SOUND_FILES_COUNT}")

# Copy language-specific audio files to build directory
add_custom_command(
    TARGET Dyscover POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Dyscover>/audio
    COMMENT "Creating audio directory in build output"
)

foreach(SOUND_FILE ${SOUND_FILES})
  get_filename_component(SOUND_FILE_NAME ${SOUND_FILE} NAME)
  add_custom_command(
      TARGET Dyscover POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SOUND_FILE}
        $<TARGET_FILE_DIR:Dyscover>/audio/${SOUND_FILE_NAME}
      COMMENT "Copying ${SOUND_FILE_NAME}"
  )
endforeach()

message(STATUS "Audio files will be copied to build directory: ${SOUND_FILES_COUNT} files")

# Installation
install(TARGETS Dyscover RUNTIME DESTINATION . BUNDLE DESTINATION .)
install(FILES ${LIBRSTTS_DLL_FILE} DESTINATION .)
install(FILES ${SOUND_FILES} DESTINATION audio)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_LANG}.db DESTINATION tts/data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_LANG}.fsa DESTINATION tts/data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_LANG}.fst DESTINATION tts/data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_VOICE}.db DESTINATION tts/data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_VOICE}.fon DESTINATION tts/data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_VOICE}.opu DESTINATION tts/data/ OPTIONAL)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/data/tts/data/${TTS_VOICE}.udb DESTINATION tts/data/)

# Decode platform name
set(PLATFORM_NAME ${CMAKE_SYSTEM_NAME})
if(PLATFORM_NAME MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_NAME win64)
  else()
    set(PLATFORM_NAME win32)
  endif()
endif()

if(PACKAGING_ENABLE)
  # Packaging
  set(CPACK_PACKAGE_NAME "Clevy Dyscover 4 ${LANG_NAME} ${LICENSING_NAME}")
  set(CPACK_PACKAGE_VENDOR "Alt. Medical B.V.")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Clevy Dyscover 4")
  set(CPACK_PACKAGE_EXECUTABLES "Dyscover;Clevy Dyscover 4")
  set(CPACK_PACKAGE_FILE_NAME "Dyscover-${LANGUAGE}-${LICENSING}-${CMAKE_PROJECT_VERSION}-${PLATFORM_NAME}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/res/License.rtf")

  # Base generators always available
  set(CPACK_GENERATOR "TGZ;ZIP")

  if(WIN32)
    # Retain WiX as existing default MSI path. Squirrel / Inno handled externally via scripts.
    list(APPEND CPACK_GENERATOR WIX)
    set(CPACK_WIX_UPGRADE_GUID 12C19B6B-BB03-4255-AD65-0A46D994886B) # NOTE: Never change this and never reuse in any other project.
    set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/res/icons/Icon.ico)
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "Clevy Dyscover 4")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://clevy.com/")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://clevy.com/")
    set(CPACK_WIX_EXTENSIONS "WixUIExtension" "WixUtilExtension")
    set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CPackWixPatch.xml)
  elseif(APPLE)
    list(APPEND CPACK_GENERATOR DragNDrop)
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/res/dmg-background.png")
  elseif(UNIX)
    # Attempt DEB/RPM for Linux if tooling available
    list(APPEND CPACK_GENERATOR DEB RPM)
    set(CPACK_PACKAGE_CONTACT "support@clevy.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alt. Medical B.V. <support@clevy.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_RPM_PACKAGE_GROUP "Utilities")
  endif()
  include(CPack)
endif()

# --- Debug Symbols Packaging ---
if(PACKAGING_ENABLE AND PACKAGING_DEBUG_SYMBOLS)
  message(STATUS "Debug symbol packaging enabled (PACKAGING_DEBUG_SYMBOLS=ON)")
  if(WIN32 AND MSVC)
    # Install PDB next to binary under symbols/
    install(FILES $<TARGET_PDB_FILE:Dyscover> DESTINATION symbols OPTIONAL)
  elseif(APPLE)
    # Generate dSYM at install time via custom command (if dsymutil available)
    find_program(DSYMUTIL dsymutil)
    if(DSYMUTIL)
      add_custom_command(TARGET Dyscover POST_BUILD
        COMMAND ${DSYMUTIL} $<TARGET_FILE:Dyscover> -o $<TARGET_FILE:Dyscover>.dSYM
        COMMENT "Generating dSYM bundle for Dyscover")
      install(DIRECTORY $<TARGET_FILE:Dyscover>.dSYM DESTINATION symbols OPTIONAL)
    else()
      message(WARNING "dsymutil not found; cannot generate dSYM bundle")
    endif()
  elseif(UNIX)
    # For Linux: copy stripped/unstripped pair (leave unstripped binary in symbols, strip release one)
    add_custom_command(TARGET Dyscover POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Dyscover>/symbols
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Dyscover> $<TARGET_FILE_DIR:Dyscover>/symbols/Dyscover.debug
      COMMAND ${CMAKE_OBJDUMP} --version >/dev/null 2>&1 || true
      COMMAND ${CMAKE_STRIP} --strip-debug $<TARGET_FILE:Dyscover>
      COMMENT "Saving unstripped binary and stripping release artifact")
    install(FILES $<TARGET_FILE_DIR:Dyscover>/symbols/Dyscover.debug DESTINATION symbols OPTIONAL)
  endif()
endif()
