# Quickstart: Modifying Supported Devices (Compile-Time)

## Goal
Change which USB devices (Vendor ID + Product ID) the application recognizes.

## Steps
1. Open `src/SupportedDevices.h` (create if missing).
2. Add or remove entries in the `SUPPORTED_DEVICES` array:
   ```cpp
   inline constexpr SupportedDevice SUPPORTED_DEVICES[] = {
       {"04B4", "0101", "Cypress Semiconductor Device"},
       {"1234", "ABCD", "Example Device"} // New
   };
   ```
3. Rebuild:
   ```bash
   cmake --build build --target Dyscover
   ```
4. Run application and connect the device to verify detection.

## Testing Changes
- Add unit test for new VID/PID in `tests/unit/DeviceStaticListTest.cpp`.
- Run existing detection integration tests.

## Validation Checklist
- VID/PID each exactly 4 uppercase hex chars.
- No duplicate (vendorId, productId) pair.
- Description meaningful (optional).

## Removal of Runtime Config
JSON file-based configuration is deprecated and removed. End users cannot modify device support without a new build.

## Migration Note
If you previously added devices via `supported_devices.json`, port them into the static array and delete the old file if present.
