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

## Device Detection (T025-T037 Complete - US1)

Cross-platform USB device detection is now fully implemented via `IDeviceDetector` abstraction in `src/platform/DeviceDetector.h`:

### Platform Implementations
| Platform | Implementation | Hotplug Support | Notes |
|----------|---------------|-----------------|-------|
| Windows  | Config Manager API (`CM_Locate_DevNode`, `CM_Get_Child`, `CM_Get_Sibling`) | Native (`WM_DEVICECHANGE` messages via wxFrame) | Supports both USB and Bluetooth (BTHENUM) devices |
| Linux    | libudev enumeration + udev monitor | Native (udev monitor thread) | Detects USB devices via VID/PID matching |
| macOS    | IOKit (`IOServiceMatching`, `IOUSBDevice`) | Native (IONotificationPort with CFRunLoop) | Enumerates USB devices and registers hotplug callbacks |
| ChromeOS | Reuses Linux detector | Same as Linux | Polling wrapper provides fallback for sandboxed scenarios |

### Usage in Application
```cpp
// Create detector with listener (typically in Core.cpp)
struct MyListener : public IDeviceDetectorListener {
    void onDevicePresenceChanged(bool present) override {
        // Handle device connection/disconnection
    }
};

MyListener listener;
auto detector = CreateDeviceDetector(&listener);
detector->startMonitoring(); // Begins hotplug detection
// ... detector will call listener when device state changes
detector->stopMonitoring(); // Stop monitoring before cleanup
```

### Capability Flags
- `HOTPLUG_EVENTS` (bit 0): Platform supports native hotplug notifications
- `POLLING_ONLY` (bit 2): Polling wrapper active (for restricted environments)

Use `detector->capabilities()` to check supported features.

### Supported Devices
Device VID/PID list defined in `src/SupportedDevices.h`:
- Cypress (04B4:0101) - Clevy Keyboard
- Use `IsSupported(vid, pid)` helper for VID/PID validation

### Automatic Fallback
If a platform detector doesn't advertise `HOTPLUG_EVENTS`, the factory automatically wraps it with `PollingDeviceDetector` (500ms interval) to provide change notifications without native support.

### Legacy Code
`DeviceWindows::IsClevyKeyboardPresent()` and `DeviceLinux::IsClevyKeyboardPresent()` are deprecated and return false. Use `CreateDeviceDetector()` instead.

## Resource Loading (T055-T060 Complete)

All resource paths now use the `IResourceLocator` abstraction in `src/platform/ResourceLocator.cpp`:
- **Icons**: Platform-specific extensions automatically appended (.ico for Windows/Linux/ChromeOS, .icns for macOS)
- **Splash**: Bitmap resolved via `splashBitmap()` method
- **Audio**: Sound files directory via `audioDir()` 
- **TTS Data**: Speech synthesis data in nested path via `ttsDir()` returning `/tts/data`
- **Translations**: Language-specific path via `translationsDir()` returning `/lang/nl` (or nl_be)

`ResourceLoader.cpp` functions (`GetSoundFilesPath()`, `GetTTSDataPath()`, `GetTranslationsPath()`, `LoadIcon()`, `LoadSplashBitmap()`) now delegate to ResourceLocator, eliminating direct platform conditionals in callers.

## Next Actions
Complete device detection implementation (US1) or audio controller backends (US3). Keyboard handling (US2) and resource loading (US4) are functional with room for advanced features (AltGr mapping, dead keys, injection).
