# Phase 0 Research: Compile Device List

**Date**: 2025-11-23
**Feature**: Compile Device List (Spec: spec.md)

## Unknowns Resolved

### 1. C++ Standard Version
- **Decision**: Assume C++14 minimum (project uses features consistent with C++11/14; using `constexpr` for array OK).
- **Rationale**: Existing code compiles without modern C++17 constructs; C++14 supports needed features for static initialization.
- **Alternatives Considered**: C++17 (would allow `std::string_view`); rejected to avoid widening scope.

### 2. Location for Static Device List
- **Decision**: Introduce `src/SupportedDevices.h` (header-only) with `inline constexpr std::array<SupportedDevice, N>`.
- **Rationale**: Centralizes device data; avoids polluting `Device.h`; header-only simplifies inclusion.
- **Alternatives Considered**: Embedding in `Device.cpp`; rejected due to less discoverability.

### 3. Preserve Description Field?
- **Decision**: Keep `description` as `const char*` (optional) for logging/debug only.
- **Rationale**: Provides operator insight; zero runtime config, minimal overhead.
- **Alternatives Considered**: Drop description; rejected—reduces diagnostics clarity.

### 4. Removal Strategy for DeviceConfig Components
- **Decision**: Remove `DeviceConfig.cpp/.h` and associated JSON logic & tests.
- **Rationale**: Fully replaced by compile-time list; leaving code risks confusion.
- **Alternatives Considered**: Deprecate with stub; rejected—unnecessary maintenance.

### 5. Testing Approach Without File IO
- **Decision**: Add unit tests targeting `isSupported(vid,pid)` using known entries and non-entries.
- **Rationale**: Ensures correctness of embedded list and parsing of hex values.
- **Alternatives Considered**: Mock platform enumeration; deferred to integration tests.

### 6. Hex Representation Handling
- **Decision**: Store VID/PID in uppercase hex (4 chars) as `const char*`; comparison normalizes input by uppercasing and padding.
- **Rationale**: Matches existing spec formatting; avoids dynamic allocation.
- **Alternatives Considered**: Store as `uint16_t`; rejected to keep logs consistent with hex strings.

### 7. Documentation Update Plan
- **Decision**: Replace `doc/vendor-id-management.md` content with compile-time instructions + migration note.
- **Rationale**: Prevent stale guidance referencing JSON file.
- **Alternatives Considered**: Add new doc file; rejected—single source of truth preferred.

## Architectural Notes
- No runtime persistence; static list compiled into binary.
- Detection flow unchanged except source of supported IDs.
- Performance unchanged or marginally improved (skip file read + parse).

## Risk Assessment
- Low: Removal of runtime config path may break existing customization workflows if any dependent tooling assumed JSON editability.
- Mitigation: Provide migration notice in updated documentation.

## Open Items (None)
All unknowns resolved; no outstanding clarifications.

## Next Steps
Proceed to Phase 1 design artifacts: `data-model.md`, `quickstart.md`, `contracts/` (likely N/A), then update agent context.
