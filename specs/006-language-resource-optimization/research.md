# Research: Layout-Based Resource Organization

**Feature**: 006-language-resource-optimization (Phase 2)  
**Date**: 2025-11-27  
**Status**: Complete

## Research Questions

### Q1: How to split Keys.cpp into per-layout modules while maintaining runtime compatibility?

**Decision**: Use a hybrid approach with thin dispatcher + dynamically discovered layout modules

**Rationale**:
- Current `Keys.cpp` is 796 lines with all layouts in preprocessor blocks
- Splitting requires maintaining the same `KeyTranslationEntry` interface
- Runtime must select correct layout based on LANGUAGE at compile time
- Zero performance overhead requirement (no runtime overhead vs current approach)

**Alternatives Considered**:

1. **Full Dynamic Loading (REJECTED)**
   - Load layout .so/.dll at runtime based on config
   - **Why rejected**: Adds runtime overhead, complicates deployment, requires dlopen/LoadLibrary platform-specific code

2. **CMake Code Generation (REJECTED)**  
   - Generate single Keys.cpp from templates at build time
   - **Why rejected**: Obscures source code, debugging harder, doesn't solve merge conflict problem

3. **Header-Only Layout Definitions (REJECTED)**
   - Each layout as constexpr in header
   - **Why rejected**: Increases compile time, all layouts compiled into every translation unit

4. **Hybrid Dispatcher (SELECTED)**
   - Thin `Keys.cpp` dispatcher with extern declarations
   - Each layout compiled as separate translation unit
   - Linker resolves correct layout based on compile-time LANGUAGE flag
   - **Advantages**: Clean separation, zero runtime overhead, maintains current interface, enables modular development

**Implementation Pattern**:
```cpp
// src/Keys.cpp (dispatcher)
#if defined __LANGUAGE_NL__
  extern const std::vector<KeyTranslationEntry> g_dutchDefault;
  extern const std::vector<KeyTranslationEntry> g_dutchClassic;
  // ...dispatch to appropriate layout
#elif defined __LANGUAGE_NL_BE__
  extern const std::vector<KeyTranslationEntry> g_flemishDefault;
  // ...
#endif

// res/layouts/classic/nl_nl/layout.cpp
const std::vector<KeyTranslationEntry> g_dutchClassic = { /* ... */ };
```

### Q2: How should CMake discover and build layout modules automatically?

**Decision**: CMake GLOB pattern with structure validation

**Rationale**:
- Must discover all `res/layouts/*/layout.cpp` files automatically
- Must validate each layout directory has required structure (layout.cpp, audio/, tts/)
- Must compile appropriate layouts based on LANGUAGE setting
- Must fail fast if structure invalid

**Best Practices for CMake Layout Discovery**:

1. **Use file(GLOB_RECURSE) with explicit patterns**
   ```cmake
   file(GLOB_RECURSE LAYOUT_SOURCES "res/layouts/*/*/layout.cpp")
   ```

2. **Validate structure before compilation**
   ```cmake
   foreach(LAYOUT_FILE ${LAYOUT_SOURCES})
     get_filename_component(LAYOUT_DIR ${LAYOUT_FILE} DIRECTORY)
     if(NOT EXISTS "${LAYOUT_DIR}/audio" OR NOT IS_DIRECTORY "${LAYOUT_DIR}/audio")
       message(FATAL_ERROR "Missing audio/ directory in ${LAYOUT_DIR}")
     endif()
   endforeach()
   ```

3. **Filter based on LANGUAGE setting**
   ```cmake
   # Only compile layouts matching LANGUAGE
   foreach(LAYOUT_FILE ${LAYOUT_SOURCES})
     if(LAYOUT_FILE MATCHES "/${LANGUAGE}/layout.cpp$")
       target_sources(Dyscover PRIVATE ${LAYOUT_FILE})
     endif()
   endforeach()
   ```

**Alternatives Considered**:

1. **Manual CMakeLists.txt per layout (REJECTED)**
   - Each layout has add_subdirectory() entry
   - **Why rejected**: Requires manual updates when adding languages, defeats automation goal

2. **JSON manifest file (REJECTED)**
   - layouts.json lists all available layouts
   - **Why rejected**: Introduces separate source of truth, can go out of sync with filesystem

3. **GLOB with validation (SELECTED)**
   - Automatic discovery via filesystem structure
   - Validation ensures correctness
   - **Advantages**: Self-documenting (structure IS the manifest), impossible to be out of sync

### Q3: How to handle shared audio files across layouts?

**Decision**: Symbolic links with CMake deduplication during install

**Rationale**:
- Some audio files (e.g., common letters) may be identical across languages
- Storage efficiency: don't duplicate 50+ identical files
- Build clarity: each layout directory shows what it needs
- Cross-platform: CMake handles symlinks vs copies automatically

**Pattern**:
```bash
# During migration
res/layouts/classic/nl_nl/audio/a.wav -> ../../../common/a.wav
res/layouts/classic/nl_be/audio/a.wav -> ../../../common/a.wav
res/common/a.wav  # Actual file
```

