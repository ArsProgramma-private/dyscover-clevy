# Quickstart: Device Vendor Management

**Feature**: specs/001-device-vendor-management/spec.md

## Overview

This feature allows developers to manage the list of supported USB devices by editing a configuration file. Changes take effect immediately without restarting the application.

## Configuration File

**Location**: `~/.config/dyscover-clevy/supported_devices.json` (Linux/macOS) or `%APPDATA%\Dyscover Clevy\supported_devices.json` (Windows)

**Default Content**:
```json
{
  "version": "1.0",
  "devices": [
    {
      "vendorId": "04B4",
      "productId": "0101"
    },
    {
      "vendorId": "0CD3",
      "productId": "320F"
    }
  ]
}
```

## Adding a New Device

1. Open the configuration file in a text editor
2. Add a new object to the `devices` array:
   ```json
   {
     "vendorId": "1234",
     "productId": "5678",
     "description": "My New Device"
   }
   ```
3. Save the file
4. The application will detect the new device immediately

## Removing a Device

1. Open the configuration file
2. Remove the corresponding object from the `devices` array
3. Save the file
4. The device will no longer be recognized

## Validation

- Vendor ID and Product ID must be exactly 4 hexadecimal characters (0-9, A-F)
- Duplicate VID/PID combinations are not allowed
- Invalid entries are skipped with warnings logged

## Troubleshooting

- **File not found**: The application creates a default file on first run
- **Invalid JSON**: Check syntax with a JSON validator
- **Device not detected**: Verify VID/PID format and uniqueness
- **Changes not applied**: Ensure file is saved and application has read permissions