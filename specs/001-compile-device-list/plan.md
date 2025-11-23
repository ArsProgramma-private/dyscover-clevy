# Implementation Plan: Compile Device List

**Branch**: `001-compile-device-list` | **Date**: 2025-11-23 | **Spec**: `spec.md`
**Input**: Feature specification from `/specs/001-compile-device-list/spec.md`

## Summary

Embed supported USB Vendor/Product IDs directly in the binary, replacing the current runtime JSON configuration. Device detection code (Windows: `DeviceWindows.cpp`, Linux: `DeviceLinux.cpp`) will reference a static `constexpr` array (or `std::array`) of `SupportedDevice` entries. Removing the external file eliminates end‑user tampering while preserving existing detection logic pathways.

## Technical Context

**Language/Version**: C++ (current project standard, appears C++11/14)  
**Primary Dependencies**: wxWidgets (UI), platform APIs (SetupAPI/ConfigMgr32 on Windows), libudev on Linux. No new external deps required.  
**Storage**: N/A (data embedded in code, no persistence)  
**Testing**: Existing test harness (custom). Will add unit tests for new static list access + integration tests verifying detection with simulated VID/PID inputs.  
**Target Platform**: Linux x86_64, Windows x86_64 (same as existing)  
**Project Type**: Cross‑platform desktop application  
**Performance Goals**: Maintain startup <2.0s p95 (no regression expected; removal of file IO may offer micro‑improvement)  
**Constraints**: Memory <150MB RSS steady state; maintain detection latency; no dynamic allocation in hot path beyond existing.  
**Scale/Scope**: Small contained change (one data list, minor refactor of config load removal, docs update).  

UNKNOWN / NEEDS CLARIFICATION:
- Exact current C++ standard enforced by build (C++11 vs C++14 vs later) → affects use of `constexpr`/structured bindings.
- Location for static list: new header `SupportedDevices.h` vs embed inside `Device.h`.
- Whether vendor list should preserve `description` field for logging (spec keeps description optional).

## Constitution Check (Pre-Design)

Code Quality: Planned functions remain <50 LOC; only small refactors (remove JSON load). No complexity violations anticipated.
Testing Standards: Will introduce failing tests: (1) unit test for `isSupported(vid,pid)` using embedded list; (2) integration test path for Windows/Linux enumeration using stub/mocked list when possible.
UX Consistency & Accessibility: No new UI; requires documentation update and removal of config file instructions. Localization: remove any strings referencing runtime config modification (NEEDS SCAN).
Performance: Slight positive (remove file read). Memory negligible change. Startup unaffected or improved.
Exceptions: None anticipated; no gate violations.

Gate Status: PASS (all mandatory gates satisfied; unknowns slated for Phase 0 research).

## Project Structure

### Documentation (this feature)

```text
specs/001-compile-device-list/
├── plan.md
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # (likely empty or README noting N/A)
└── tasks.md             # Phase 2 (later)
```

### Source Code Adjustments

```text
src/
├── Device.h / Device.cpp              # Remove JSON config usage
├── DeviceWindows.cpp                  # Switch to static list reference
├── DeviceLinux.cpp                    # Switch to static list reference
├── SupportedDevices.h (NEW)           # Static constexpr array + helper
└── (Remove) DeviceConfig.*            # Delete if only used for this feature

tests/
├── unit/DeviceStaticListTest.cpp      # New unit tests
└── integration/DeviceDetectionStaticListTest.cpp
```

**Structure Decision**: Single project layout retained; adding one header for embedded list. Removing obsolete config component and its tests. Documentation updated (`doc/vendor-id-management.md`).

## Complexity Tracking

No violations; section not required post-design unless future refactor introduces complexity.

## Constitution Check (Post-Design)

Code Quality: Design keeps functions small; static list header simple; deletion of DeviceConfig reduces code surface.
Testing Standards: New unit/integration tests planned; removal of JSON path requires test updates (ensure previous JSON tests removed or migrated). Coverage impact positive (simpler code). Failing tests will be authored before implementation.
UX & Accessibility: No UI changes; documentation update will remove end-user modification instructions—ensure localization keys referencing config removed (scan required during implementation).
Performance: Static list lookup O(n) for small n acceptable; option documented to optimize if list grows >200 entries. Startup may slightly improve due to removed file IO.
Exceptions: None.

Re-check Result: PASS — proceed to Phase 2 task breakdown.

## Performance Confirmation (Post-Implementation)

File IO removed (previous JSON load). Static array iteration is O(n) with n≈1; negligible CPU. Memory footprint unchanged aside from removing dynamic vectors. Startup path simplified; expected micro-improvement (<1ms). No additional allocations in IsSupported (uses std::string normalization only). Future optimization comment added to SupportedDevices.h for growth scenario.

## Final Constitution Summary

All gates satisfied: Code Quality (no large or complex functions), Testing (unit + integration simulations added), Accessibility (documentation updated, no new UI), Performance (budgets maintained), No exceptions required.
