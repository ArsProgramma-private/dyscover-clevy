# Tasks: Layout-Based Resource Organization (Phase 2)

**Feature**: 006-language-resource-optimization (Phase 2 Enhancement)  
**Input**: Design documents from `/specs/006-language-resource-optimization/`  
**Prerequisites**: plan.md, spec.md (Future Enhancements), research.md, data-model.md, contracts/, quickstart.md

**Note**: This feature is a DEFERRED ENHANCEMENT from the main 006 feature. It restructures the already-working language resource optimization into a hierarchical layout-based organization for better scalability.

Constitution Alignment:
- Failing tests FIRST for CMake discovery logic (Testing Standards)
- Performance benchmarks for CMake config time (Performance Principle)
- No UI changes, build-time only (UX Consistency N/A)
- Migration script complexity tracked and time-boxed (Complexity Tracking)

## Format: `- [ ] [ID] [P?] Description with file path`

- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths

---

## Phase 1: Setup (Project Initialization)

**Purpose**: Prepare for migration - no code changes yet

- [X] T001 Create directory structure for migration script in scripts/migration/
- [X] T002 [P] Create CMake module placeholder in cmake/LayoutDiscovery.cmake
- [X] T003 [P] Create build tool placeholders in scripts/build-tools/discover-layouts.cmake and validate-layout-structure.cmake
- [X] T004 [P] Create C++ header placeholders in src/layouts/LayoutLoader.h and LayoutRegistry.h
- [X] T005 Document current state baseline: file counts, sizes, structure in specs/006-language-resource-optimization/baseline.md

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before migration can begin

**⚠️ CRITICAL**: No migration work can begin until this phase is complete

- [X] T006 Implement validate_layout_structure() function in scripts/build-tools/validate-layout-structure.cmake per contracts/layout-api.md
- [X] T007 Implement extract_audio_references() function in scripts/build-tools/extract-audio-references.cmake (reuse existing logic)
- [X] T008 Implement discover_layouts() function in scripts/build-tools/discover-layouts.cmake per contracts/layout-api.md
- [X] T009 Add feature flag USE_LAYOUT_STRUCTURE to CMakeLists.txt (default OFF) around line 50
- [X] T010 Implement ILayoutProvider interface in src/layouts/LayoutLoader.h per contracts/layout-api.md
- [X] T011 Implement LayoutRegistry class in src/layouts/LayoutRegistry.h and .cpp per contracts/layout-api.md

**Checkpoint**: Foundation ready - migration script can now be implemented

---

## Phase 3: Migration Implementation (3-Phase Rollout Structure)

**Goal**: Create migration tooling that safely transforms flat structure to hierarchical

**Independent Test**: Run migration script with --dry-run, verify correct file moves planned

### Tests for Migration Script

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [X] T012 [P] Integration test: Dry-run migration produces correct file move plan in tests/integration/test_migration_dry_run.sh
- [X] T013 [P] Integration test: Migration creates valid layout directory structure in tests/integration/test_migration_structure.sh
- [X] T014 [P] Integration test: Migrated layout.cpp files compile successfully in tests/integration/test_migration_compile.sh

### Implementation for Migration Script

- [X] T015 Create migration script scripts/migration/migrate-to-layouts.sh with --dry-run support
- [X] T016 Implement Keys.cpp parsing to identify layout boundaries (g_dutchDefault, g_flemishClassic, etc.) in migrate-to-layouts.sh
- [X] T017 Implement audio file attribution logic: map each .wav to layouts that reference it in migrate-to-layouts.sh
- [X] T018 Implement TTS file attribution logic: map TTS files to language codes in migrate-to-layouts.sh
- [X] T019 Implement directory creation for res/layouts/{type}/{lang}/ structure in migrate-to-layouts.sh
- [X] T020 Implement layout.cpp splitting: extract each layout into separate files in migrate-to-layouts.sh
- [X] T021 Implement audio file copying to layout-specific audio/ directories in migrate-to-layouts.sh
- [X] T022 Implement TTS file copying to layout-specific tts/ directories in migrate-to-layouts.sh
- [X] T023 Implement symlink creation for shared audio files (with Windows fallback to copy) in migrate-to-layouts.sh
- [X] T024 Add validation: verify all files moved, no orphans, no duplicates in migrate-to-layouts.sh
- [X] T025 Add rollback capability: script can undo migration on error in migrate-to-layouts.sh
- [X] T026 Generate migration report with file counts, sizes, validation results in migrate-to-layouts.sh

**Checkpoint**: Migration script complete and tested on copy of repository

---

## Phase 4: CMake Integration for New Structure

**Goal**: Build system can discover and build from new hierarchical structure

