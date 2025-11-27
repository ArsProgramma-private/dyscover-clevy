# Feature Specification: Language-Specific Resource Optimization

**Feature Branch**: `006-language-resource-optimization`  
**Created**: November 27, 2025  
**Status**: Draft  
**Input**: User description: "Optimize build to include only language-specific audio and TTS files based on selected language configuration"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Smaller Distribution Packages (Priority: P1)

Users receive application packages that contain only the audio and TTS files needed for their selected language, resulting in faster downloads and reduced storage requirements.

**Why this priority**: This is the primary value proposition - directly addresses the current inefficiency where all language resources are bundled regardless of the target language. Reduces bandwidth costs and improves user experience during installation.

**Independent Test**: Build the application for a specific language (e.g., Dutch) and verify that the output directory and installation package contain only Dutch audio files (res/data/*.wav for Dutch phonemes/sounds) and Dutch TTS data files (nl_nl.db, nl_nl.fsa, nl_nl.fst, Ilse.db, Ilse.fon, Ilse.opu, Ilse.udb), without any Flemish resources.

**Acceptance Scenarios**:

1. **Given** a build configured for Dutch (LANGUAGE=nl), **When** the build completes, **Then** the output directory contains only audio files referenced in Dutch keyboard layouts (g_dutchDefault, g_dutchClassic, g_dutchKWeC) and Dutch TTS files
2. **Given** a build configured for Flemish (LANGUAGE=nl_be), **When** the build completes, **Then** the output directory contains only audio files referenced in Flemish keyboard layouts (g_flemishDefault, g_flemishClassic) and Flemish TTS files
3. **Given** a build configured for Dutch, **When** examining the installation package, **Then** the package size is reduced by approximately 30-50% compared to the current all-inclusive package

---

### User Story 2 - Compile-Time Resource Validation (Priority: P2)

Developers receive compile-time errors if keyboard layout definitions reference audio files that don't exist for the selected language, preventing runtime issues.

**Why this priority**: Prevents bugs from reaching users by catching resource mismatches during build time rather than at runtime. Improves development workflow and code quality.

**Independent Test**: Modify a keyboard layout definition to reference a non-existent audio file for the selected language and verify that the build fails with a clear error message indicating which layout and which audio file is missing.

**Acceptance Scenarios**:

1. **Given** a keyboard layout entry referencing "xyz.wav" that doesn't exist, **When** the build is executed, **Then** the build fails with an error message: "Missing audio file 'xyz.wav' referenced in [layout name] for language [LANGUAGE]"
2. **Given** all keyboard layout entries reference valid audio files, **When** the build is executed, **Then** the build completes successfully
3. **Given** a TTS voice file is missing for the selected language, **When** the build is executed, **Then** the build fails with a clear error message identifying the missing TTS resource

---

### User Story 3 - Multi-Language Build Support (Priority: P3)

Build system supports generating packages for all supported languages without manual intervention, enabling automated release pipelines.

**Why this priority**: Enables CI/CD automation and reduces release overhead. While important for maintainability, it doesn't directly impact end-user experience, making it lower priority than P1 and P2.

**Independent Test**: Execute a build script that generates packages for all supported languages (nl, nl_be) and verify that each package contains only its language-specific resources and has appropriate naming to distinguish between language variants.

**Acceptance Scenarios**:

1. **Given** a multi-language build script, **When** executed, **Then** separate packages are generated for each language with distinct filenames (e.g., Dyscover-nl-4.0.5.0-win64.zip, Dyscover-nl_be-4.0.5.0-win64.zip)
2. **Given** multiple language packages, **When** examining each package, **Then** each contains only resources for its target language
3. **Given** a CI/CD pipeline configuration, **When** triggered, **Then** all language-specific packages are built in parallel without conflicts

---

### Edge Cases

- What happens when a keyboard layout references an audio file that exists in one language but not another? (Clarified: Build fails with descriptive error if required file missing)
- How does the build system handle shared audio files that are common across multiple languages? (Clarified: Include file if ANY layout for selected language references it)
- What validation occurs if Keys.cpp defines a layout for a language that isn't configured in CMakeLists.txt? (Clarified: Out of scope - assumes Keys.cpp layouts match LANGUAGE configuration)
- How are future keyboard layouts (for potential new languages) handled in the resource organization? (Clarified: Keys.cpp remains single file; build system extracts from existing structure)

## Clarifications

### Session 2025-11-27

- Q: Should Keys.cpp be restructured into separate layout files (e.g., `/classic/nl_nl.cpp`)? → A: Out of scope - keep Keys.cpp as single file; build system extracts requirements from existing structure
- Q: Should source audio files be reorganized into language subdirectories? → A: Keep flat `res/data/` structure - build system filters based on language requirements  
- Q: How should shared audio files (used by multiple languages) be handled? → A: Include audio file if ANY layout for the selected language references it
- Q: Should validation fail immediately or collect all errors? → A: Collect all missing file errors and report them together before failing
- Q: When should resource extraction happen? → A: Pre-build script runs automatically before compilation, supporting flexible language switching via CLI without CMake reconfiguration

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Build system MUST copy only audio files (.wav) that are referenced in ANY keyboard layout definition (KeyTranslationEntry::sound field) for the selected LANGUAGE to the output directory (e.g., if Dutch is selected, include files referenced in g_dutchDefault, g_dutchClassic, or g_dutchKWeC)
- **FR-002**: Build system MUST copy only TTS data files that correspond to the TTS_LANG and TTS_VOICE variables set for the selected LANGUAGE
- **FR-003**: Build system MUST validate at compile time that all audio files referenced in keyboard layout entries exist in the source directory
- **FR-004**: Build system MUST collect all missing audio file errors and report them together in a single comprehensive error message before failing (not fail-fast on first error)
- **FR-005**: Build configuration MUST generate a compile-time resource manifest that lists all required audio and TTS files for the selected language
- **FR-006**: Resource file organization MUST maintain the existing flat `res/data/` directory structure (no subdirectories by language)
- **FR-007**: Build system MUST use a pre-build script (executed automatically before compilation) to extract audio file requirements from Keys.cpp for the selected language, enabling language switching via CLI parameter without CMake reconfiguration
- **FR-008**: Installation/packaging rules MUST only include language-specific resources in the final distribution
- **FR-009**: Build system MUST support generating packages for multiple languages independently without cross-contamination

### Key Entities *(include if feature involves data)*

- **KeyboardLayout**: Represents a keyboard layout configuration (e.g., g_dutchDefault, g_flemishClassic) with language association and audio file references
- **AudioResource**: Individual WAV file with metadata about which languages/layouts reference it
- **TTSResource**: TTS data files (.db, .fsa, .fst, .fon, .opu, .udb) associated with specific language and voice configurations
- **LanguageConfiguration**: Build-time configuration specifying LANGUAGE, TTS_LANG, TTS_VOICE, and derived resource requirements
- **ResourceManifest**: Compile-time generated list of all required resources for a specific language build

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Dutch language build package size reduced by at least 40% compared to current all-inclusive package
- **SC-002**: Flemish language build package size reduced by at least 40% compared to current all-inclusive package
- **SC-003**: Build completes with zero runtime resource-loading failures when running end-to-end tests for each language
- **SC-004**: Build fails within 10 seconds with comprehensive error message listing ALL missing audio files (not just first error)
- **SC-005**: CI/CD pipeline generates correct language-specific packages for all supported languages with 100% success rate
- **SC-006**: Manual audit confirms that Dutch builds contain zero Flemish-specific resources and vice versa
- **SC-007**: Resource validation catches 100% of missing or incorrect audio file references at build time (validated through intentional error injection tests)

## Future Enhancements

### Phase 2: Layout-Based Resource Organization

**Context**: As more languages are added to the application, the current flat file structure (`res/data/*.wav`) and single-file layout definitions (`src/Keys.cpp`) may become difficult to maintain. A hierarchical organization based on keyboard layouts could improve scalability and maintainability.

**Proposed Structure**:

```
res/
├── layouts/
│   ├── classic/
│   │   ├── nl_nl/
│   │   │   ├── layout.cpp          # Layout definition
│   │   │   ├── audio/
│   │   │   │   ├── a.wav
│   │   │   │   ├── b.wav
│   │   │   │   └── ...
│   │   │   └── tts/
│   │   │       ├── nl_nl.db
│   │   │       └── ...
│   │   ├── nl_be/
│   │   │   ├── layout.cpp
│   │   │   ├── audio/
│   │   │   └── tts/
│   │   └── en_us/
│   │       ├── layout.cpp
│   │       ├── audio/
│   │       └── tts/
│   ├── modern/
│   │   ├── nl_nl/
│   │   └── nl_be/
│   └── accessible/
│       ├── nl_nl/
│       └── nl_be/
```

**Benefits**:

1. **Clearer Organization**: Each layout variant (classic, modern, accessible) has its own subdirectory structure
2. **Language Isolation**: All resources for a specific language/layout combination are co-located
3. **Easier Maintenance**: Adding a new language means creating a new directory, not modifying shared files
4. **Reduced Merge Conflicts**: Multiple developers can work on different languages simultaneously
5. **Simplified Build Logic**: Directory structure directly maps to build requirements

**Considerations**:

- **Migration Effort**: Requires restructuring existing codebase and updating build system
- **Shared Resources**: Some audio files may be common across layouts (need deduplication strategy)
- **Build Complexity**: May require more sophisticated CMake logic to traverse directory structure
- **Backward Compatibility**: Need migration path for existing configurations

**Decision Deferred**: This restructuring is not required for the current optimization goals (package size reduction). It should be considered when:

- More than 5 languages are supported
- Multiple layout variants per language are introduced
- Development team size increases and merge conflicts become frequent
- New keyboard layout types are added beyond current variants

**Related to**: FR-006 (which explicitly maintains current flat structure), Session clarification about keeping Keys.cpp as single file
