# Research Findings: Device Vendor Management

**Date**: November 23, 2025
**Feature**: specs/001-device-vendor-management/spec.md

## Resolved Technical Decisions

### Persistence Format
**Decision**: Use JSON format for device configuration file  
**Rationale**: JSON is human-readable, widely supported in C++, requires no external dependencies, and can be easily parsed/validated. Aligns with modern configuration practices.  
**Alternatives Considered**: 
- INI format: Simpler for basic key-value but lacks structure for device arrays
- XML: More verbose and complex parsing without benefits for this use case

### Testing Framework
**Decision**: Use existing testing framework in codebase (if any) or Google Test  
**Rationale**: Maintains consistency with project standards. Google Test is widely used in C++ projects and integrates well with CI.  
**Alternatives Considered**: 
- Custom test harness: Would require maintenance overhead
- Boost.Test: Adds dependency complexity

### VID/PID Uniqueness
**Decision**: Enforce unique VID/PID pairs  
**Rationale**: Prevents configuration errors and simplifies lookup logic. Device identification should be deterministic.  
**Alternatives Considered**: 
- Allow duplicates: Could lead to unpredictable behavior
- Unique VID only: Too restrictive, same VID can have multiple PIDs

### Expected Scale
**Decision**: Support up to 100 device entries  
**Rationale**: Based on typical USB device ecosystem, this covers manufacturer expansions without performance impact.  
**Alternatives Considered**: 
- Unlimited: Unnecessary for this use case
- Fixed small limit (10): Too restrictive for future growth

### Linux Device Detection
**Decision**: Implement using udev library for device enumeration  
**Rationale**: Standard Linux approach for device detection, provides VID/PID information via sysfs.  
**Alternatives Considered**: 
- Manual sysfs parsing: Error-prone and platform-specific
- libusb: Overkill for detection-only use case

## Implementation Patterns

### Configuration File Location
- Store in user config directory (platform-specific)
- Fallback to application directory if user config unavailable
- File name: `supported_devices.json`

### Error Handling
- Invalid JSON: Log error, use defaults
- Missing file: Create with current hardcoded devices
- Malformed entries: Skip invalid, log warnings

### Thread Safety
- Configuration loading: Single-threaded during startup
- Runtime access: Read-only after initialization