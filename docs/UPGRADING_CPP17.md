# Upgrade / Migration Notes — C++17

This document summarises the important changes and migration steps introduced by the project upgrade to require C++17 as a minimum language standard.

What changed
- CMake now requires a minimum of C++17 by default. The project will fail configuration if the compiler does not provide C++17 support.
- Minimum recommended compilers (CI targets): GCC >= 9 (Linux), Clang >= 10 (macOS), MSVC (Visual Studio 2019) >= 16.8 (Windows).
- CMake now supports an optional `ENABLE_CXX20` toggle to opt-in to use C++20 where desired.
- To make CI reproducible, optional runtime libraries (librstts and PortAudio) can be disabled using `BUILD_WITH_LIBRSTTS=OFF` and `BUILD_WITH_PORTAUDIO=OFF`.

How to build locally

1. Configure for standard build (C++17):

```bash
cmake -S . -B build -DBUILD_WITH_LIBRSTTS=OFF -DBUILD_WITH_PORTAUDIO=OFF
cmake --build build
```

2. To enable C++20 features during development (compiler must support C++20):

```bash
cmake -S . -B build -DENABLE_CXX20=ON -DBUILD_WITH_LIBRSTTS=OFF -DBUILD_WITH_PORTAUDIO=OFF
cmake --build build
```

Packaging and ABI policy
- ABI changes are permitted as part of this modernization. Any change that affects public ABI must be documented in release notes and migration guidance for downstream integrators.
- If you maintain binary distributions or downstream packs, please validate ABI stability in your packaging pipeline and include a migration note when releasing.

CI notes
- The project includes a GitHub Actions workflow that validates the minimum C++ support across a small matrix of OS/compiler combinations. The workflow purposely disables optional runtime libs to enable early compiler validation without heavy binary dependencies.

Contact / Questions
- If you need a stricter ABI guarantee, or if a downstream consumer requires ABI stability, open an issue or a PR to discuss guard rails (e.g. keep older ABIs, add compatibility factories, or semantic versioning guidance).
