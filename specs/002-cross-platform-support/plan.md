# Implementation Plan: Cross-Platform Support (Linux, macOS, Windows, ChromeOS)

**Branch**: `002-cross-platform-support` | **Date**: 2025-11-23 | **Spec**: `specs/002-cross-platform-support/spec.md`
**Input**: Feature specification from `specs/002-cross-platform-support/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Enable full cross-platform support for Dyscover Clevy across Linux, macOS, Windows, and ChromeOS by implementing platform abstraction layers for device detection, keyboard input handling, audio volume control, and resource loading. Current gaps: missing macOS & ChromeOS device/keyboard/audio implementations, stubbed Linux keyboard & audio, and incomplete resource handling for ChromeOS/macOS. Approach: introduce unified interfaces (`IDeviceDetector`, `IKeyboardHandler`, `IAudioController`, `IResourceLocator`) with per-platform implementations compiled conditionally in CMake; add detection/event flow tests and abstraction boundary unit tests before implementation (Red→Green). Leverage existing Windows + Linux code as reference; design macOS layer around IOKit + Quartz Event Taps; Linux keyboard via `evdev` (preferred) with fallback to X11; ChromeOS treated as constrained Linux derivative (udev + restricted input events) with graceful degradation. Audio: abstract volume operations with Windows (waveOut), macOS (CoreAudio), Linux (PulseAudio/PipeWire via `pactl`/`libpulse` or ALSA fallback). Resource loading: unify path resolution avoiding duplicate conditional compilation in callers.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17 (CMake enforces; optional C++20 flag)  
**Primary Dependencies**: wxWidgets (UI), PortAudio (optional speech/audio), libudev (Linux device enumeration), CoreAudio (macOS volume) NEEDS CLARIFICATION (exact macOS device APIs: IOKit IORegistry + IOHIDManager), PulseAudio/PipeWire vs ALSA (Linux volume path) NEEDS CLARIFICATION, ChromeOS constraints NEEDS CLARIFICATION  
**Storage**: N/A (runtime in-memory only)  
**Testing**: CTest + custom unit tests (existing); add new unit tests for abstraction interfaces; integration tests gated by hardware (feature will add conditional integration tests)  
**Target Platform**: Windows (x86/x64), Linux (x86/x64), macOS (x86_64, Apple Silicon future NEEDS CLARIFICATION), ChromeOS (Linux kernel derivative)  
**Project Type**: Single desktop cross-platform application (monolithic executable)  
**Performance Goals**: Device detection latency <500ms, key press to speech ≤200ms p95 (unchanged), startup ≤2.0s p95, volume operations <100ms  
**Constraints**: Code quality: -Wall -Wextra -Werror; function LOC ≤50; complexity ≤10; cross-platform conditional logic isolated; accessibility (focus order, contrast) unaffected by platform differences  
**Scale/Scope**: ~40 existing source files + +6–10 new headers/implementations (platform abstractions and macOS/ChromeOS variants)  

Unresolved / NEEDS CLARIFICATION items to research in Phase 0:
1. Precise macOS API stack for vendor/product ID enumeration (IOKit vs HID Manager)  
2. Feasibility & security of low-level keyboard hooks on macOS (CGEventTap vs NSEventMonitor)  
3. ChromeOS restrictions: access to udev & evdev inside packaged application  
4. Linux volume control: prefer PulseAudio/PipeWire—choose library vs external command invocation  
5. Fallback strategy for volume on ChromeOS (may restrict direct mixer access)  
6. Apple Silicon build differences (universal binary vs x86_64 only initial target)  
7. Need for abstraction layer namespacing (e.g., `platform/` subdirectory) vs keeping flat in `src/`  
8. Testing approach for device presence without physical hardware (mock layer design)  
9. Strategy for layout-specific keystroke translation cross-platform (dead keys handling)  
10. Security / permission requirements for injecting keystrokes on macOS & ChromeOS  

## Constitution Check (Initial)

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Mandatory Gates (from Dyscover Clevy Constitution):
- Code Quality: Plan introduces ≤10 new interfaces; each implementation function constrained ≤50 LOC (complex logic split). No violations anticipated.
- Testing Standards: For each interface a failing unit test will precede implementation (listed below). Coverage: new abstraction headers + implementations target ≥90% line coverage; overall project impact +2–3%.
- UX Consistency & Accessibility: No new UI surfaces; resource loading changes preserve existing icon paths; ensure alt text / names unchanged.
- Performance: Added abstraction should have negligible startup impact (<10ms). Device detection path may introduce platform branching; ensure no extra full-tree enumeration loops.
- Exceptions: Potential temporary absence of ChromeOS keystroke injection (security) marked with expiry (one release) if unresolved.

Planned Failing Tests (outline):
1. `DeviceDetectorTest` - enumerates mock devices list → detects supported VID/PID
2. `KeyboardHandlerTranslateTest` - given key code & modifiers returns expected character (layout stub)
3. `AudioControllerFallbackTest` - when platform reports unsupported volume set returns false gracefully
4. `ResourceLocatorPathsTest` - ensures paths compose correctly per platform enum
5. `DeviceHotplugCallbackTest` - simulated event triggers listener notification

Performance Impact Estimate: Minimal; added indirection layers (virtual dispatch) negligible vs existing event handling.

Accessibility Impact: None functionally; ensure keyboard translation does not break key labeling or event suppression logic.

Gate Verdict (Pre-Phase 0): PASS (pending research resolution of NEEDS CLARIFICATION items)

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

```text
src/
├── platform/
│   ├── DeviceDetector.h/.cpp (abstract + factory)
│   ├── DeviceDetectorWindows.cpp
│   ├── DeviceDetectorLinux.cpp
│   ├── DeviceDetectorMac.cpp
│   ├── DeviceDetectorChromeOS.cpp (may alias Linux w/ constraints)
│   ├── KeyboardHandler.h/.cpp (abstract)
│   ├── KeyboardHandlerWindows.cpp
│   ├── KeyboardHandlerLinux.cpp
│   ├── KeyboardHandlerMac.cpp
│   ├── KeyboardHandlerChromeOS.cpp
│   ├── AudioController.h/.cpp
│   ├── AudioControllerWindows.cpp
│   ├── AudioControllerLinux.cpp
│   ├── AudioControllerMac.cpp
│   ├── AudioControllerChromeOS.cpp
│   ├── ResourceLocator.h/.cpp (unified implementation; minor platform branching)
│   └── PlatformUtils.h/.cpp (shared helpers: VID/PID parsing, path join)
tests/
├── unit/
│   ├── DeviceDetectorTest.cpp
│   ├── KeyboardHandlerTranslateTest.cpp
│   ├── AudioControllerFallbackTest.cpp
│   ├── ResourceLocatorPathsTest.cpp
│   └── DeviceHotplugCallbackTest.cpp
└── integration/
  ├── DeviceEnumerationIntegrationTest.cpp (conditional BUILD_INTEGRATION_TESTS)
  └── KeyboardInputIntegrationTest.cpp (conditional)