**CMake Install Logic**:
```cmake
# Resolve symlinks during install, install actual files once
install(FILES ${LAYOUT_AUDIO_FILES} 
        DESTINATION share/dyscover/audio
        FOLLOW_SYMLINK_CHAIN)
```

**Alternatives Considered**:

1. **Duplicate files (REJECTED)**
   - Copy shared files to each layout
   - **Why rejected**: Wastes 40-50% disk space, makes updates error-prone

2. **Shared directory only (REJECTED)**
   - All audio in res/common/, layouts reference by name
   - **Why rejected**: Obscures which files belong to which layout

3. **Symlinks + CMake deduplication (SELECTED)**
   - Best of both: local visibility + storage efficiency
   - CMake handles platform differences automatically

### Q4: Migration strategy for zero-downtime transition?

**Decision**: Feature flag with dual-structure support during transition

**Rationale**:
- Must not break existing builds during migration
- Must allow gradual testing before full switch
- Must enable rollback if issues found

**Migration Phases**:

**Phase 1: Parallel Structure (1-2 weeks)**
- Both old (src/Keys.cpp, res/data/) and new (res/layouts/) exist
- CMake flag: `USE_LAYOUT_STRUCTURE` (default OFF)
- CI builds both variants, compares binary outputs
- **Success criteria**: Binaries byte-identical or functionally equivalent

**Phase 2: Default Switchover (1 week)**
- Set `USE_LAYOUT_STRUCTURE` default to ON
- Old structure still present for rollback
- Monitor CI for any failures
- **Success criteria**: All tests pass, no performance regression

**Phase 3: Cleanup (1 week)**
- Remove old structure (src/Keys.cpp old code, res/data/ flat layout)
- Remove `USE_LAYOUT_STRUCTURE` flag
- Update documentation
- **Success criteria**: Builds succeed without old structure

**Rollback Plan**:
- Any phase: `cmake -DUSE_LAYOUT_STRUCTURE=OFF` reverts to old structure
- Git history preserves both versions until Phase 3 complete

**Alternatives Considered**:

1. **Big Bang Migration (REJECTED)**
   - Delete old, create new in single commit
   - **Why rejected**: High risk, no rollback, blocks all other work

2. **Branch Migration (REJECTED)**
   - Long-lived feature branch for restructure
   - **Why rejected**: Merge conflicts with ongoing development, delays integration

3. **Feature Flag with Dual Support (SELECTED)**
   - Enables gradual transition, continuous testing, easy rollback
   - Minimal disruption to ongoing development

### Q5: How to ensure cross-platform compatibility during restructuring?

**Decision**: Normalize path separators and test on all platforms in CI

**Rationale**:
- Windows uses backslashes, Unix uses forward slashes
- Symlinks not supported on Windows (use file copies instead)
- CMake abstracts most differences but validation needed

**Best Practices**:

1. **Use CMake native path handling**
   ```cmake
   file(TO_CMAKE_PATH "${LAYOUT_DIR}" LAYOUT_DIR_CMAKE)
   ```

2. **Platform-specific symlink handling**
   ```cmake
   if(WIN32)
     file(COPY_FILE ${SOURCE} ${DEST})
   else()
     file(CREATE_LINK ${SOURCE} ${DEST} SYMBOLIC)
   endif()
   ```

3. **CI matrix testing**
   ```yaml
   strategy:
     matrix:
       os: [ubuntu-latest, windows-latest, macos-latest]
       language: [nl, nl_be]
   ```

**References**:
- CMake file() documentation: https://cmake.org/cmake/help/latest/command/file.html
- Best practices for cross-platform CMake: https://github.com/onqtam/awesome-cmake

## Technology Stack Summary

| Component | Technology | Version | Rationale |
|-----------|-----------|---------|-----------|
| Build System | CMake | 3.20+ | Already in use, robust file operations |
| Layout Discovery | CMake GLOB | Built-in | Automatic, no external dependencies |
| Migration Script | Bash | 5.0+ | Portable, good for file operations |
| Validation | CMake Scripting | Built-in | Integrates with build, fails fast |
| Testing | CTest + Integration | Existing | Reuse current test infrastructure |
| Path Handling | CMake file() | Built-in | Cross-platform, handles symlinks |

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|---------|------------|
| Migration script corrupts files | Low | High | Test on copy of repo, version control, backup |
| Performance regression | Low | Medium | Benchmark before/after, <10% threshold |
| Cross-platform issues | Medium | Medium | CI matrix testing, early platform validation |
| Developer confusion | Medium | Low | Clear documentation, quickstart guide |
| Rollback complexity | Low | Medium | Feature flag enables instant rollback |

## References

- CMake Best Practices: https://cliutils.gitlab.io/modern-cmake/
- C++ Project Structure: https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs
- Modular C++ Design: "Large-Scale C++ Software Design" by John Lakos
- Current implementation: `/home/mpere/code/clevy/dyscover-clevy/src/Keys.cpp`, `/home/mpere/code/clevy/dyscover-clevy/cmake/ResourceManifest.cmake`
