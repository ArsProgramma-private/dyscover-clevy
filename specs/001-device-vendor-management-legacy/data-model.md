# Data Model: Device Vendor Management

**Date**: November 23, 2025
**Feature**: specs/001-device-vendor-management/spec.md

## Entities

### SupportedDevice
Represents a USB device identified by Vendor ID and Product ID.

**Attributes**:
- `vendorId` (string): 4-character hexadecimal Vendor ID (e.g., "04B4")
- `productId` (string): 4-character hexadecimal Product ID (e.g., "0101")
- `description` (string, optional): Human-readable device description

**Constraints**:
- `vendorId` and `productId` must be exactly 4 hexadecimal characters (0-9, A-F, case-insensitive)
- Combination of `vendorId` and `productId` must be unique across all devices
- `description` limited to 100 characters if provided

**Relationships**:
- None (standalone entity)

**Lifecycle**:
- Created: When developer adds new device to configuration
- Updated: When description is modified
- Deleted: When developer removes device from configuration

## Validation Rules

### Format Validation
- VID/PID must match regex: `^[0-9A-Fa-f]{4}$`
- Invalid characters or wrong length → reject with error message

### Uniqueness Validation
- Check against existing devices before add/update
- Duplicate VID/PID → reject with error message

### Data Integrity
- Configuration file corruption → fallback to defaults
- Partial invalid entries → skip invalid, load valid ones

## Storage Schema

### JSON Structure
```json
{
  "version": "1.0",
  "devices": [
    {
      "vendorId": "04B4",
      "productId": "0101",
      "description": "Cypress Semiconductor USB Device"
    },
    {
      "vendorId": "0CD3",
      "productId": "320F",
      "description": "Generic USB Device"
    }
  ]
}
```

### Migration Strategy
- Version field for future schema changes
- Backward compatibility: Ignore unknown fields
- Forward compatibility: Add defaults for missing fields