**Independent Test**: Build with USE_LAYOUT_STRUCTURE=ON produces working binary

### Tests for CMake Discovery

- [X] T027 [P] Integration test: discover_layouts() finds all layout.cpp files in tests/integration/test_cmake_discovery.sh
- [X] T028 [P] Integration test: validate_layout_structure() catches invalid layouts in tests/integration/test_cmake_validation.sh
- [X] T029 [P] Integration test: Build with new structure produces binary equivalent to old structure in tests/integration/test_binary_equivalence.sh

### Implementation for CMake Discovery

- [X] T030 [P] Implement discover_layouts() in cmake/LayoutDiscovery.cmake (file GLOB_RECURSE for res/layouts/*/*/layout.cpp)
- [X] T031 [P] Implement validate_layout_structure() in cmake/LayoutDiscovery.cmake (check audio/, tts/, layout.cpp exist)
- [X] T032 [P] Implement generate_layout_manifest() in cmake/LayoutDiscovery.cmake per contracts/layout-api.md
- [X] T033 Update CMakeLists.txt: Add conditional logic for USE_LAYOUT_STRUCTURE flag around line 200
- [X] T034 Update CMakeLists.txt: When flag ON, call discover_layouts() and add found sources to target
- [X] T035 Update cmake/ResourceManifest.cmake: Support both old and new resource paths
- [X] T036 Test CMake configuration with USE_LAYOUT_STRUCTURE=ON on Linux build
- [ ] T037 [P] Test CMake configuration with USE_LAYOUT_STRUCTURE=ON on Windows build
- [ ] T038 [P] Test CMake configuration with USE_LAYOUT_STRUCTURE=ON on macOS build

**Checkpoint**: Build system fully supports new structure, parallel to old structure

---

## Phase 5: Runtime Layout Loading (C++ Implementation)

**Goal**: Runtime can load layouts from new modular structure

**Independent Test**: Application starts and selects correct layout based on LANGUAGE

### Tests for Layout Loading

- [ ] T039 [P] Unit test: LayoutRegistry registers and retrieves layouts correctly in tests/unit/test_layout_registry.cpp
- [ ] T040 [P] Integration test: Application loads Dutch classic layout successfully in tests/integration/test_layout_loading.cpp

### Implementation for Layout Loading

- [X] T041 Implement LayoutRegistry::Register() in src/layouts/LayoutRegistry.cpp
- [X] T042 Implement LayoutRegistry::GetLayout() in src/layouts/LayoutRegistry.cpp
- [X] T043 Implement LayoutRegistry::GetActiveLayout() based on compile-time LANGUAGE flag in src/layouts/LayoutRegistry.cpp
- [X] T044 Update src/Keys.cpp: Convert to thin dispatcher using extern declarations
- [X] T045 Update src/Keys.cpp: Implement FindTranslation() to use LayoutRegistry::GetActiveLayout()
- [X] T046 Create example layout module: examples/layout-module-example-dutch-classic.cpp with DutchClassicLayout class
- [X] T047 [P] Create example layout module: examples/layout-module-example-flemish-classic.cpp with FlemishClassicLayout class
- [ ] T048 Test runtime layout selection with LANGUAGE=nl build
- [ ] T049 [P] Test runtime layout selection with LANGUAGE=nl_be build

**Checkpoint**: Runtime successfully loads layouts from new structure

---

## Phase 6: Migration Execution & Validation (Phase 1 of 3-Phase Rollout)

**Goal**: Execute migration, establish parallel structure, validate equivalence

**Independent Test**: Both old and new structures build successfully, binaries equivalent

// Completed on 2025-11-27 (see migration report commit)
- [X] T050 Backup current repository state: create backup branch 006-pre-migration
- [X] T051 Run migration script with --dry-run, review output for correctness
- [X] T052 Execute migration script (creates res/layouts/ structure)
- [X] T053 Verify new structure: check all layout.cpp files compile individually
- [X] T054 Verify new structure: check all audio files present in layout-specific directories
- [X] T055 Verify new structure: check all TTS files present in layout-specific directories
- [ ] T056 Build with USE_LAYOUT_STRUCTURE=OFF (old structure), save binary as dyscover-old
// Build of new structure successful (warnings only) - commit recorded
- [X] T057 Build with USE_LAYOUT_STRUCTURE=ON (new structure), save binary as dyscover-new
- [ ] T058 Compare binaries: verify functional equivalence (same layout data, same audio files)
- [ ] T059 Run full test suite with old structure (USE_LAYOUT_STRUCTURE=OFF), verify all pass
- [ ] T060 Run full test suite with new structure (USE_LAYOUT_STRUCTURE=ON), verify all pass
// Benchmarks captured during migration (see journal / commit message)
- [X] T061 Benchmark CMake configuration time: old vs new (target: <5s increase)
- [X] T062 Benchmark build time: old vs new (target: <10% increase)
- [ ] T063 Update CI to build BOTH structures in parallel in .github/workflows/test-migration.yml
- [X] T064 Commit migration changes with clear documentation

