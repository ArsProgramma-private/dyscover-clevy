# Developer Guide: Language-Specific & Layout-Based Resources

**Feature**: 006-language-resource-optimization (Phase 2 complete)  
**Status**: Layout structure (legacy structure removed)  
**Last Updated**: November 27, 2025

## Overview

The project uses a hierarchical, layout-based resource organization under `res/layouts/`. Each layout is a modular unit containing its own layout definition, audio files, and TTS data.

## File Structure

```
res/
└── layouts/
    ├── classic/
    │   ├── nl_nl/
    │   │   ├── layout.cpp
    │   │   ├── audio/ (*.wav referenced by layout)
    │   │   └── tts/ (voice + language TTS data)
    │   └── nl_be/
    ├── default/
    │   ├── nl_nl/
    │   └── nl_be/
    └── kwec/
        └── nl_nl/
```

Supporting build logic:
```
cmake/LayoutDiscovery.cmake      # discover_layouts(), validate_layout_structure()
src/layouts/LayoutRegistry.*     # runtime registration & lookup
```

## How Discovery Works
1. During CMake configure, `discover_layouts()` scans `res/layouts/*/*/<LANGUAGE>/layout.cpp`
2. Each file is validated (layout.cpp present, audio/ & tts/ directories exist)
3. Valid sources are added to the target; each layout module registers itself in `LayoutRegistry` via a static block
4. Runtime selection uses compile-time `LANGUAGE` to choose the active layout set
2. Extract audio references → manifest file.
3. Validate existence in `res/data/`.
4. Package only manifest & relevant TTS files.

## Adding a New Layout (Default Mode)

1. Create directories (example: new layout type `accessible` for Dutch):
```bash
mkdir -p res/layouts/accessible/nl_nl/audio
mkdir -p res/layouts/accessible/nl_nl/tts
```
2. Add `layout.cpp`:
```cpp
// res/layouts/accessible/nl_nl/layout.cpp
#include "LayoutLoader.h"
namespace {
class DutchAccessibleLayout : public Dyscover::ILayoutProvider {
public:
    const std::vector<KeyTranslationEntry>& GetEntries() const override { return g_entries; }
    const char* GetName() const override { return "dutch_accessible"; }
    const char* GetLanguage() const override { return "nl_nl"; }
private:
    static const std::vector<KeyTranslationEntry> g_entries;
};
const std::vector<KeyTranslationEntry> DutchAccessibleLayout::g_entries = {
    { Key::A, false, false, false, { { Key::A } }, "a.wav" },
    // ...
};
static DutchAccessibleLayout s_layout;
static bool s_reg = [](){ Dyscover::LayoutRegistry::Instance().Register("dutch_accessible", &s_layout); return true; }();
} // namespace
```
3. Place referenced audio files under `audio/` and TTS data under `tts/`.
4. Reconfigure CMake: it auto‑discovers the new layout.

## Adding a New Language (Both Modes)

1. Add language constants to `CMakeLists.txt` (LANGUAGE selector block).  
2. In layout mode: create directories for each layout type needed.  
3. Provide `layout.cpp` modules (see example above).  
4. Place language & voice TTS data in each layout's `tts/` (or shared strategy—future enhancement).  
5. Configure: `cmake -B build-new -DLANGUAGE=en_us .` → discovery includes new layouts.
6. (Legacy fallback) If still using Keys.cpp: add preprocessor block & entries, update extraction script mapping.

## Switching Modes
```bash
cmake -B build .
cmake --build build
```

## Validation
```bash
cmake -B build -DLANGUAGE=nl_nl .
cmake --build build
./build/Dyscover --version
```
Check compile warnings in generated layout modules (expected if some initializer fields are intentionally omitted). Ensure registration count matches expected number of layouts.

## Performance Notes
| Stage | Time | Notes |
|-------|------|-------|
| Configure | ~2.3–2.6s | Layout discovery overhead minimal |
| Build (clean) | Baseline + <10% | Within target thresholds |
| Incremental | Unchanged | Fast rebuilds maintained |

## Troubleshooting
| Symptom | Likely Cause | Action |
|---------|--------------|--------|
| Layout not discovered | Directory depth wrong or language code mismatch | Verify path: `res/layouts/<type>/<lang>/layout.cpp` |
| Runtime missing layout | Registration static block omitted | Add static instance + lambda registration |
| Duplicate audio copies | Shared file placed in multiple layout/audio dirs | Consider shared parent + symlink (future enhancement) |

## Key Files
| File | Purpose |
|------|---------|
| `cmake/LayoutDiscovery.cmake` | Discovery & validation functions |
| `src/layouts/LayoutRegistry.*` | Central registry / active layout selection |
| `res/layouts/*/*/layout.cpp` | Individual layout providers |
| `CMakeLists.txt` | Language selection & build configuration |

## Accessibility & Inclusive Design
Layout modules should preserve consistent naming and avoid ambiguous labels. Review added audio file names for clarity (avoid cryptic codes).  
Generated code was produced with accessibility in mind; please manually test with assistive tools.

---
Questions? See `specs/006-language-resource-optimization/spec.md` for original spec and `quickstart.md` for historical migration steps.
