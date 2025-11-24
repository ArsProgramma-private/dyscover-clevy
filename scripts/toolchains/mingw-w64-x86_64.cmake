# MinGW-w64 x86_64 toolchain file for cross-compiling Dyscover on Linux
# Usage:
#   cmake -S . -B build-win64 -DCMAKE_TOOLCHAIN_FILE=scripts/toolchains/mingw-w64-x86_64.cmake \
#         -DCMAKE_BUILD_TYPE=Release -DLANGUAGE=nl -DLICENSING=demo
# Requires packages: mingw-w64, x86_64-w64-mingw32-gcc/g++.
# Assumes wxWidgets & other deps cross-built or available in SEARCH_ROOT.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compilers
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Arch specifics
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Prevent Unix default libs assumption
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optionally set WX root if cross-built (uncomment and adjust):
# set(wxWidgets_ROOT_DIR "/usr/x86_64-w64-mingw32")

# Force static runtime if desired:
# add_compile_options(-static -static-libgcc -static-libstdc++)
