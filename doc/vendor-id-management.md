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