# Tasks: Language-Specific Resource Optimization

**Feature Branch**: `006-language-resource-optimization`  
**Input**: Specification from `spec.md` with clarifications from 2025-11-27  
**Context**: Optimize build to include only language-specific audio and TTS files based on selected language configuration

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (US1, US2, US3)
- Include exact file paths in descriptions

## Constitution Alignment

- Code Quality: Resource extraction script kept under 50 LOC per function
- Testing Standards: Build validation tested with intentional error injection
- Performance: Build-time extraction minimizes reconfiguration overhead
- Exceptions: None anticipated

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create resource extraction tooling infrastructure

- [X] T001 Create scripts directory structure for resource extraction at `scripts/build-tools/`
- [X] T002 [P] Create CMake module for resource manifest generation at `cmake/ResourceManifest.cmake`
- [X] T003 [P] Document build system changes in `specs/006-language-resource-optimization/implementation-notes.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core resource extraction and validation infrastructure - MUST complete before ANY user story implementation

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T004 Create resource extraction script `scripts/build-tools/extract-audio-resources.cmake` that parses Keys.cpp and extracts audio filenames from KeyTranslationEntry::sound fields
- [X] T005 Add CMake function `generate_resource_manifest()` in `cmake/ResourceManifest.cmake` that invokes extraction script and creates manifest file
- [X] T006 Create validation script `scripts/build-tools/validate-resources.cmake` that checks if all files in manifest exist in `res/data/`
- [X] T007 Add error collection logic to validation script that accumulates all missing files before reporting (not fail-fast)
- [X] T008 Integrate pre-build resource extraction into CMakeLists.txt using `add_custom_command` with PRE_BUILD event

**Checkpoint**: Foundation ready - resource extraction and validation pipeline established

---

## Phase 3: User Story 1 - Smaller Distribution Packages (Priority: P1) 🎯 MVP

**Goal**: Build packages contain only language-specific resources, reducing size by 40%+

**Independent Test**: Build for Dutch (LANGUAGE=nl), verify output contains only Dutch audio files and no Flemish resources, measure package size reduction

### Implementation for User Story 1

- [X] T009 [P] [US1] Modify `scripts/build-tools/extract-audio-resources.cmake` to filter audio files by LANGUAGE preprocessor define (check for `#if defined __LANGUAGE_NL__` or `#elif defined __LANGUAGE_NL_BE__` blocks)
- [X] T010 [P] [US1] Update TTS resource collection in CMakeLists.txt (lines ~616-623) to use TTS_LANG and TTS_VOICE variables instead of copying all tts/data/* files
- [X] T011 [US1] Replace `file(GLOB SOUND_FILES "res/data/*.wav")` at line ~605 in CMakeLists.txt with manifest-based file list from resource extraction
- [X] T012 [US1] Update `add_custom_command` for audio copying (lines ~607-612) to use filtered manifest list instead of SOUND_FILES glob
- [X] T013 [US1] Update `install(FILES ${SOUND_FILES} DESTINATION audio)` at line ~617 to use filtered manifest list
- [X] T014 [US1] Verify Dutch build: Run `cmake -DLANGUAGE=nl` and `make`, check build/audio/ contains only Dutch-referenced files
- [X] T015 [US1] Verify Flemish build: Run `cmake -DLANGUAGE=nl_be` and `make`, check build/audio/ contains only Flemish-referenced files
- [X] T016 [US1] Measure package size reduction: Compare new Dutch/Flemish packages vs baseline all-inclusive package, verify ≥40% reduction

**Checkpoint**: Builds produce language-specific packages with verified size reduction

---

## Phase 4: User Story 2 - Compile-Time Resource Validation (Priority: P2)

**Goal**: Build fails with comprehensive error message if any referenced audio file is missing

**Independent Test**: Temporarily rename an audio file referenced in Keys.cpp, run build, verify build fails with complete list of all missing files

### Implementation for User Story 2

- [X] T017 [P] [US2] Enhance `scripts/build-tools/validate-resources.cmake` to parse manifest and check existence of each file in `res/data/`
- [X] T018 [P] [US2] Add error accumulation to validation script: collect all missing files into list before generating error message
- [X] T019 [US2] Format comprehensive error message showing: "Missing audio files for language [LANGUAGE]: file1.wav, file2.wav, file3.wav referenced in layouts: [layout names]"
- [X] T020 [US2] Add validation for TTS resources: check that ${TTS_LANG}.db, ${TTS_LANG}.fsa, ${TTS_LANG}.fst, ${TTS_VOICE}.db, ${TTS_VOICE}.fon, ${TTS_VOICE}.udb exist in res/data/tts/data/
- [X] T021 [US2] Integrate validation into CMakeLists.txt as mandatory build step via `execute_process` during configuration phase
- [X] T022 [US2] Create test case: Rename a.wav to a_missing.wav, run cmake, verify build fails with error listing a.wav and which layouts reference it
- [X] T023 [US2] Create test case: Add non-existent "xyz.wav" to Keys.cpp g_dutchDefault, verify build fails with clear error
- [X] T024 [US2] Verify error timing: Ensure validation completes and reports within 10 seconds of build start

**Checkpoint**: Build validation catches all missing resources with comprehensive error reporting

---

## Phase 5: User Story 3 - Multi-Language Build Support (Priority: P3)

**Goal**: Support building packages for all languages without manual intervention, enable CI/CD automation

**Independent Test**: Run multi-language build script, verify separate packages generated for nl and nl_be with correct naming and language-specific resources

### Implementation for User Story 3

- [X] T025 [P] [US3] Create multi-language build script `scripts/build-all-languages.sh` that iterates through supported languages (nl, nl_be)
- [X] T026 [P] [US3] Add language-specific build directory naming to script: build-nl/, build-nl_be/ to avoid conflicts
- [X] T027 [US3] Implement parallel build support in script using `cmake --build --parallel` for each language
- [X] T028 [US3] Add package naming logic to ensure distinct filenames: Dyscover-nl-${VERSION}-${PLATFORM}.zip, Dyscover-nl_be-${VERSION}-${PLATFORM}.zip
- [X] T029 [US3] Update build-windows.ps1 (line ~176) to support `-Language` parameter properly integrated with resource extraction
- [X] T030 [US3] Update scripts/quick-build-linux.sh to support language switching without CMake reconfiguration via pre-build extraction
- [X] T031 [US3] Test multi-language build: Run `scripts/build-all-languages.sh`, verify two distinct packages created
- [X] T032 [US3] Verify package independence: Unpack both packages, confirm Dutch package has zero Flemish files and vice versa
- [X] T033 [US3] Create CI/CD workflow example in `.github/workflows/multi-language-build.yml` demonstrating parallel language builds

**Checkpoint**: All languages can be built independently with proper isolation and naming

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, testing, and final validation

- [X] T034 [P] Update CMakeLists.txt comments to explain resource extraction and manifest generation process
- [X] T035 [P] Add developer documentation in `specs/006-language-resource-optimization/developer-guide.md` explaining how to add new languages
- [X] T036 [P] Update README.md with new build process details and language-specific packaging information
- [X] T037 Create validation script `scripts/validate-resource-optimization.sh` that builds all languages and verifies package size reductions
- [X] T038 Run complete validation: Build all languages, verify SC-001 through SC-007 success criteria from spec.md
- [X] T039 Performance verification: Confirm resource extraction adds <5 seconds to build time
- [X] T040 Clean up any temporary debugging output from extraction scripts

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on T001-T003 completion - BLOCKS all user stories
- **User Stories (Phase 3-5)**: All depend on Foundational (T004-T008) completion
  - US1 can proceed independently after Phase 2
  - US2 depends on US1 completion (T009-T016) to have filtering in place for validation
  - US3 can start after Phase 2 but benefits from US1+US2 validation being complete
- **Polish (Phase 6)**: Depends on all user stories (T009-T033) being complete

### User Story Dependencies

- **User Story 1 (P1 - T009-T016)**: Can start after Foundational - No dependencies on other stories
- **User Story 2 (P2 - T017-T024)**: Depends on US1 filtering logic (needs manifest to validate) - Start after T011 complete
- **User Story 3 (P3 - T025-T033)**: Can start after Foundational - Independent of US1/US2 but should verify their functionality

### Within Each User Story

**User Story 1 Flow**:
1. T009, T010 can run in parallel (different concerns: audio extraction vs TTS)
2. T011 must complete before T012, T013 (manifest-based copying depends on extraction)
3. T014, T015 can run in parallel (independent build tests)
4. T016 runs last (requires completed builds to measure)

**User Story 2 Flow**:
1. T017, T018, T019, T020 can run in parallel (different validation aspects)
2. T021 integrates validation (requires T017-T020)
3. T022, T023, T024 can run in parallel (independent test cases)

**User Story 3 Flow**:
1. T025, T026, T027, T028 can be developed in parallel (different script aspects)
2. T029, T030 can run in parallel (different platform build scripts)
3. T031-T033 run sequentially (each validates previous work)

### Parallel Opportunities

```bash
# Phase 1: All setup tasks in parallel
T001 & T002 & T003

# Phase 2: Foundational tasks are sequential (each builds on previous)

# User Story 1: Parallel extraction development
T009 & T010  # Audio and TTS extraction independently

# User Story 1: Parallel build validation
T014 & T015  # Dutch and Flemish builds independently

# User Story 2: Parallel validation development  
T017 & T018 & T019 & T020  # Different validation concerns

# User Story 2: Parallel test cases
T022 & T023 & T024  # Independent error injection tests

# User Story 3: Parallel script development
T025 & T026 & T027 & T028  # Multi-language build script components
T029 & T030  # Platform-specific build script updates

# Phase 6: All documentation in parallel
T034 & T035 & T036  # Different documentation targets
```

---

## Parallel Example: User Story 1

**Scenario**: Implementing resource filtering

```bash
# Step 1: Develop extraction and TTS filtering in parallel
Developer A: T009 - Audio extraction filtering in extract-audio-resources.cmake
Developer B: T010 - TTS resource variable-based collection in CMakeLists.txt

# Step 2: Integrate manifest-based copying (sequential)
Developer A: T011 - Replace glob with manifest
Developer A: T012 - Update build-time copying
Developer A: T013 - Update install rules

# Step 3: Validate builds in parallel
Developer A: T014 - Test Dutch build
Developer B: T015 - Test Flemish build

# Step 4: Final verification
Developer A: T016 - Measure package size reduction
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T003) - ~2 hours
2. Complete Phase 2: Foundational (T004-T008) - ~1 day
   - **CRITICAL CHECKPOINT**: Resource extraction pipeline working
3. Complete Phase 3: User Story 1 (T009-T016) - ~2 days
   - **STOP and VALIDATE**: Build Dutch package, verify 40% size reduction
4. **MVP COMPLETE**: Language-specific packages ready for distribution

**Estimated MVP Delivery**: 3-4 days

### Incremental Delivery

1. **Sprint 1**: Setup + Foundational → Resource extraction pipeline ready
2. **Sprint 2**: User Story 1 → MVP (language-specific packages) → **DEPLOY**
3. **Sprint 3**: User Story 2 → Build validation → **DEPLOY** (improved DX)
4. **Sprint 4**: User Story 3 → Multi-language automation → **DEPLOY** (CI/CD ready)
5. **Sprint 5**: Polish → Documentation and final validation

### Parallel Team Strategy

With 2 developers:

1. **Week 1**: Both complete Setup + Foundational together
2. **Week 2**: 
   - Dev A: US1 (T009-T016) - Package size optimization
   - Dev B: US3 (T025-T033) - Multi-language build infrastructure
3. **Week 3**:
   - Dev A: US2 (T017-T024) - Build validation (requires US1 manifest)
   - Dev B: Polish (T034-T040) - Documentation
4. **Week 4**: Joint validation and deployment

---

## Notes

- All tasks T001-T008 (Foundational) must complete before any user story work begins
- Resource extraction script should parse Keys.cpp at CMake configure time, not every build
- Validation script must collect ALL errors before failing (per clarification session requirement)
- Keep flat res/data/ structure (per clarification - no subdirectories by language)
- Keys.cpp remains single file (per clarification - no restructuring)
- Pre-build extraction enables CLI language switching without CMake reconfiguration
- Success criteria SC-001 through SC-007 from spec.md must all pass for completion
- Each user story delivers independent value and can be deployed separately

---

## Success Criteria Mapping

- **SC-001, SC-002**: Verified by T016 (package size measurements)
- **SC-003**: Verified by T014, T015 (build completion tests)
- **SC-004**: Verified by T022, T023, T024 (error reporting tests)
- **SC-005**: Verified by T031 (multi-language build test)
- **SC-006**: Verified by T032 (package content audit)
- **SC-007**: Verified by T022, T023 (error injection tests)

All success criteria validated in T038 (comprehensive validation script).
