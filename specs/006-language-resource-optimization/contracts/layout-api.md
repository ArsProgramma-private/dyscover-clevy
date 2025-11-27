# Contract: Layout Discovery and Loading API

**Feature**: 006-language-resource-optimization (Phase 2)  
**Date**: 2025-11-27  
**Version**: 1.0

## Overview

This contract defines the interface between the CMake build system and the C++ runtime for discovering, validating, and loading keyboard layout modules.

## Build-Time Contract (CMake)

### Function: `discover_layouts()`

**Purpose**: Find all layout directories matching the selected LANGUAGE

**Signature**:
```cmake
discover_layouts(
  OUTPUT_VAR         # Variable name to store results
  LANGUAGE           # Language code (e.g., "nl", "nl_be")
  LAYOUT_ROOT        # Root directory (default: ${CMAKE_SOURCE_DIR}/res/layouts)
)
```

**Returns**:
```cmake
# OUTPUT_VAR contains list of absolute paths to layout.cpp files
set(DISCOVERED_LAYOUTS
  "/abs/path/res/layouts/classic/nl_nl/layout.cpp"
  "/abs/path/res/layouts/default/nl_nl/layout.cpp"
  "/abs/path/res/layouts/kwec/nl_nl/layout.cpp"
)
```

**Behavior**:
1. Scan `${LAYOUT_ROOT}/*/*/${LANGUAGE}/layout.cpp`
2. Validate each found directory (see `validate_layout_structure()`)
3. Return list of valid layout.cpp paths
4. Log warning for invalid layouts (don't fail)

**Example**:
```cmake
discover_layouts(LAYOUT_SOURCES LANGUAGE nl)
foreach(LAYOUT ${LAYOUT_SOURCES})
  target_sources(Dyscover PRIVATE ${LAYOUT})
endforeach()
```

---

### Function: `validate_layout_structure()`

**Purpose**: Verify a layout directory has required structure

**Signature**:
```cmake
validate_layout_structure(
  LAYOUT_DIR         # Path to layout directory
  RESULT_VAR         # Variable to store validation result (TRUE/FALSE)
  ERROR_VAR          # Variable to store error message (if invalid)
)
```

**Returns**:
```cmake
# On success
set(RESULT_VAR TRUE)
set(ERROR_VAR "")

# On failure
set(RESULT_VAR FALSE)
set(ERROR_VAR "Missing audio/ directory in /path/to/layout")
```

**Validation Checks**:
1. `layout.cpp` exists and is readable
2. `audio/` directory exists
3. `audio/` contains at least one .wav file
4. `tts/` directory exists
5. `tts/` contains required language files: `{language}.db`, `{language}.fsa`, `{language}.fst`
6. `tts/` contains required voice files: `{voice}.db`, `{voice}.fon`, `{voice}.udb` (voice name from CMake var)

**Example**:
```cmake
validate_layout_structure(
  "${CMAKE_SOURCE_DIR}/res/layouts/classic/nl_nl"
  VALID
  ERROR_MSG
)
if(NOT VALID)
  message(WARNING "Invalid layout: ${ERROR_MSG}")
endif()
```

---

### Function: `extract_audio_references()`

**Purpose**: Parse layout.cpp to extract all audio file references

**Signature**:
```cmake
extract_audio_references(
  LAYOUT_FILE        # Path to layout.cpp
  OUTPUT_VAR         # Variable to store list of audio filenames
)
```

**Returns**:
```cmake
# OUTPUT_VAR contains list of audio filenames (not full paths)
set(AUDIO_FILES
  "a.wav"
  "b.wav"
  "cijfer-0.wav"
  # ...
)
```

**Parsing Rules**:
1. Match regex: `"([^"]+\.wav)"`
2. Extract unique filenames
3. Sort alphabetically
4. Return deduplicated list

**Example**:
```cmake
extract_audio_references(
  "${CMAKE_SOURCE_DIR}/res/layouts/classic/nl_nl/layout.cpp"
  REQUIRED_AUDIO
)
foreach(AUDIO ${REQUIRED_AUDIO})
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/res/layouts/classic/nl_nl/audio/${AUDIO}")
    message(FATAL_ERROR "Missing audio file: ${AUDIO}")
  endif()
endforeach()
```

---

### Function: `generate_layout_manifest()`

**Purpose**: Create install manifest for a specific layout

**Signature**:
```cmake
generate_layout_manifest(
  LAYOUT_DIR         # Path to layout directory
  LANGUAGE           # Language code
  LAYOUT_TYPE        # Layout type (classic/default/kwec)
  OUTPUT_FILE        # Path to output manifest file
)
```

**Output Format** (text file, one line per resource):
```
layout.cpp
audio/a.wav
audio/b.wav
tts/nl_nl.db
tts/nl_nl.fsa
tts/nl_nl.fst
tts/Ilse.db
tts/Ilse.fon
tts/Ilse.udb
```

**Example**:
```cmake
generate_layout_manifest(
  "${CMAKE_SOURCE_DIR}/res/layouts/classic/nl_nl"
  "nl"
  "classic"
  "${CMAKE_BINARY_DIR}/manifests/classic-nl.txt"
)
```

## Runtime Contract (C++)

### Interface: `LayoutLoader`

**Purpose**: Abstract interface for loading keyboard layouts at compile time

**Header**: `src/layouts/LayoutLoader.h`

```cpp
#pragma once
#include <vector>
#include "Keys.h"

namespace Dyscover {

// Abstract interface for layout providers
class ILayoutProvider {
public:
    virtual ~ILayoutProvider() = default;
    virtual const std::vector<KeyTranslationEntry>& GetEntries() const = 0;
    virtual const char* GetName() const = 0;
    virtual const char* GetLanguage() const = 0;
};

// Layout registry for compile-time layout selection
class LayoutRegistry {
public:
    static LayoutRegistry& Instance();
    
    // Register a layout (called from layout modules' static initializers)
    void Register(const char* name, ILayoutProvider* provider);
    
    // Get layout by name (compile-time selected via preprocessor)
    const ILayoutProvider* GetLayout(const char* name) const;
    
    // Get current active layout (set at compile time)
    const ILayoutProvider* GetActiveLayout() const;
    
private:
    LayoutRegistry() = default;
    std::map<std::string, ILayoutProvider*> layouts_;
};

} // namespace Dyscover
```

**Usage Pattern**:
```cpp
// In each layout module (e.g., res/layouts/classic/nl_nl/layout.cpp)
namespace {
class DutchClassicLayout : public Dyscover::ILayoutProvider {
public:
    const std::vector<KeyTranslationEntry>& GetEntries() const override {
        return g_dutchClassic;
    }
    const char* GetName() const override { return "dutch_classic"; }
    const char* GetLanguage() const override { return "nl"; }
    
private:
    static const std::vector<KeyTranslationEntry> g_dutchClassic;
};

const std::vector<KeyTranslationEntry> DutchClassicLayout::g_dutchClassic = {
    { Key::A, false, false, false, { { Key::A } }, "a.wav" },
    // ...
};

// Static registration
static DutchClassicLayout s_layout;
static bool s_registered = []() {
    Dyscover::LayoutRegistry::Instance().Register("dutch_classic", &s_layout);
    return true;
}();
} // anonymous namespace
```

## Error Handling Contract

### Build-Time Errors

**FATAL_ERROR**: Build must stop
- Missing required layout directory
- Invalid layout.cpp syntax (compilation error)
- Missing audio file referenced in layout.cpp
- Incomplete TTS files

**WARNING**: Build continues
- Layout directory exists but is invalid (skipped)
- Extra files in audio/ or tts/ directories
- Symlink resolution failures (fall back to copy)

### Runtime Errors

**Should NOT occur** if build-time validation passes:
- All layouts available at compile time
- No dynamic loading failures
- No missing resource files

## Backward Compatibility Contract

During migration (USE_LAYOUT_STRUCTURE feature flag):

### When `USE_LAYOUT_STRUCTURE=OFF` (default during Phase 1-2):
- CMake uses old discovery: parse `src/Keys.cpp`
- Resources copied from `res/data/`
- New `res/layouts/` structure ignored

### When `USE_LAYOUT_STRUCTURE=ON` (Phase 2-3):
- CMake uses new discovery: `discover_layouts()`
- Resources copied from `res/layouts/*/
*/`
- Old `src/Keys.cpp` and `res/data/` ignored

### Equivalence Guarantee:
- Binary output MUST be functionally identical regardless of flag setting
- Same KeyTranslationEntry data
- Same audio files installed
- Same TTS files installed

## Testing Contract

### Required Build-Time Tests

1. **Discovery Test**: Verify `discover_layouts()` finds all expected layouts
2. **Validation Test**: Verify `validate_layout_structure()` catches invalid layouts
3. **Extraction Test**: Verify `extract_audio_references()` finds all .wav references
4. **Cross-Language Test**: Verify only selected LANGUAGE's layouts are compiled

### Required Integration Tests

1. **Equivalence Test**: Build with old and new structure, verify binary equivalence
2. **Cross-Platform Test**: Build on Linux, Windows, macOS with new structure
3. **Multi-Language Test**: Build all languages, verify no cross-contamination

### Performance Tests

1. **CMake Configuration Time**: <5s increase with new structure (vs old)
2. **Build Time**: <10% increase with new structure
3. **Binary Size**: No increase (same layouts, just different organization)

## Versioning and Migration

**Contract Version**: 1.0  
**Introduced**: Phase 2 (deferred enhancement)  
**Deprecated**: N/A (new contract)  
**Removed**: N/A

**Breaking Changes** (from old to new):
- `src/Keys.cpp` no longer monolithic (split into modules)
- `res/data/` no longer flat (hierarchical structure)
- CMake discovery logic completely rewritten

**Non-Breaking**:
- `KeyTranslationEntry` struct unchanged
- `FindTranslation()` function signature unchanged
- Runtime behavior identical

## Acceptance Criteria

This contract is fulfilled when:

1. ✅ All CMake functions implemented and documented
2. ✅ All C++ interfaces implemented and documented
3. ✅ Build-time validation catches all specified error conditions
4. ✅ Integration tests pass on all platforms (Linux, Windows, macOS)
5. ✅ Performance benchmarks meet thresholds (<5s config, <10% build)
6. ✅ Backward compatibility maintained during migration phases
7. ✅ Binary equivalence verified between old and new structures
