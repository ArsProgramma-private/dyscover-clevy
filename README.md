Clevy
=====

This repository contains source code of Clevy Dyscover.

Dependencies
------------

 - C++ compiler that supports at least C++17 (recommended: GCC >= 9, Clang >= 10, MSVC/Visual Studio 2019 16.8+)
 - CMake (version 3.15 or above)
- wxWidgets (version 3.0 or above)

Building
--------

```
cmake -S . -B build
cmake --build build

Optional build toggles (CMake cache options):

- ENABLE_CXX20=ON — allow building the project with C++20 instead of the default C++17. The build will still assert a minimum of C++17 and will fail if the compiler does not support the requested standard.
- BUILD_WITH_LIBRSTTS=OFF — skip linking and packaging the librstts runtime (useful for CI images or platforms that don't include the prebuilt librstts binary). Default: ON.

CI
--
This repository now includes a GitHub Actions CI workflow which verifies C++17 compiler support on Linux, macOS and Windows runners. The CI currently performs a lightweight C++17 smoke compile test so it can validate toolchain support without requiring heavy binary dependencies.
```
