# Quickstart: Cross-Platform Support Development

## Build Matrix
| Platform | Toolchain | Dependencies to Install |
|----------|-----------|-------------------------|
| Windows (x64) | MSVC ≥19.28 | wxWidgets, PortAudio (optional), librstts (repo), CMake ≥3.15 |
| Linux (x64) | GCC ≥9 / Clang ≥10 | wxWidgets dev, libudev dev, libpulse dev, alsa-lib dev, PortAudio (optional), gettext, pkg-config |
| macOS (x86_64) | Xcode (clang ≥10) | wxWidgets (homebrew), CoreAudio (system), IOKit (system), PortAudio (brew), gettext |
| ChromeOS | Crostini container GCC/Clang | Same as Linux; may lack permissions for device/audio APIs |

## Factory Integration Steps
1. Add new abstraction headers under `src/platform/`.
2. Implement platform-specific `.cpp` files guarded by `#if defined(_WIN32)`, `#if defined(__APPLE__)`, `#if defined(__linux__)`.
3. Extend `CMakeLists.txt` to include new source files per platform block.
4. Provide mock implementations in `tests/unit` compiled always (no platform restriction).

## Testing
Run unit tests:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build --target DeviceStaticListTest
ctest --test-dir build -R unit-
```
(Add new tests similarly.)

## Adding macOS Support
- Ensure Homebrew packages: `brew install wxwidgets portaudio gettext`.
- Pass `-DLANGUAGE=nl` etc. as before.
- Future: universal binary via `CMAKE_OSX_ARCHITECTURES="x86_64;arm64"`.

## ChromeOS Notes
- If udev/libpulse inaccessible: factories set capability flags to POLLING_ONLY / volume unsupported.
- Document limitations in release notes.

## Fallback Logic
- Volume: try libpulse; if unavailable attempt ALSA; else mark unsupported.
- Keystroke interception: probe permissions; disable gracefully if denied.

## Next Actions
Implement interfaces sequentially: DeviceDetector → KeyboardHandler → AudioController → ResourceLocator refactor. Then migrate existing code paths to use abstractions.
