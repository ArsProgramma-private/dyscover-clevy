# Task Breakdown: Compile Device List

Feature: Compile Device List (`001-compile-device-list`)
Spec: `spec.md` | Plan: `plan.md`

## Phase 1: Setup

- [ ] T001 Resolve duplicate spec prefix by archiving `specs/001-device-vendor-management/` (rename to `specs/001-device-vendor-management-legacy/`)
- [ ] T002 Create feature branch `001-compile-device-list` (if missing) from `main`
- [ ] T003 Add placeholder header `src/SupportedDevices.h`
- [ ] T004 [P] Add initial unit test file scaffold `tests/unit/DeviceStaticListTest.cpp`
- [ ] T005 Confirm C++ standard in `CMakeLists.txt` (ensure at least C++14)

## Phase 2: Foundational

- [ ] T006 Remove JSON config generation logic from `src/Device.cpp`
- [ ] T007 Remove `src/DeviceConfig.cpp` and `src/DeviceConfig.h` references from `CMakeLists.txt`
- [ ] T008 Delete obsolete JSON config tests `tests/unit/DeviceConfigTest.cpp` (if exists)
- [ ] T009 [P] Delete obsolete integration tests referencing runtime JSON (file path scan `tests/integration/`)
- [ ] T010 Add build guard to ignore existing user config file (no read attempt) in `src/Device.cpp`

## Phase 3 (US1): Modify Device Support

- [ ] T011 [US1] Implement `SupportedDevice` struct in `src/SupportedDevices.h`
- [ ] T012 [P] [US1] Implement `inline constexpr SupportedDevice SUPPORTED_DEVICES[]` with initial entries
- [ ] T013 [US1] Implement helper `bool IsSupported(const std::string& vid, const std::string& pid)` in `src/SupportedDevices.h`
- [ ] T014 [P] [US1] Add normalization helper (uppercase/pad) `NormalizeHex4` in `src/SupportedDevices.h`
- [ ] T015 [US1] Write failing unit tests for `IsSupported` in `tests/unit/DeviceStaticListTest.cpp`
- [ ] T016 [US1] Implement passing logic to satisfy tests
- [ ] T017 [US1] Document modification steps in updated `doc/vendor-id-management.md`

## Phase 4 (US2): Prevent Runtime Modification

- [ ] T018 [US2] Remove any remaining references to `supported_devices.json` in `src/Device.cpp`
- [ ] T019 [P] [US2] Remove path resolution logic for config file (Linux & Windows) in `src/Device.cpp`
- [ ] T020 [US2] Add comment guard explaining compile-time list approach in `src/SupportedDevices.h`
- [ ] T021 [US2] Add unit test ensuring file read is not performed (mock or sentinel) in `tests/unit/DeviceStaticListTest.cpp`
- [ ] T022 [US2] Update plan for performance note removal of IO (append note in `plan.md`)
- [ ] T023 [US2] Update quickstart to reflect final static API if needed `quickstart.md`

## Phase 5 (US3): Maintain Device Detection

- [ ] T024 [US3] Refactor `src/DeviceWindows.cpp` to replace config usage with `IsSupported`
- [ ] T025 [P] [US3] Refactor `src/DeviceLinux.cpp` to replace config usage with `IsSupported`
- [ ] T026 [US3] Add integration test harness enumerating fake VID/PID pairs in `tests/integration/DeviceDetectionStaticListTest.cpp`
- [ ] T027 [US3] Write failing integration tests for detection (supported vs unsupported)
- [ ] T028 [US3] Implement logic updates to pass integration tests
- [ ] T029 [US3] Verify logging still includes `description` when present

## Phase 6: Polish & Cross-Cutting

- [ ] T030 Remove vendor config migration code remnants (search for `supported_devices.json` literals)
- [ ] T031 [P] Scan localization files for obsolete keys referencing runtime config in `lang/` and remove them
- [ ] T032 Add migration note to README `README.md`
- [ ] T033 Add future optimization comment (switch to `unordered_set` if >200 devices) in `SupportedDevices.h`
- [ ] T034 Performance review: confirm no new allocations in hot path (document in `plan.md`)
- [ ] T035 Accessibility doc check: update any references to interactive config in `doc/vendor-id-management.md`
- [ ] T036 Final Constitution re-check (append to `plan.md` summary section)

## Dependency Graph (User Story Order)

1. US1 (Modify Device Support) → provides static list and API
2. US2 (Prevent Runtime Modification) → depends on static list existing
3. US3 (Maintain Device Detection) → depends on API from US1 and removal from US2

Polish phase depends on completion of US1–US3.

## Parallel Execution Examples

- Early: T003, T004, T005 can run parallel after branch setup.
- US1: T012 and T014 can proceed while T011 struct definition drafted.
- US3: T024 and T025 refactors can proceed in parallel after IsSupported ready.
- Polish: T031 localization scan parallel with T034 performance review.

## Implementation Strategy

MVP Scope: Complete US1 (static list + IsSupported + tests + doc update). Delivers compile-time control of devices.
Incremental Delivery:
1. MVP (US1) → static embedded list operational.
2. US2 → guarantees immutability; removes runtime artifacts.
3. US3 → restores full detection using new API; integration tests ensure parity.
4. Polish → cleanup, performance confirmation, documentation & localization consistency.

## Format Validation

All tasks follow: `- [ ] T### [P] [US#] Description with file path` (where applicable). Story labels only present in phases 3–5.

## Counts

- Total Tasks: 36
- US1 Tasks: 7 (T011–T017)
- US2 Tasks: 6 (T018–T023)
- US3 Tasks: 6 (T024–T029)
- Parallelizable Tasks Marked: 8

## Independent Test Criteria per Story

- US1: Unit tests confirm `IsSupported` logic with known entries.
- US2: Tests ensure absence of file IO and immutability of list.
- US3: Integration tests verify platform detection produces correct supported/unsupported decisions.

## MVP Recommendation

Ship after US1 completion for immediate compile-time enforcement; defer removal + refactors to subsequent increments if needed.
