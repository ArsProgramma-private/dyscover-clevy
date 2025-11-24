# Vendor ID Management (Compile-Time)

## Overview

Vendor ID (VID) and Product ID (PID) support is now defined at compile-time. The list of supported USB devices is embedded directly in the binary to prevent accidental or unauthorized runtime modification. To add or remove device support, modify the source code and rebuild.

## Location of Device List

`src/SupportedDevices.h` contains the static array:

```cpp
struct SupportedDevice {
    const char* vendorId;    // 4 uppercase hex
    const char* productId;   // 4 uppercase hex
    const char* description; // Optional
};

inline constexpr SupportedDevice SUPPORTED_DEVICES[] = {
    {"04B4", "0101", "Cypress Semiconductor Device"}
    // Add more entries here
};
```

## Modifying Supported Devices

1. Open `src/SupportedDevices.h`.
2. Add or remove entries in `SUPPORTED_DEVICES`.
3. Ensure each VID/PID is exactly 4 uppercase hex characters.
4. Rebuild the application:
   ```bash
   cmake -S . -B build
   cmake --build build --target Dyscover
   ```
5. Run the application and connect the device to verify detection.

## Validation Rules (Developer Responsibility)
- VID/PID must be 4 characters, uppercase hexadecimal (0–9, A–F).
- No duplicate (VID, PID) pairs.
- Descriptions are optional but recommended for diagnostics.
- If the list grows beyond ~200 entries, consider migrating to a hashed lookup (`unordered_set<uint32_t>` combining VID/PID).

## Runtime Behavior
- The application no longer reads or writes any `supported_devices.json` file.
- End users cannot adjust device support without a new build.
- Detection logic normalizes probed VID/PID (uppercase, pad/truncate) before comparison.

## Migration (From JSON Configuration)
If you previously added devices via `supported_devices.json`:
1. Copy each device entry into `SUPPORTED_DEVICES`.
2. Remove any lingering JSON config file from user directories—it is ignored.
3. Rebuild and verify detection.

## Logging & Diagnostics
- When a supported device is detected, existing subsystem logs continue to report presence.
- Descriptions aid in disambiguating multiple similar devices.

## Accessibility Notes
- If a UI exposes device information, ensure text has sufficient contrast and does not rely solely on color to indicate status.
- Provide clear status messaging (e.g., "Supported device connected" / "No supported device detected").

### Cross-Platform Keyboard Flow Regression Check (T077)
The cross-platform abstraction layer (Phase 002) introduces `IKeyboardHandler` and `IDeviceDetector` interfaces that replace platform-specific implementations. The following accessibility aspects have been verified as unaffected:

- **Keyboard Event Flow**: Key interception, translation, and injection continue through the same logical path in `Core::OnKeyEvent()`. The platform abstraction adds a fallback layer but does not alter event ordering or suppression logic.
- **Focus Management**: No changes to focus order or keyboard navigation paths. Device detection callbacks do not interfere with focus chains.
- **Key Labeling**: Translation logic (US layout and XKB for Linux) maintains consistent character output. No changes to spoken feedback or visual key representations.
- **Modifier Keys**: Shift, Ctrl, Alt, and AltGr handling remains consistent across platforms through the `KeyModifiers` struct abstraction.
- **Caps Lock State**: The `isCapsLockActive()` abstraction provides platform-specific Caps Lock detection without altering the application's response to Caps Lock events.
- **Permission Degradation**: When keyboard interception permissions are denied (macOS Accessibility, ChromeOS sandbox), the handler gracefully returns `PermissionState::Denied` without creating keyboard traps or breaking event flow. Users receive clear audio feedback about connection state.
- **No New UI**: The abstraction layer is backend-only. No new UI surfaces were introduced that could affect screen reader navigation or keyboard access.

**Verification Method**: 
1. Manual testing on each platform confirms keyboard events reach the application correctly
2. Unit tests verify translation correctness (`KeyboardHandlerTranslateTest`, `KeyboardHandlerPermissionTest`)
3. Device hotplug events trigger audio feedback without interrupting active keyboard interaction
4. Legacy keyboard flow remains active as fallback when platform handler is unavailable

**Conclusion**: Cross-platform support changes do not introduce accessibility regressions. Keyboard event handling, focus management, and user feedback mechanisms remain intact.

## Troubleshooting
- Device not recognized: confirm VID/PID formatting and rebuild.
- Unexpected detection: check for overlapping VID/PID values or unintended truncation/normalization.
- Performance concerns: list iteration is O(n); optimize if n becomes large.

## Advantages of Compile-Time Approach
- Immutable at runtime.
- Simpler initialization (no file IO).
- Reduced risk of malformed config.
- Clear audit trail via version control.

## Future Extension
- Introduce build script to generate `SupportedDevices.h` from a maintained registry file if list growth demands automation.

This document was updated with accessibility in mind; please still audit with tools like Accessibility Insights.