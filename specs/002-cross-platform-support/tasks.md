# Tasks: Cross-Platform Support (Linux, macOS, Windows, ChromeOS)

**Input**: Design documents from `specs/002-cross-platform-support/`
**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`

Constitution Alignment:
- Failing test tasks appear before implementation for reusable logic.
- No new UI surfaces; accessibility impact limited to ensuring keyboard event integrity.
- Performance validation via lightweight benchmarking hooks (startup + detection latency) late in polish phase.

## Phase 1: Setup (Shared Infrastructure)
**Purpose**: Prepare repository for new platform abstraction layer and test scaffolding.

 - [X] T001 Create `src/platform/` directory structure in src/platform/
 - [X] T002 Add placeholder CMake grouping comment in `CMakeLists.txt` for platform sources CMakeLists.txt
 - [X] T003 [P] Create initial mock headers for detectors in src/platform/DeviceDetector.h
 - [X] T004 [P] Create initial mock headers for keyboard handler in src/platform/KeyboardHandler.h
 - [X] T005 [P] Create initial mock headers for audio controller in src/platform/AudioController.h
 - [X] T006 [P] Create resource locator abstraction header in src/platform/ResourceLocator.h
 - [X] T007 Add PlatformUtils header in src/platform/PlatformUtils.h
 - [X] T008 Initialize unit test directory for new tests in tests/unit/
 - [X] T009 Add compile definitions for platform detection (ensure existing macro usage consistent) in CMakeLists.txt

---
## Phase 2: Foundational (Blocking Prerequisites)
**Purpose**: Abstraction contracts, factories, and CMake integration before per-platform logic.

 - [X] T010 Implement DeviceDetector interface + factory stub in src/platform/DeviceDetector.cpp
 - [X] T011 Implement KeyboardHandler interface + factory stub in src/platform/KeyboardHandler.cpp
 - [X] T012 Implement AudioController interface + factory stub in src/platform/AudioController.cpp
 - [X] T013 Implement ResourceLocator unified implementation in src/platform/ResourceLocator.cpp
 - [X] T014 [P] Implement PlatformUtils helpers (VID/PID extraction generic) in src/platform/PlatformUtils.cpp
 - [X] T015 Wire new factories into application core (create injection point) in src/Core.cpp
 - [X] T016 Update `CMakeLists.txt` to conditionally add new platform source groups CMakeLists.txt
- [ ] T017 Add failing unit test skeleton for device detector in tests/unit/DeviceDetectorTest.cpp
- [ ] T018 Add failing unit test skeleton for keyboard translate logic in tests/unit/KeyboardHandlerTranslateTest.cpp
- [ ] T019 Add failing unit test skeleton for audio fallback in tests/unit/AudioControllerFallbackTest.cpp
- [ ] T020 Add failing unit test skeleton for resource locator paths in tests/unit/ResourceLocatorPathsTest.cpp
- [ ] T021 Add failing unit test skeleton for hotplug callback (mock) in tests/unit/DeviceHotplugCallbackTest.cpp
- [ ] T022 Integrate mocks enabling tests to compile (return NOT IMPLEMENTED) in src/platform/DeviceDetector.cpp
- [ ] T023 [P] Document factory usage in quickstart.md specs/002-cross-platform-support/quickstart.md
- [ ] T024 Ensure build passes with new abstractions (no functionality yet) in CMakeLists.txt

Checkpoint: Foundational abstractions compile + failing tests in place.

---
## Phase 3: User Story 1 - Device Detection Works on All Platforms (Priority: P1) [US1] 🎯 MVP
**Goal**: Cross-platform detection (Windows, Linux, macOS, ChromeOS) + presence monitoring.
**Independent Test**: Run detector tests + manual device plug/unplug on one platform; keyboard/audio not required.

### Tests (write & make fail first)
- [ ] T025 [P] [US1] Implement failing VID/PID match test logic in tests/unit/DeviceDetectorTest.cpp
- [ ] T026 [P] [US1] Implement failing hotplug mock event test in tests/unit/DeviceHotplugCallbackTest.cpp

### Implementation
- [ ] T027 [P] [US1] Implement Windows detector using existing logic (refactor from DeviceWindows.cpp) in src/platform/DeviceDetectorWindows.cpp
- [ ] T028 [P] [US1] Refactor Linux detector from DeviceLinux.cpp into src/platform/DeviceDetectorLinux.cpp
- [ ] T029 [P] [US1] Implement macOS detector (IOKit enumeration) in src/platform/DeviceDetectorMac.cpp
- [ ] T030 [P] [US1] Implement ChromeOS detector (reuse Linux + sandbox fallback) in src/platform/DeviceDetectorChromeOS.cpp
- [ ] T031 [US1] Implement polling fallback logic for restricted platforms in src/platform/DeviceDetector.cpp
- [ ] T032 [US1] Integrate BT hardware ID check (from Windows code) cross-platform if feasible in src/platform/DeviceDetectorWindows.cpp
- [ ] T033 [US1] Add capability flags (HOTPLUG_EVENTS/POLLING_ONLY) in src/platform/DeviceDetector.h
- [ ] T034 [US1] Update SupportedDevices usage to new detector in src/SupportedDevices.h
- [ ] T035 [US1] Update Core to use detector factory & listener in src/Core.cpp
- [ ] T036 [US1] Remove direct device enumeration logic from legacy `DeviceWindows.cpp` / `DeviceLinux.cpp` in src/DeviceWindows.cpp
- [ ] T037 [US1] Adapt tests to pass by implementing mocks in tests/unit/DeviceDetectorTest.cpp
- [ ] T038 [US1] Add detection latency benchmark (optional) in tests/unit/DeviceDetectorBenchmark.cpp
- [ ] T039 [US1] Documentation update detection section in specs/002-cross-platform-support/quickstart.md

Checkpoint: Device presence detection functional across platforms.

---
## Phase 4: User Story 2 - Keyboard Input Handling Across Platforms (Priority: P2) [US2]
**Goal**: Intercept, translate, inject key events; Caps Lock state cross-platform.
**Independent Test**: Run translation and injection tests without requiring device presence (detector mocked).

### Tests
 - [X] T040 [P] [US2] Extend translation test with printable/non-printable coverage in tests/unit/KeyboardHandlerTranslateTest.cpp
 - [X] T041 [P] [US2] Add Caps Lock state test (mock platform) in tests/unit/KeyboardHandlerTranslateTest.cpp
 - [X] T042 [P] [US2] Add keystroke injection permission failure test in tests/unit/KeyboardHandlerPermissionTest.cpp

### Implementation
 - [X] T043 [P] [US2] Implement Windows keyboard handler (refactor from KeyboardWindows.cpp) in src/platform/KeyboardHandlerWindows.cpp
 - [X] T044 [P] [US2] Implement Linux keyboard handler (evdev + XKB translation) in src/platform/KeyboardHandlerLinux.cpp
 - [X] T045 [P] [US2] Implement macOS keyboard handler (CGEventTap + TISInputSource) in src/platform/KeyboardHandlerMac.cpp
 - [X] T046 [P] [US2] Implement ChromeOS handler (restricted: event interception disabled gracefully) in src/platform/KeyboardHandlerChromeOS.cpp
 - [X] T047 [US2] Add permission probing logic & state enum in src/platform/KeyboardHandler.cpp
 - [X] T048 [US2] Implement dead key safe translation fallback (empty if unsupported) in src/platform/KeyboardHandlerLinux.cpp
 - [X] T049 [US2] Add injection capability fallback: return false + log once in src/platform/KeyboardHandlerMac.cpp
 - [X] T050 [US2] Wire handler into Core replacing direct Windows hook usage in src/Core.cpp
 - [X] T051 [US2] Remove direct logic from legacy KeyboardWindows.cpp in src/KeyboardWindows.cpp
 - [X] T052 [US2] Validate handler with unit tests (make pass) in tests/unit/KeyboardHandlerTranslateTest.cpp
 - [X] T053 [US2] Update quickstart with permissions note in specs/002-cross-platform-support/quickstart.md

Checkpoint: Cross-platform keyboard handling available.

---
## Phase 5: User Story 4 - Resources Load Correctly on All Platforms (Priority: P2) [US4]
**Goal**: Unified resource loading (icons, splash, audio, translations) functioning across OS variants including ChromeOS.
**Independent Test**: Launch app and verify icons & paths resolved without platform condition leaks.

### Tests
- [ ] T054 [P] [US4] Extend resource locator paths test: icons/audio/tts/translations in tests/unit/ResourceLocatorPathsTest.cpp

### Implementation
- [ ] T055 [P] [US4] Refactor existing `ResourceLoader.cpp` to use ResourceLocator interface in src/ResourceLoader.cpp
- [ ] T056 [P] [US4] Implement platform-specific icon suffix logic (ico/icns) in src/platform/ResourceLocator.cpp
- [ ] T057 [US4] Ensure ChromeOS uses Linux icon strategy in src/platform/ResourceLocator.cpp
- [ ] T058 [US4] Replace direct path concatenations in codebase with ResourceLocator calls in src/Core.cpp
- [ ] T059 [US4] Migrate splash bitmap loading behind abstraction in src/ResourceLoader.cpp
- [ ] T060 [US4] Update quickstart resource section in specs/002-cross-platform-support/quickstart.md

Checkpoint: Resource loading unified and verified.

---
## Phase 6: User Story 3 - Audio Volume Control Works Cross-Platform (Priority: P3) [US3]
**Goal**: Cross-platform volume get/set with graceful fallback.
**Independent Test**: Call volume API on platform; returns correct or unsupported state without crash.

### Tests
- [ ] T061 [P] [US3] Implement failing volume get test variations (supported vs unsupported) in tests/unit/AudioControllerFallbackTest.cpp
- [ ] T062 [P] [US3] Add backend selection test (PulseAudio vs ALSA fallback) in tests/unit/AudioControllerFallbackTest.cpp

### Implementation
- [ ] T063 [P] [US3] Implement Windows volume logic (refactor waveOut functions) in src/platform/AudioControllerWindows.cpp
- [ ] T064 [P] [US3] Implement Linux volume (libpulse primary) in src/platform/AudioControllerLinux.cpp
- [ ] T065 [P] [US3] Implement ALSA mixer fallback logic in src/platform/AudioControllerLinux.cpp
- [ ] T066 [P] [US3] Implement macOS CoreAudio volume logic in src/platform/AudioControllerMac.cpp
- [ ] T067 [P] [US3] Implement ChromeOS fallback (unsupported set) in src/platform/AudioControllerChromeOS.cpp
- [ ] T068 [US3] Add backend selection logic & probing in src/platform/AudioController.cpp
- [ ] T069 [US3] Integrate audio controller usage replacing `AudioLevel.cpp` calls in src/AudioLevel.cpp
- [ ] T070 [US3] Update documentation fallback explanations in specs/002-cross-platform-support/quickstart.md
- [ ] T071 [US3] Make tests pass by implementing mocks/backends in tests/unit/AudioControllerFallbackTest.cpp

Checkpoint: Audio volume abstraction functional with fallbacks.

---
## Phase 7: Polish & Cross-Cutting Concerns
**Purpose**: Stabilization, performance, documentation, deprecations.

- [ ] T072 [P] Remove deprecated legacy platform files (device/keyboard) after migration in src/DeviceWindows.cpp
- [ ] T073 [P] Add logging normalization (single tag per subsystem) in src/Core.cpp
- [ ] T074 [P] Add performance micro-benchmark (startup + detection latency) in tests/unit/PerformanceSmokeTest.cpp
- [ ] T075 Refactor any >50 LOC functions introduced (if any) in src/platform/KeyboardHandlerLinux.cpp
- [ ] T076 Add README cross-platform build instructions in README.md
- [ ] T077 [P] Accessibility regression check (keyboard flow unaffected) in docs/vendor-id-management.md
- [ ] T078 Final update of quickstart with verified platform notes in specs/002-cross-platform-support/quickstart.md
- [ ] T079 Prepare release packaging adjustments for macOS resources in CMakeLists.txt
- [ ] T080 Add CI matrix note (macOS & Linux) in README.md
- [ ] T081 Final code style pass (clang-tidy config if present) in src/platform/
- [ ] T082 Create integration test enable flag docs in specs/002-cross-platform-support/quickstart.md

---
## Dependencies & Execution Order

Phase Dependencies:
- Phase 1 → None
- Phase 2 → Requires Phase 1
- Phase 3 (US1) → Requires Phase 2
- Phase 4 (US2) → Requires Phase 2 (independent of US1 runtime but can leverage detection)
- Phase 5 (US4) → Requires Phase 2 (may optionally wait for US1 for validation paths)
- Phase 6 (US3) → Requires Phase 2 (independent of US1/US2 logic)
- Phase 7 → Requires chosen user stories complete (minimum US1 for MVP)

User Story Independence:
- US1 provides device detection but US2/US3/US4 can operate with mocks if implemented before US1.
- Each story has its own test criteria and can be demoed alone.

Within Stories:
- Tests (failing) → platform implementations → integration wiring → refactor legacy → pass tests.

### Parallel Opportunities
- Interface header creation (T003–T007) parallel.
- Platform implementations per OS (e.g., T027–T030) parallel.
- Keyboard per platform (T043–T046) parallel.
- Audio backends (T063–T067) parallel.
- Resource locator refactor + path updates (T055–T059) parallel.

### Parallel Examples
- Run T027, T028, T029, T030 concurrently for device detector per OS.
- Run T063, T064, T066 in parallel while ALSA fallback T065 proceeds after initial Linux backend stub.

---
## Independent Test Criteria Summary
- US1: Plug/unplug device; unit tests for VID/PID & hotplug; benchmark latency <500ms.
- US2: Key translation correctness (printables, modifiers); Caps Lock state; injection permission fallback.
- US4: Icon, splash, audio, translations loaded using locator with no direct platform macro usage in callers.
- US3: Volume get/set behavior across supported backends; unsupported surfaces return false gracefully.

---
## Implementation Strategy
- MVP = Complete US1 (device detection) after Foundational phases.
- Incremental: US2 (keyboard) + US4 (resources) in parallel → US3 (audio).
- Final polish consolidates logging, performance, documentation, and removes deprecated code.

---
## Notes
- [P] tasks assume distinct files; avoid editing same file in parallel.
- Legacy files only removed after new abstractions stable (T072).
- ChromeOS treated as Linux variant; feature gaps documented.
- Accessibility unaffected; regression check ensures no keyboard trap or broken focus paths.

