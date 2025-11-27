# Baseline: Current State Before Migration

**Feature**: 006-language-resource-optimization (Phase 2)  
**Date**: 2025-11-27  
**Purpose**: Document current state for comparison after migration

## File Structure (Before Migration)

### Source Files

```
src/
└── Keys.cpp                     # 835 lines (monolithic layout definitions)
```

**Layout Definitions in Keys.cpp**:
- `g_dutchDefault`: ~160 lines
- `g_flemishDefault`: ~160 lines
- `g_dutchClassic`: ~163 lines
- `g_flemishClassic`: ~163 lines
- `g_dutchKWeC`: ~160 lines
- Helper functions and includes: ~29 lines

### Resource Files

```
res/
└── data/
    ├── *.wav                    # 68 audio files
    └── tts/                     # 340 TTS files (all languages mixed)
```

**Directory Sizes**:
- `res/data/`: 426 MB total
- `res/data/tts/`: 422 MB (TTS data)
- Audio files: ~4 MB (68 .wav files)

### Build System

```
cmake/
└── ResourceManifest.cmake       # Current resource discovery

scripts/build-tools/
├── extract-audio-resources.cmake
└── validate-resources.cmake
```

## Metrics Summary

| Metric | Value | Notes |
|--------|-------|-------|
| **Code Organization** |
| Total source files | 1 | src/Keys.cpp (monolithic) |
| Lines of code (Keys.cpp) | 835 | All layouts in one file |
| Layout definitions | 5 | All in same file |
| **Resource Organization** |
| Total audio files | 68 | All in res/data/ (flat) |
| Total TTS files | 340 | All in res/data/tts/ (flat) |
| Total resource size | 426 MB | Includes all languages |
| **Build System** |
| CMake discovery logic | Manual | Explicit file lists in CMakeLists.txt |
| Validation | Basic | Resource existence checks only |
| Language isolation | None | All resources built regardless of LANGUAGE |

## Known Issues (Pre-Migration)

1. **Merge Conflicts**: Multiple developers editing Keys.cpp simultaneously causes frequent conflicts
2. **Language Isolation**: Cannot easily identify which audio files belong to which language/layout
3. **Scalability**: Adding new language requires manual CMakeLists.txt updates and careful Keys.cpp editing
4. **Build Efficiency**: All resources copied even if only building one language
5. **Maintainability**: Hard to find specific layout definition in 835-line file

## Expected Improvements (Post-Migration)

### File Structure

```
res/
└── layouts/
    ├── classic/
    │   ├── nl_nl/
    │   │   ├── layout.cpp       # ~163 lines (Dutch classic only)
    │   │   ├── audio/           # ~30 Dutch audio files
    │   │   └── tts/             # ~68 Dutch TTS files
    │   └── nl_be/
    │       ├── layout.cpp       # ~163 lines (Flemish classic only)
    │       ├── audio/           # ~32 Flemish audio files
    │       └── tts/             # ~68 Flemish TTS files
    ├── default/
    │   ├── nl_nl/
    │   └── nl_be/
    └── kwec/
        └── nl_nl/

src/
├── Keys.cpp                     # Thin dispatcher (~50 lines)
└── layouts/
    ├── LayoutLoader.h
    └── LayoutRegistry.h/cpp
```

### Metrics Targets

| Metric | Current | Target | Improvement |
|--------|---------|--------|-------------|
| **Code Organization** |
| Source files per layout | N/A | 1 | Clear separation |
| Lines per layout file | N/A | ~163 | Focused, readable |
| Merge conflict risk | High | Low | Isolated files |
| **Resource Organization** |
| Directory depth | 1 (flat) | 4 (hierarchical) | Clear structure |
| Language isolation | None | Complete | Easy identification |
| Shared resource handling | N/A | Symlinks | No duplication |
| **Build System** |
| CMake discovery | Manual | Automatic (GLOB) | No CMake edits |
| Validation level | Basic | Comprehensive | Catch errors early |
| Language-specific builds | No | Yes | Faster builds |

## Baseline Measurements (for comparison)

Run on: 2025-11-27  
Hardware: [TBD - record actual hardware]  
Branch: 006-language-resource-optimization (before migration)

### Build Performance

| Metric | Value |
|--------|-------|
| CMake configuration time | [TBD - measure] |
| Full build time (clean) | [TBD - measure] |
| Incremental build time | [TBD - measure] |
| Binary size | [TBD - measure] |
| Resource bundle size | [TBD - measure] |

### Runtime Performance

| Metric | Value |
|--------|-------|
| Application startup time | [TBD - measure] |
| Layout loading time | [TBD - measure] |
| Memory footprint | [TBD - measure] |

**Note**: These measurements should be taken just before executing migration to establish accurate baseline for comparison after migration completes.

## Validation Checklist

Before proceeding with migration, verify:

- [x] Current structure documented
- [x] File counts recorded
- [x] Directory sizes recorded
- [ ] Performance benchmarks measured (TODO before migration execution)
- [ ] Current git branch clean (TODO before migration execution)
- [ ] Backup branch created (TODO before migration execution)

## Migration Plan Reference

See: [quickstart.md](./quickstart.md) for detailed 3-phase migration plan

**Next Steps**:
1. Complete Phase 1 (Setup) tasks
2. Implement Phase 2 (Foundational) infrastructure
3. Run migration script with --dry-run
4. Measure performance benchmarks
5. Execute migration
6. Compare post-migration metrics to this baseline
