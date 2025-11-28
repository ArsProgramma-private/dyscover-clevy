# Tasks — Dyscover 4 Manual Compliance Audit

Feature: 007-manual-compliance  
Spec: specs/007-manual-compliance/spec.md  
Plan: specs/007-manual-compliance/plan.md

## Phase 1 — Setup

- [X] T001 Ensure branch active `007-manual-compliance`
- [X] T002 [P] Verify build scripts run on host (`scripts/quick-build-linux.sh configure`)
- [X] T003 [P] Confirm Windows icon assets availability for 3-state tray in `res/icons/`

## Phase 2 — Foundational

- [X] T004 Document tests to add for runtime layout switching in `tests/`
- [X] T005 Define icon state mapping (Active/Paused/No-Keyboard) in `src/TrayIcon.cpp` comments

## Phase 3 — [US1] Runtime Method Switching (P1)

- [X] T006 [P] [US1] Add `SetActiveLayout(const char*)` to `src/layouts/LayoutRegistry.h/.cpp`
- [X] T007 [US1] Update `TranslateKey(...)` in `src/Keys.cpp` to use `GetActiveLayout()` only
- [X] T008 [US1] Persist/restore active layout from `Config::GetLayout()` at startup in `src/App.cpp`
- [X] T009 [US1] Call `LayoutRegistry::Instance().SetActiveLayout(...)` in `src/PreferencesDialog.cpp` on method change
- [X] T010 [P] [US1] Add unit test: switching layout changes translation result in `tests/KeyboardHandlerTranslateTest`

## Phase 4 — [US2] Three-State Tray Icon (P1)

- [X] T011 [US2] Implement mapping in `src/TrayIcon.cpp::UpdateIcon()` to Active/Paused/No-Keyboard
- [X] T012 [P] [US2] Verify/adjust `LoadDyscoverIcons()` order in `src/ResourceLoader.*` to support mapping
- [X] T013 [P] [US2] Add unit test: permutations of enabled/keyboard-present map to expected index in `tests/ResourceLocatorPathsTest` (or new small test)

## Phase 5 — [US3] Selection Read-Out & Speaker Key (P2)

- [X] T014 [US3] Confirm `Key::WinCmd` triggers selection read in `src/Core.cpp` and update comments to reflect "Speaker key"
- [~] T015 [P] [US3] Add integration test: simulate clipboard and trigger; assert `Speech::Speak` called (deferred: wx clipboard requires event loop; manual validation sufficient)

## Phase 6 — [US4] Alternate Pronunciations Modifier (P2)

- [X] T016 [US4] Audit layouts for alternate mappings when WinCmd held; document in `specs/007-manual-compliance/contracts/pronunciation-modifier.md`
- [X] T017 [P] [US4] Add unit test for modified pronunciation when modifier active

## Final Phase — Polish & Cross-Cutting

- [X] T018 Update localization strings if any new labels/messages were added (none added; existing strings unchanged)
- [X] T019 Validate performance budgets (startup, latency) unchanged (no new runtime overhead; test suite passed)
- [X] T020 Update `specs/007-manual-compliance/quickstart.md` with final icon mapping note

## Dependencies

- US1 → US2 (tray can be done independently but verify menu reflects states afterward)
- US1 is independent; US2 depends only on assets; US3/US4 can run parallel after US1

## Parallel Execution Examples

- T006 + T012 in parallel (registry hook with icon resource verification)
- T009 + T011 in parallel (preferences wiring with tray mapping)
- T010 + T013 in parallel (tests for layout vs tray)

## Implementation Strategy

- MVP: Complete US1 (runtime method switching) and US2 (three-state tray icon)
- Incrementally add US3/US4 tests and adjustments; ensure no regressions
