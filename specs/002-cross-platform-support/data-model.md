# Data Model: Cross-Platform Support

## Entities

### DeviceDetector
Represents abstraction responsible for enumerating and monitoring supported devices.
- Fields:
  - `listener` (observer callback interface)
  - `supportedSet` (VID/PID pairs from `SupportedDevices.h`)
  - `pollIntervalMs` (for fallback polling on restricted platforms)
- Operations:
  - `bool isPresent()`
  - `void startMonitoring()` / `void stopMonitoring()`
  - `void refresh()` (manual trigger)
  - `PlatformCapabilities capabilities()` (bitflags: HOTPLUG_EVENTS, BLUETOOTH, POLLING_ONLY)

### KeyboardHandler
Handles key event interception, translation, injection, and state queries.
- Fields:
  - `layoutContext` (platform-specific handle e.g., XKB keymap, TIS input source)
  - `capsLockStateCache` (optional cached state)
  - `permissionState` (enum: GRANTED, DENIED, LIMITED)
- Operations:
  - `bool isCapsLockActive()`
  - `std::string translate(Key key, Modifiers mods)`
  - `bool sendKey(Key key, KeyEventType type)`
  - `void startInterception()` / `void stopInterception()`
  - `PermissionState getPermissionState()`

### AudioController
Abstracts volume get/set with graceful degradation.
- Fields:
  - `backendType` (enum: WindowsWaveOut, PulseAudio, ALSA, CoreAudio, Unsupported)
  - `lastKnownVolume` (int 0–65535)
  - `supported` (bool)
- Operations:
  - `int getVolume()` (returns 0–65535 or -1 on error)
  - `bool setVolume(int value)`
  - `BackendType backend()`

### ResourceLocator
Unified path resolution.
- Fields:
  - `exeDir` (base executable directory)
  - `paths` (struct: `audio`, `tts`, `translations`, `icons`)
  - `platformStyle` (enum: Windows, Linux, Mac, ChromeOS)
- Operations:
  - `std::string iconFile(const std::string& baseName)`
  - `std::string splashBitmap()`
  - `std::string audioDir()`
  - `std::string ttsDir()`
  - `std::string translationsDir()`

### PlatformUtils
Collection of shared helpers.
- Operations:
  - `bool extractVidPid(const std::string& hardwareId, std::string& vid, std::string& pid)`
  - `std::string platformName()`
  - `bool hasCapability(Capability cap)`

## Relationships
- `DeviceDetector` depends on `SupportedDevices.h` and notifies application core via listener interface.
- `KeyboardHandler` interacts with `DeviceDetector` indirectly (optional feature gating when device absent).
- `AudioController` independent; used by speech and UI feedback subsystems.
- `ResourceLocator` used by UI and audio subsystems; no platform-specific consumers beyond path + format.
- `PlatformUtils` is stateless; shared by detector & keyboard implementations.

## State & Transitions
- `DeviceDetector` monitoring state: STOPPED → (startMonitoring) → RUNNING → (stopMonitoring) → STOPPED.
- `KeyboardHandler` permission state: UNKNOWN → (probe) → GRANTED|DENIED|LIMITED; if DENIED, interception disabled.
- `AudioController` backend initialization: UNINITIALIZED → (probe) → SELECTED_BACKEND|UNSUPPORTED.

## Validation Rules
- `setVolume(value)`: value must be in [0, 65535]; reject otherwise.
- `translate(key, mods)`: returns empty string if key is non-printable or translation backend unavailable.
- `extractVidPid`: must return 4 hex digits each; else false.

## Testing Focus
- Mock implementations for each interface verifying transitions and error paths.
- Coverage emphasis: enumeration filtering, permission degradation, backend fallback selection.