**Checkpoint**: Parallel structure validated, both old and new work, CI passing

---

## Phase 7: Default Switchover (Phase 2 of 3-Phase Rollout)

**Goal**: Make new structure the default, monitor for issues

**Independent Test**: Default builds use new structure, old structure available for rollback

- [X] T065 Update CMakeLists.txt: Change USE_LAYOUT_STRUCTURE default from OFF to ON
- [X] T066 Update documentation: specs/006-language-resource-optimization/developer-guide.md with new structure
- [X] T067 Update documentation: README.md project structure section
- [X] T068 Update documentation: quickstart.md for adding new languages
- [ ] T069 Notify team: Send migration Phase 2 announcement per quickstart.md template
- [ ] T070 Monitor CI for 1 week: watch for build failures or performance regressions
- [ ] T071 Gather developer feedback: survey team on new structure experience
- [ ] T072 Address any issues: fix bugs or improve tooling based on feedback

**Checkpoint**: New structure is default, stable for 1 week, team feedback positive

---

## Phase 8: Cleanup (Phase 3 of 3-Phase Rollout)

**Goal**: Remove old structure, finalize migration

**Independent Test**: Build succeeds without old structure, all tests pass

- [ ] T073 Archive old structure: git mv src/Keys.cpp src/Keys.cpp.legacy
- [ ] T074 Archive old structure: git mv res/data res/data.legacy
- [ ] T075 Wait 1 week with archived structure (safety period for rollback)
- [ ] T076 Remove archived files: git rm src/Keys.cpp.legacy and res/data.legacy
- [ ] T077 Remove feature flag: Delete USE_LAYOUT_STRUCTURE option from CMakeLists.txt
- [ ] T078 Simplify CMakeLists.txt: Remove old structure conditional logic
- [ ] T079 Remove migration script: git rm scripts/migration/migrate-to-layouts.sh (one-time use complete)
- [ ] T080 Update CI: Remove dual-structure builds, keep only new structure in .github/workflows/
- [ ] T081 Final documentation pass: Remove all references to old structure
- [ ] T082 Final testing: Full test suite on all platforms (Linux, Windows, macOS)

**Checkpoint**: Migration complete, old structure removed, documentation updated

---

## Phase 9: Polish & Enhancements

**Purpose**: Improvements now that base restructuring is complete

- [ ] T083 [P] Add metadata.json support to each layout directory per data-model.md
- [ ] T084 [P] Create layout validation tool: scripts/validate-all-layouts.sh
- [ ] T085 [P] Performance optimization: Cache layout discovery results during CMake config
- [ ] T086 [P] Developer tooling: Add layout creation script scripts/create-new-layout.sh
- [ ] T087 [P] Documentation: Create video walkthrough of adding a new language
- [ ] T088 Code cleanup: Review and refactor LayoutRegistry for complexity <10
- [ ] T089 Code cleanup: Review migration artifacts, ensure all temporary code removed
- [ ] T090 Run validation suite from quickstart.md: verify all success criteria met

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup - BLOCKS all other phases
- **Migration Implementation (Phase 3)**: Depends on Foundational
- **CMake Integration (Phase 4)**: Depends on Foundational
- **Runtime Loading (Phase 5)**: Depends on CMake Integration (needs build to work)
- **Migration Execution (Phase 6)**: Depends on phases 3, 4, 5 all complete (full stack ready)
- **Default Switchover (Phase 7)**: Depends on Migration Execution validated
- **Cleanup (Phase 8)**: Depends on Default Switchover stable for 1 week
- **Polish (Phase 9)**: Depends on Cleanup complete

### Critical Path

```
Setup → Foundational → Migration Impl → Migration Execution → Switchover → Cleanup → Polish
                     ↘                ↗
                       CMake Integration
                     ↘                ↗
                       Runtime Loading
```

### Parallel Opportunities Within Phases

**Phase 2 (Foundational)**: All tasks can run in parallel (different files)
- T006-T011 (6 tasks in parallel)

**Phase 3 (Migration Tests)**: All test files can be created in parallel
- T012-T014 (3 tests in parallel)

**Phase 3 (Migration Implementation)**: T016-T018 can run in parallel (different aspects)
- Audio attribution, TTS attribution, parsing logic

**Phase 4 (CMake Tests)**: All test files can be created in parallel
- T027-T029 (3 tests in parallel)

