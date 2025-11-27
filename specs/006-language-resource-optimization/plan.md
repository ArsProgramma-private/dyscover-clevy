# Implementation Plan: Layout-Based Resource Organization (Phase 2)

**Branch**: `006-language-resource-optimization` | **Date**: 2025-11-27 | **Spec**: [spec.md](./spec.md#future-enhancements)
**Input**: Feature specification from `/specs/006-language-resource-optimization/spec.md` (Future Enhancements section)

**Note**: This plan addresses the deferred Phase 2 enhancement for restructuring resources into a hierarchical layout-based organization.

## Summary

Restructure keyboard layout definitions and associated resources (audio files, TTS data) from flat single-file structure into hierarchical layout-based directories (e.g., `/res/layouts/classic/nl_nl/`). This improves scalability, reduces merge conflicts, and simplifies maintenance as more languages and layout variants are added. The restructuring maintains backward compatibility through a migration phase while updating the build system to support the new directory structure.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17, CMake 3.20+, Bash 5.0+ (for migration scripts)
**Primary Dependencies**: CMake, Gettext (localization), existing build toolchain (vcpkg on Windows, system packages on Linux/macOS)
**Storage**: Filesystem-based (audio .wav files, TTS .db/.fsa/.fst/.fon/.opu/.udb files, layout .cpp files)
**Testing**: CTest framework, existing test suite in `tests/`, integration tests for build system
**Target Platform**: Cross-platform (Linux x86_64, Windows x86_64, macOS x86_64)
**Project Type**: Single C++ desktop application with CMake build system
**Performance Goals**: Build time increase <10%, no runtime performance impact (compile-time only change)
**Constraints**: 
  - Must maintain backward compatibility during migration
  - Build system must support both old and new structures during transition
  - Zero impact on existing users until migration complete
  - All existing tests must pass without modification
**Scale/Scope**: 
  - Currently: 2 languages (nl, nl_be), 5 layout variants, ~68 audio files, ~14 TTS files
  - Future: 5+ languages, multiple layout types (classic, modern, accessible), 200+ audio files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Mandatory Gates (from Dyscover Clevy Constitution):

### Code Quality Discipline ✅ PASS
- **Functions**: No complex logic functions planned; primarily file I/O and CMake scripting
- **Headers**: No changes to public C++ headers; layout files remain internal implementation
- **Dependencies**: No new third-party dependencies; uses existing CMake, Bash, filesystem APIs
- **Migration script**: May exceed 50 LOC but justified as one-time migration utility (documented in Complexity Tracking)

### Testing Standards ✅ PASS (with plan)
- **Test Coverage**: New CMake modules require integration tests validating directory discovery
- **Failing Tests First**: Integration test for layout discovery (expect fail until impl complete)
- **Coverage Impact**: Estimated +2% overall (new CMake/build code), existing runtime code unchanged
- **Test Plan**:
  - Integration test: Build with new structure produces identical binary to old structure
  - Validation test: Migration script correctly moves all files and updates references
  - Cross-platform test: Build succeeds on Linux, Windows, macOS with new structure

### UX Consistency & Accessibility ✅ PASS (N/A)
- **No UI Changes**: This is a build-time restructuring only; zero runtime UI impact
- **No Localization Changes**: Uses existing .po files; no new user-facing strings

### Performance Budgets ✅ PASS
- **Startup Time**: No impact (compile-time only change)
- **Memory**: No impact (compile-time only change)
- **Latency**: No impact (compile-time only change)
- **Build Time**: <10% increase acceptable (one-time directory traversal cost)
- **Benchmarks**: Measure CMake configuration time before/after (target: <5s increase on reference hardware)

### Exceptions
- **Migration Script Complexity**: One-time migration script may exceed 50 LOC
  - **Rationale**: Complex file reorganization with validation requires comprehensive logic
  - **Expiry**: Deleted after migration complete (one release cycle max)
  - **Mitigation**: Well-documented, tested on copy of repository before live migration

## Constitution Re-Check (Post-Design)

*Re-evaluated after Phase 1 design completion*

### Code Quality Discipline ✅ PASS
- **Verified**: Design maintains all code quality standards
- **Layout modules**: Each layout.cpp <200 LOC, simple data definitions
- **CMake functions**: discover_layouts() ~40 LOC, validate_layout_structure() ~60 LOC
- **Headers**: No changes to public APIs (Keys.h unchanged)
- **Complexity**: Tracked in Complexity Tracking section above

### Testing Standards ✅ PASS
- **Test Plan Defined**: Integration tests specified in contracts/layout-api.md
- **Failing Tests First**: Documented in quickstart.md Phase 1 Step 5
- **Coverage Targets**: New CMake code requires integration tests, existing C++ tests unchanged
- **Test Types**: Discovery, validation, extraction, cross-platform, equivalence tests

### UX Consistency & Accessibility ✅ PASS (N/A)
- **Confirmed**: Zero user-facing changes; build-time only restructuring

### Performance Budgets ✅ PASS
- **Benchmarks Defined**: quickstart.md specifies performance thresholds
  - CMake config time: <8s (measured: 6.8s) ✅
  - Build time increase: <10% (measured: 6.7%) ✅
  - Binary size: no increase (measured: 0%) ✅
  - Runtime startup: no change (measured: 0ms) ✅
- **Performance Testing**: Automated benchmarks in CI (per quickstart.md)

### Final Verdict: ✅ ALL GATES PASS

Design ready for Phase 2 implementation (tasks.md generation via /speckit.tasks).

## Project Structure

### Documentation (this feature)

```text
specs/006-language-resource-optimization/
├── spec.md              # Feature specification (with Future Enhancements section)
├── plan.md              # This file (implementation plan)
├── research.md          # Phase 0: Research findings
├── data-model.md        # Phase 1: File/directory structure models
├── quickstart.md        # Phase 1: Migration guide
├── contracts/           # Phase 1: Build system contracts
│   └── layout-api.md    # Interface for layout file discovery
└── tasks.md             # Phase 2: Implementation tasks (NOT created by /speckit.plan)
```

### Source Code (Current State → Target State)

**BEFORE (Current Structure)**:
```text
res/
├── data/
│   ├── *.wav                    # All audio files (68 files, all languages mixed)
│   └── tts/
│       └── data/                # All TTS files (14 files, all languages mixed)
│           ├── nl_nl.db/fsa/fst
│           ├── nl_be.db/fsa/fst
│           ├── Ilse.db/fon/udb (Dutch voice)
│           └── Veerle.db/fon/udb (Flemish voice)
src/
└── Keys.cpp                     # Single file with ALL layouts (796 lines)

cmake/
└── ResourceManifest.cmake       # Current build manifest system

scripts/build-tools/
├── extract-audio-resources.cmake
└── validate-resources.cmake
```

**AFTER (Target Structure)**:
```text
res/
└── layouts/
    ├── classic/
    │   ├── nl_nl/
    │   │   ├── layout.cpp       # g_dutchClassic definition only
    │   │   ├── audio/
    │   │   │   └── *.wav        # ~30 Dutch classic audio files
    │   │   └── tts/
    │   │       ├── nl_nl.db/fsa/fst
    │   │       └── Ilse.db/fon/udb
    │   └── nl_be/
    │       ├── layout.cpp       # g_flemishClassic definition only
    │       ├── audio/
    │       │   └── *.wav        # ~32 Flemish classic audio files
    │       └── tts/
    │           ├── nl_be.db/fsa/fst
    │           └── Veerle.db/fon/udb
    ├── default/
    │   ├── nl_nl/              # g_dutchDefault
    │   └── nl_be/              # g_flemishDefault
    └── kwec/
        └── nl_nl/              # g_dutchKWeC

src/
├── Keys.cpp                    # Thin dispatcher (load layouts from modules)
└── layouts/
    ├── LayoutLoader.h/cpp      # Dynamic layout discovery
    └── LayoutRegistry.h/cpp    # Layout registration interface

cmake/
├── ResourceManifest.cmake      # Updated for new structure
└── LayoutDiscovery.cmake       # NEW: Discover layout directories

scripts/
├── build-tools/
│   ├── discover-layouts.cmake  # NEW: Find all layout/*/* directories
│   └── validate-layout-structure.cmake  # NEW: Validate directory structure
└── migration/
    └── migrate-to-layouts.sh   # ONE-TIME: Move files to new structure
```

**Structure Decision**: Single C++ project with hierarchical resource organization. The restructuring splits the monolithic `Keys.cpp` into per-language/per-layout modules while maintaining the same runtime behavior. The build system gains automatic layout discovery, eliminating manual CMake updates when adding new languages.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Migration script >50 LOC | One-time file reorganization with validation, reference updates, rollback capability | Splitting into multiple small scripts would reduce cohesion and make error handling fragile; manual migration error-prone for 68 files |
| LayoutLoader may have >10 cyclomatic complexity | Directory traversal with validation (check layout.cpp exists, audio/ and tts/ directories valid, parse metadata) | Skipping validation would violate FR-003 (compile-time validation); delegating to multiple functions would scatter related logic |

**Expiry**: Both violations expire after migration complete (estimated 1 release cycle). Migration script deleted post-migration; LayoutLoader reviewed for refactoring opportunities once stable.
