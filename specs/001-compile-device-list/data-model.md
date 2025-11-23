# Phase 1: Data Model - Compile Device List

## Entities

### SupportedDevice
| Field        | Type         | Constraints                                | Notes                     |
|--------------|--------------|---------------------------------------------|---------------------------|
| vendorId     | const char*  | 4 hex chars, uppercase (e.g., "04B4")      | Stored as string literal  |
| productId    | const char*  | 4 hex chars, uppercase (e.g., "0101")      | Stored as string literal  |
| description  | const char*  | Optional, may be nullptr or empty string    | For logging/diagnostics   |

## Relationships
None. Flat static list embedded at compile time.

## Representation
```cpp
struct SupportedDevice {
    const char* vendorId;    // 4 uppercase hex
    const char* productId;   // 4 uppercase hex
    const char* description; // optional
};

inline constexpr SupportedDevice SUPPORTED_DEVICES[] = {
    {"04B4", "0101", "Cypress Semiconductor Device"}
    // Additional entries added by developers
};
```

## Validation Rules
- Developer responsibility: ensure hex strings are length 4 and valid characters [0-9A-F].
- At runtime, comparison normalizes probed VID/PID to uppercase and pads if necessary before matching.
- Duplicate (vendorId, productId) entries prohibited (manual review).

## State & Transitions
- No dynamic state. List fixed at build time; changes require recompile.

## Removal of Previous Model
- `DeviceConfig` class and JSON schema obsolete; removed to avoid confusion.

## Performance Considerations
- Linear search over small array (<50 expected). If growth exceeds ~200 entries, switch to `std::unordered_set<uint32_t>` keyed by combined VID/PID.

## Future Extension Points
- Optional: add `flags` field for capability bits (e.g., requires firmware quirk).
- Optional: add build-time generation script if list size grows.