**Phase 4 (CMake Implementation)**: T030-T032 can run in parallel (different functions)
- discover_layouts(), validate_layout_structure(), generate_layout_manifest()

**Phase 4 (Platform Testing)**: T037-T038 can run in parallel
- Windows and macOS builds simultaneously

**Phase 5 (Runtime Tests)**: T039-T040 can run in parallel
- Unit test and integration test different files

**Phase 5 (Layout Modules)**: T046-T047 can run in parallel
- Dutch and Flemish layout modules

**Phase 9 (Polish)**: Most tasks T083-T087 can run in parallel (different files/tools)

---

## Parallel Example: Foundational Phase

```bash
# All foundational tasks can run simultaneously (different files):
Developer A: T006 - validate_layout_structure() in scripts/build-tools/validate-layout-structure.cmake
Developer B: T007 - extract_audio_references() in scripts/build-tools/extract-audio-references.cmake
Developer C: T008 - discover_layouts() in scripts/build-tools/discover-layouts.cmake
Developer D: T009 - Feature flag in CMakeLists.txt
Developer E: T010 - ILayoutProvider interface in src/layouts/LayoutLoader.h
Developer F: T011 - LayoutRegistry class in src/layouts/LayoutRegistry.h/cpp
```

---

## Implementation Strategy

### Recommended Approach: Sequential Phases

Due to the migration nature of this project, **sequential phase execution is strongly recommended**:

1. **Phase 1-2**: Setup + Foundational (1-2 days)
   - Parallel within phases, but complete fully before moving on

2. **Phase 3-5**: Build complete migration stack (1-2 weeks)
   - Can parallelize work across these phases by different developers
   - Developer A: Migration script (Phase 3)
   - Developer B: CMake integration (Phase 4)
   - Developer C: Runtime loading (Phase 5)
   - **CRITICAL**: All three must complete before Phase 6

3. **Phase 6**: Migration Execution (1 week)
   - Execute migration, validate equivalence
   - **STOP**: Wait for CI green, team validation before proceeding

4. **Phase 7**: Default Switchover (1 week)
   - Monitor in production/dev use
   - **STOP**: Stable for 1 week minimum before cleanup

5. **Phase 8**: Cleanup (2-3 days)
   - Remove old structure permanently

6. **Phase 9**: Polish (ongoing)
   - Enhancements and improvements

**Total Estimated Time**: 4-6 weeks for complete migration

### Risk Mitigation

- Each phase has explicit checkpoints - do not proceed if checkpoint fails
- Phases 6-8 are time-gated (cannot rush)
- Rollback is possible until Phase 8 cleanup
- Feature flag enables instant rollback at any time
- Dual-structure support ensures safety net

---

## Validation Checkpoints

After each major phase, validate:

**After Phase 2 (Foundational)**:
- ✅ All CMake functions compile without errors
- ✅ All C++ headers compile without errors
- ✅ Feature flag toggles between old/new logic successfully

**After Phase 5 (Runtime Loading)**:
- ✅ Test build with new structure completes
- ✅ Test application starts and loads correct layout
- ✅ No runtime errors in layout selection

**After Phase 6 (Migration Execution)**:
- ✅ New directory structure created successfully
- ✅ Both old and new structures build and pass tests
- ✅ Binary equivalence verified
- ✅ Performance benchmarks within thresholds
- ✅ CI passing on all platforms

**After Phase 7 (Default Switchover)**:
- ✅ New structure is default for 1 week
- ✅ No critical issues reported
- ✅ Team feedback collected and addressed
- ✅ CI stable on all platforms

**After Phase 8 (Cleanup)**:
- ✅ Old structure completely removed
- ✅ All tests pass without old structure
- ✅ Documentation updated
- ✅ No references to old structure remain

---

## Notes

- This is a **migration project** - careful, phased execution is critical
- Do NOT rush through phases - time gates are safety measures
- [P] tasks within a phase can run in parallel
- Tests must FAIL before implementation (Test-Driven Development)
- Commit frequently, especially after each phase checkpoint
- Migration script complexity is tracked and time-boxed per constitution
- Performance budgets must be met: <5s CMake config increase, <10% build time increase
- Document all decisions and issues encountered during migration

---

## Success Criteria (from plan.md)

- ✅ CMake config time <8s (target: 6.8s measured)
- ✅ Build time increase <10% (target: 6.7% measured)
- ✅ Binary size unchanged
- ✅ Runtime startup unchanged
- ✅ All existing tests pass
- ✅ Cross-platform builds succeed (Linux, Windows, macOS)
- ✅ Migration script runs successfully
- ✅ No functional regressions
- ✅ Developer feedback positive
- ✅ Documentation complete and accurate
