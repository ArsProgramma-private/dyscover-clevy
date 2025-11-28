# Implementation Plan: Dyscover 4 Manual Compliance Audit

**Branch**: `007-manual-compliance` | **Date**: 2025-11-28 | **Spec**: `specs/007-manual-compliance/spec.md`
**Input**: Feature specification from `/specs/007-manual-compliance/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Ensure the shipped application matches the capabilities described in the Clevy Dyscover 4 manual. Key changes required in code:
- Enable runtime method switching (Clevy Dyscover / Clevy Keyboard / KWeC) to affect key→sound/keystroke mappings immediately.
- Implement a three-state tray icon (Active, Paused, No Keyboard for full builds).
- Confirm and standardize the speaker key mapping to Windows key (`WinCmd`) for selection read-out and pronunciation modifier behavior.
Non-functional gates: preserve startup and latency budgets, keep UI accessible, and maintain high test coverage for modified logic.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17  
**Primary Dependencies**: wxWidgets (UI, tray), PortAudio (Audio), librstts (TTS), custom platform abstractions (DeviceDetector, KeyboardHandler, AudioController)  
**Storage**: N/A (wxFileConfig INI; Windows registry for autostart)  
**Testing**: CTest with unit/integration binaries (e.g., DeviceDetectorTest, KeyboardHandler*Test); extend with new unit tests for runtime layout switching and tray state mapping  
**Target Platform**: Windows (primary/manual), Linux and macOS builds present; features validated primarily on Windows  
**Project Type**: Single native desktop application  
**Performance Goals**: Speech response ≤200ms p95 from key press to audible output (per Constitution); startup ≤2.0s p95; steady-state RSS <150MB  
**Constraints**: Maintain accessibility (keyboard operability, visible focus, localization), avoid increasing CPU >40% during continuous speech; no dropped key events  
**Scale/Scope**: Two languages (NL, NL_BE); three methods (Dyscover, Keyboard, KWeC)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Mandatory Gates (from Dyscover Clevy Constitution):
- Code Quality: No planned functions >50 LOC or cyclomatic >10 without justification.
- Testing Standards: Each reusable logic unit lists a failing test to write first; coverage impact estimated.
- UX Consistency & Accessibility: New UI surfaces define keyboard focus order + localization keys.
- Performance: Proposed changes list expected impact on startup time, memory, latency, CPU.
- Exceptions: Any anticipated temporary violation documented with expiry.

Gate Assessment (pre-Phase 0):
- Code Quality: Planned changes are localized (Keys/Layouts registry hook, TrayIcon update). No >50 LOC functions expected. PASS.
- Testing: Add unit tests for TranslateKey honoring runtime layout, and tray icon state selection under permutations. Add integration test for selection read with WinCmd. PENDING tests to author. WARN.
- UX & A11y: No new UI panels; tray icon state mapping only. Ensure labels/localization updated for any new strings. PASS.
- Performance: Runtime layout switching is lookup-only; negligible overhead. Tray icon selection is constant-time. NO anticipated regression. PASS.
- Exceptions: None anticipated. PASS.

Gate Assessment (post-Phase 1):
- Code Quality: Designs keep changes local; no functions >50 LOC expected. PASS.
- Testing: Contracts defined; unit + integration tests identified. Ensure coverage does not regress (<add tests>). WARN (action required in implementation phase).
- UX & A11y: No new dialogs; tray icon mapping only. Localization stable. PASS.
- Performance: No material impact anticipated. PASS.
- Exceptions: None. PASS.

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
# [REMOVE IF UNUSED] Option 1: Single project (DEFAULT)
src/
├── models/
├── services/
├── cli/
└── lib/

tests/
├── contract/
├── integration/
└── unit/

# [REMOVE IF UNUSED] Option 2: Web application (when "frontend" + "backend" detected)
backend/
├── src/
│   ├── models/
│   ├── services/
│   └── api/
└── tests/

frontend/
├── src/
│   ├── components/
│   ├── pages/
│   └── services/
└── tests/

# [REMOVE IF UNUSED] Option 3: Mobile + API (when "iOS/Android" detected)
api/
└── [same as backend above]

ios/ or android/
└── [platform-specific structure: feature modules, UI flows, platform tests]
```

**Structure Decision**: Single native app. Relevant modules:
- `src/Keys.cpp`, `src/layouts/*` — translation + layout registry
- `src/PreferencesDialog.*`, `src/Config.*` — runtime method setting
- `src/TrayIcon.*` — icon state mapping
- `src/Core.*` — key handling, selection read-out (WinCmd), buffering
- `tests/` — extend with new unit/integration tests for this feature

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
