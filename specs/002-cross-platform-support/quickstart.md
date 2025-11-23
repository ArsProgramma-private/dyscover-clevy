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

### Permissions & Injection Notes (T053)
| Platform | Interception | Injection (sendKey) | Notes |
|----------|--------------|---------------------|-------|
| Windows  | Planned (low-level hook) | Supported (SendInput) | Alt+Shift mapping example implemented; extended layouts pending. |
| macOS    | Requires Accessibility permission (CGEventTap) | Not yet implemented (returns false) | If permission denied, handler degrades gracefully. |
| Linux    | Interception pending (evdev) | Partial (fallback only) | Layout via xkbcommon when available; AltGr symbols not yet mapped. |
| ChromeOS | Disabled (sandbox limits) | Disabled (returns false) | Handler reports Denied; translation deterministic only. |

If interception/injection is unavailable or denied, the application falls back to legacy key processing or skips the action without crashing. Update platform notes in release documentation after enabling new capabilities.

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