```

**Structure Decision**: Adopt a `platform/` subdirectory under `src/` to encapsulate abstractions and per-platform implementations, preventing leakage into existing core files (e.g., `DeviceLinux.cpp`). Existing Windows/Linux files will be migrated or wrapped; initial implementation may co-exist during transition with deprecation notes.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Potential temporary ChromeOS keystroke injection omission | Security/API limitations | Direct injection may require elevated permissions; fallback logs + user notice |
| Volume control multi-backend (PulseAudio + ALSA fallback) | Broad Linux distro coverage | Single backend risks incompatibility (PipeWire-only or ALSA-only systems) |

---

### Phase 0 Plan (Research Tasks)
Will produce `research.md` resolving 10 clarification items. Output: decisions, rationale, alternatives table.

### Phase 1 Plan (Design Outputs)
Artifacts: `data-model.md` (entities + relationships), `contracts/` (interface header skeletons in markdown pseudo-code), `quickstart.md` (build matrix & dependencies). Update Constitution Check with concrete test list & complexity confirmations.

### Phase 2 (Deferred to /speckit.tasks)
Task decomposition & scheduling after design approval.

---

Post-Phase 1 Constitution Re-check placeholder will be appended after artifacts creation.

## Constitution Check (Post-Phase 1)

Status after design artifacts:
- Code Quality: Interfaces split; no planned implementation function expected >50 LOC (complex paths like PulseAudio init will be segmented). PASS
- Testing Standards: Five failing unit tests defined; mocks enable deterministic coverage; added integration test hooks. Estimated coverage uplift: +2.5%. PASS
- UX Consistency & Accessibility: No new UI; resource paths unchanged; PASS
- Performance: Abstraction introduces negligible overhead; detection and translation strategies bounded; PASS
- Exceptions: ChromeOS keystroke injection & macOS Accessibility permission—documented with one-release expiry. PASS WITH NOTED EXCEPTIONS (tracked in Complexity table).

Gate Verdict (Post-Phase 1): PASS — proceed to /speckit.tasks phase for implementation scheduling.
