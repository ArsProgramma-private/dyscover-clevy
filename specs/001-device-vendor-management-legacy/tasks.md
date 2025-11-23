# Tasks: Device Vendor Management

**Input**: Design documents from `/specs/001-device-vendor-management/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Included as required by Constitution Testing Standards Principle.

Constitution Alignment:
- Add failing test tasks FIRST for reusable logic (Testing Standards Principle).
- Include accessibility verification tasks for new UI (UX Consistency Principle). - No new UI planned.
- Include benchmark/performance validation tasks when changes affect latency or memory (Performance Principle).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/` at repository root
- Paths shown below assume single project - adjust based on plan.md structure

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [x] T001 Create DeviceConfig.h header file in src/DeviceConfig.h
- [x] T002 Create DeviceConfig.cpp implementation file in src/DeviceConfig.cpp
- [x] T003 Create DeviceConfig.h header file in src/DeviceConfig.h
- [x] T004 Update CMakeLists.txt to include new DeviceConfig source files

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T005 Implement config file loading in src/Config.cpp
- [x] T006 Implement config validation (hex format, uniqueness) in src/Config.cpp
- [x] T007 Implement default config creation in src/Config.cpp
- [x] T008 Implement config saving in src/Config.cpp
- [x] T009 Add config error handling and logging in src/Config.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - View Current Supported Devices (Priority: P1) 🎯 MVP

**Goal**: Developers can see the current list of supported Vendor IDs and Product IDs

**Independent Test**: Load the application and verify it uses the config file for device detection instead of hardcoded values

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [x] T010 [P] [US1] Unit test for config loading in tests/unit/ConfigTest.cpp
- [x] T011 [P] [US1] Unit test for config validation in tests/unit/ConfigTest.cpp
- [x] T012 [P] [US1] Integration test for device detection using config in tests/integration/DeviceIntegrationTest.cpp

### Implementation for User Story 1

- [x] T013 [US1] Update DeviceWindows::DoesContainClevyKeyboard to use config instead of hardcoded checks in src/DeviceWindows.cpp
- [x] T014 [US1] Implement DeviceLinux::IsClevyKeyboardPresent using udev and config in src/DeviceLinux.cpp
- [x] T015 [US1] Update Device base class to integrate config loading in src/Device.cpp

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Add New Supported Device (Priority: P2)

**Goal**: Developers can add new Vendor ID and Product ID combinations to the supported list

**Independent Test**: Add a new device to the config file and verify the application recognizes it

### Tests for User Story 2 ⚠️

- [x] T016 [P] [US2] Unit test for hex format validation in tests/unit/ConfigTest.cpp
- [x] T017 [P] [US2] Unit test for duplicate detection in tests/unit/ConfigTest.cpp
- [x] T018 [P] [US2] Integration test for config file changes in tests/integration/DeviceIntegrationTest.cpp

### Implementation for User Story 2

- [x] T019 [US2] Add hex format validation for VID/PID in src/Config.cpp
- [x] T020 [US2] Add duplicate VID/PID detection in src/Config.cpp
- [x] T021 [US2] Add config reload on file changes in src/Config.cpp

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Remove Supported Device (Priority: P3)

**Goal**: Developers can remove Vendor ID and Product ID combinations from the supported list

**Independent Test**: Remove a device from the config file and verify the application no longer recognizes it

### Tests for User Story 3 ⚠️

- [x] T022 [P] [US3] Unit test for config file error handling in tests/unit/ConfigTest.cpp
- [x] T023 [P] [US3] Unit test for default config creation in tests/unit/ConfigTest.cpp
- [x] T024 [P] [US3] Integration test for invalid config handling in tests/integration/DeviceIntegrationTest.cpp

### Implementation for User Story 3

- [x] T025 [US3] Add graceful handling of invalid config entries in src/Config.cpp
- [x] T026 [US3] Add default config creation when file is missing in src/Config.cpp
- [x] T027 [US3] Add config file backup and recovery in src/Config.cpp

**Checkpoint**: All user stories should now be independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [x] T028 [P] Documentation updates in specs/001-device-vendor-management/
- [x] T029 Code cleanup and refactoring in src/
- [x] T030 Performance optimization for config loading
- [x] T031 [P] Additional unit tests in tests/unit/
- [x] T032 Run quickstart.md validation
- [x] T033 Update README.md with device management information

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Builds on US1 but independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Builds on US1/US2 but independently testable

### Within Each User Story

- Tests (if included) MUST be written and FAIL before implementation
- Config logic before device detection updates
- Core implementation before error handling
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- All tests for a user story marked [P] can run in parallel
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
Task: "Unit test for config loading in tests/unit/ConfigTest.cpp"
Task: "Unit test for config validation in tests/unit/ConfigTest.cpp"
Task: "Integration test for device detection using config in tests/integration/DeviceIntegrationTest.cpp"

# Launch implementation tasks sequentially (dependencies):
Task: "Update DeviceWindows::DoesContainClevyKeyboard to use config instead of hardcoded checks in src/DeviceWindows.cpp"
Task: "Implement DeviceLinux::IsClevyKeyboardPresent using udev and config in src/DeviceLinux.cpp"
Task: "Update Device base class to integrate config loading in src/Device.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo
4. Add User Story 3 → Test independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1
   - Developer B: User Story 2
   - Developer C: User Story 3
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Verify tests fail before implementing
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence