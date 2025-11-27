# Data Model: Layout-Based Resource Organization

**Feature**: 006-language-resource-optimization (Phase 2)  
**Date**: 2025-11-27

## Overview

This document defines the filesystem entities and their relationships for the hierarchical layout-based resource organization.

## Entities

### 1. LayoutDirectory

Represents a single keyboard layout with its associated resources.

**Location**: `res/layouts/{layout_type}/{language}/`

**Attributes**:
- `layout_type`: String (e.g., "classic", "default", "kwec")
- `language`: String (e.g., "nl_nl", "nl_be", "en_us")
- `layout_file`: Path to layout.cpp (contains KeyTranslationEntry definitions)
- `audio_dir`: Path to audio/ subdirectory
- `tts_dir`: Path to tts/ subdirectory

**Validation Rules**:
- MUST contain `layout.cpp` file
- MUST contain `audio/` directory
- MUST contain `tts/` directory
- `layout.cpp` MUST define at least one `std::vector<KeyTranslationEntry>`
- `audio/` MUST contain at least one .wav file
- `tts/` MUST contain language-specific TTS files (.db, .fsa, .fst, .fon, .udb)

**Example**:
```
res/layouts/classic/nl_nl/
├── layout.cpp           # Defines g_dutchClassic
├── audio/
│   ├── a.wav
│   ├── b.wav
│   └── ...
└── tts/
    ├── nl_nl.db
    ├── nl_nl.fsa
    ├── nl_nl.fst
    ├── Ilse.db
    ├── Ilse.fon
    └── Ilse.udb
```

### 2. LayoutType

Represents a category of keyboard layouts.

**Location**: `res/layouts/{layout_type}/`

**Known Types**:
- `classic`: Traditional layouts (g_dutchClassic, g_flemishClassic)
- `default`: Standard default layouts (g_dutchDefault, g_flemishDefault)
- `kwec`: Specialized KWeC layout (g_dutchKWeC)

**Attributes**:
- `name`: String (directory name)
- `supported_languages`: List of language subdirectories

**Relationships**:
- One LayoutType HAS MANY LayoutDirectory (one per supported language)

### 3. AudioResource

Represents a single audio file (.wav) used by a layout.

**Location**: `res/layouts/{layout_type}/{language}/audio/{filename}.wav`  
OR `res/common/audio/{filename}.wav` (for shared resources)

**Attributes**:
- `filename`: String (e.g., "a.wav", "b.wav")
- `is_shared`: Boolean (true if in common/, false if layout-specific)
- `referenced_by`: List of KeyTranslationEntry references

**Validation Rules**:
- MUST be valid WAV format
- MUST be referenced by at least one KeyTranslationEntry in layout.cpp
- If `is_shared`: MAY be symlinked from multiple layouts
- Size SHOULD be <100KB (warning if larger)

### 4. TTSResource

Represents TTS (Text-to-Speech) data files for a specific language/voice.

**Location**: `res/layouts/{layout_type}/{language}/tts/`

**File Types**:
- **Language files**: `{language}.db`, `{language}.fsa`, `{language}.fst` (e.g., nl_nl.db)
- **Voice files**: `{voice}.db`, `{voice}.fon`, `{voice}.udb` (e.g., Ilse.db)

**Attributes**:
- `language_code`: String (e.g., "nl_nl", "nl_be", "en_us")
- `voice_name`: String (e.g., "Ilse", "Veerle", "David")
- `language_files`: List of .db/.fsa/.fst files
- `voice_files`: List of .db/.fon/.udb files

**Validation Rules**:
- Language files: MUST have all three extensions (.db, .fsa, .fst)
- Voice files: MUST have all three extensions (.db, .fon, .udb)
- `language_code` MUST match parent directory name
- `voice_name` MUST match configured TTS_VOICE in CMakeLists.txt

### 5. LayoutModule

Represents the compiled C++ layout definition.

**Location**: `res/layouts/{layout_type}/{language}/layout.cpp`

**Attributes**:
- `source_file`: Path to .cpp file
- `layout_variables`: List of layout variable names (e.g., ["g_dutchClassic"])
- `audio_references`: List of audio filenames referenced in KeyTranslationEntry::sound fields
- `compile_flags`: List of preprocessor definitions (e.g., ["__LANGUAGE_NL__"])

**Validation Rules**:
- MUST be valid C++17 source code
- MUST define at least one `std::vector<KeyTranslationEntry>` variable
- All audio references in KeyTranslationEntry::sound MUST exist in audio/ directory
- MUST NOT include platform-specific code without guards

**Example Content**:
```cpp
#include "Keys.h"

const std::vector<KeyTranslationEntry> g_dutchClassic = {
    { Key::A, false, false, false, { { Key::A } }, "a.wav" },
    { Key::B, false, false, false, { { Key::B } }, "b.wav" },
    // ...
};
```

## Relationships

```
LayoutType (1) ──────┬─────── (*) LayoutDirectory
                     │
                     └─────── (*) Language

LayoutDirectory (1) ─┬─────── (1) LayoutModule
                     ├─────── (*) AudioResource
                     └─────── (1) TTSResource

AudioResource (*) ───────────── (*) KeyTranslationEntry (via filename reference)

TTSResource (1) ─────────────── (1) Language (via language_code)
```

## State Transitions

### LayoutDirectory Lifecycle

```
[Non-existent] 
    │
    ├─ migrate-to-layouts.sh → [Created]
    │
[Created]
    │
    ├─ CMake validation → [Valid] or [Invalid]
    │
[Valid]
    │
    ├─ CMake compile → [Compiled]
    │
[Compiled]
    │
    ├─ CMake install → [Installed]
```

**Invalid State Triggers**:
- Missing layout.cpp
- Missing audio/ or tts/ directory
- layout.cpp references non-existent audio file
- TTS files incomplete (missing .db/.fsa/.fst or .fon/.udb)

### Migration State Transitions

```
[Old Structure Only]  (src/Keys.cpp, res/data/)
    │
    ├─ Phase 1: Run migration script
    ↓
[Dual Structure]  (both old and new exist, USE_LAYOUT_STRUCTURE=OFF)
    │
    ├─ Phase 1: Validate equivalence
    ↓
[Dual Structure Validated]  (CI passes, binaries equivalent)
    │
    ├─ Phase 2: Set USE_LAYOUT_STRUCTURE=ON
    ↓
[New Structure Default]  (new structure used, old retained for rollback)
    │
    ├─ Phase 3: Remove old structure
    ↓
[New Structure Only]  (res/layouts/ only)
```

## Constraints

### Filesystem Constraints

- **Naming Convention**: 
  - Layout types: lowercase, alphanumeric, no spaces (e.g., "classic", "default", "kwec")
  - Languages: lowercase, underscore separator (e.g., "nl_nl", "nl_be", "en_us")
  - Audio files: lowercase, alphanumeric + underscores, .wav extension

- **Directory Depth**: Maximum 4 levels: `res/layouts/{type}/{lang}/{resource_type}/`

- **Symlinks**: 
  - Allowed for shared audio files
  - Must be relative (e.g., `../../../common/a.wav`)
  - Must resolve to existing file

### CMake Build Constraints

- **Discovery Pattern**: `res/layouts/*/*/layout.cpp` (fixed depth for performance)
- **Compilation Order**: Layout modules compiled before main application
- **Language Selection**: Only layouts matching `LANGUAGE` cmake variable are compiled
- **Include Paths**: Layout modules can include `src/Keys.h` (public interface)

### Cross-Platform Constraints

- **Windows**: Symlinks require admin privileges or Developer Mode; use file copies as fallback
- **Linux/macOS**: Symlinks supported natively
- **CMake Abstraction**: `file(CREATE_LINK)` with SYMBOLIC flag handles platform differences

## Examples

### Valid LayoutDirectory (Classic Dutch)

```
res/layouts/classic/nl_nl/
├── layout.cpp           # 163 lines, defines g_dutchClassic
├── audio/               # 30 .wav files
│   ├── a.wav
│   ├── b.wav
│   ├── cijfer-0.wav
│   └── ...
└── tts/                 # 6 files (3 language + 3 voice)
    ├── nl_nl.db         # 2.1 MB
    ├── nl_nl.fsa        # 847 KB
    ├── nl_nl.fst        # 1.3 MB
    ├── Ilse.db          # 52 KB
    ├── Ilse.fon         # 3.2 MB
    └── Ilse.udb         # 47 KB
```

### Invalid LayoutDirectory Examples

**Missing TTS files**:
```
res/layouts/classic/nl_nl/
├── layout.cpp
├── audio/
│   └── a.wav
└── tts/
    ├── nl_nl.db         # Missing .fsa and .fst
    └── Ilse.db          # Missing .fon and .udb
```
**Error**: "Incomplete TTS language files for nl_nl: missing nl_nl.fsa, nl_nl.fst"

**Audio reference mismatch**:
```
// layout.cpp
{ Key::A, false, false, false, { { Key::A } }, "missing.wav" }

// audio/ directory
audio/a.wav              # "missing.wav" doesn't exist
```
**Error**: "Missing audio file 'missing.wav' referenced in layout.cpp"

## Metadata Format

Each LayoutDirectory MAY contain optional `metadata.json`:

```json
{
  "layout_type": "classic",
  "language": "nl_nl",
  "language_name": "Dutch (Netherlands)",
  "layout_name": "Classic Dutch Keyboard",
  "maintainer": "Team Name",
  "last_updated": "2025-11-27",
  "audio_count": 30,
  "shared_audio_count": 15,
  "tts_voice": "Ilse",
  "notes": "Traditional layout used by most Dutch users"
}
```

**Purpose**: Documentation and tooling; not used by build system.

## Future Extensions

- **Multi-voice support**: Allow multiple voices per language in same layout
- **Audio format variants**: Support OGG/MP3 in addition to WAV
- **Layout inheritance**: Allow layouts to inherit from base layouts with overrides
- **Dynamic layout loading**: Runtime layout switching without rebuild
