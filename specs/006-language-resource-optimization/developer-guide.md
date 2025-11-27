# Developer Guide: Language-Specific & Layout-Based Resources

**Feature**: 006-language-resource-optimization (Phase 2 enhancement complete)  
**Status**: Layout structure DEFAULT (legacy manifest mode still available)  
**Last Updated**: November 27, 2025

## Overview

The original optimization (Phase 1) reduced package size by copying only language‑specific audio & TTS resources discovered from `Keys.cpp`. Phase 2 adds a hierarchical, layout‑based resource organization under `res/layouts/` and makes it the default build path (`USE_LAYOUT_STRUCTURE=ON`). The legacy flat manifest mode remains available via `-DUSE_LAYOUT_STRUCTURE=OFF` for rollback and comparison.

## Modes

| Mode | Flag | Source of layout data | Resource location | Use Case |
|------|------|-----------------------|-------------------|----------|
| Layout Structure (default) | `USE_LAYOUT_STRUCTURE=ON` | Modular `res/layouts/*/*/layout.cpp` providers | Per layout `audio/` + `tts/` subdirs | Normal development & new languages |
| Legacy Manifest | `USE_LAYOUT_STRUCTURE=OFF` | Monolithic `src/Keys.cpp` | Flat `res/data/` + `res/data/tts/data/` | Verification / rollback |

## File Structure (Default Layout Mode)

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
scripts/migration/migrate-to-layouts.sh  # one-time migration (retained for audit)
src/layouts/LayoutRegistry.*     # runtime registration & lookup
```

## How Discovery Works (Layout Mode)
1. During CMake configure, `discover_layouts()` scans `res/layouts/*/*/<LANGUAGE>/layout.cpp`.
2. Each file is validated (layout.cpp present, audio/ & tts/ directories exist).
3. Valid sources are added to the target; each layout module registers itself in `LayoutRegistry` via a static block.
4. Runtime selection uses compile‑time `LANGUAGE` to choose the active layout set.

## Legacy Manifest Path (Optional)
If you configure with `-DUSE_LAYOUT_STRUCTURE=OFF` the previous behavior is used:
1. Parse `src/Keys.cpp` for the active language block.
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
# Default (layout structure ON)
cmake -B build .
# Legacy manifest only
cmake -B build-legacy -DUSE_LAYOUT_STRUCTURE=OFF .
```

## Validation (Layout Mode)
```bash
cmake -B build -DLANGUAGE=nl_nl .
cmake --build build
./build/Dyscover --version
```
Check compile warnings in generated layout modules (expected if some initializer fields are intentionally omitted). Ensure registration count matches expected number of layouts.

## Performance Notes
| Stage | Legacy | Layout Mode | Delta |
|-------|--------|-------------|-------|
| Configure | ~2.3s | ~2.3–2.6s | +≤0.3s |
| Build (clean) | baseline | +<10% | Within target |
| Incremental | unchanged | unchanged | — |

## Troubleshooting
| Symptom | Likely Cause | Action |
|---------|--------------|--------|
| Layout not discovered | Directory depth wrong or language code mismatch | Verify path: `res/layouts/<type>/<lang>/layout.cpp` |
| Runtime missing layout | Registration static block omitted | Add static instance + lambda registration |
| Duplicate audio copies | Shared file placed in multiple layout/audio dirs | Consider shared parent + symlink (future enhancement) |
| Rollback needed | Unexpected regression post‑switchover | Reconfigure with `-DUSE_LAYOUT_STRUCTURE=OFF` |

## Key Files
| File | Purpose |
|------|---------|
| `cmake/LayoutDiscovery.cmake` | Discovery & validation functions |
| `src/layouts/LayoutRegistry.*` | Central registry / active layout selection |
| `scripts/migration/migrate-to-layouts.sh` | One‑time migration (audit trail) |
| `res/layouts/*/*/layout.cpp` | Individual layout providers |
| `CMakeLists.txt` | Feature flag & language selection |

## Future (Phase 3 Cleanup Planned)
Remove legacy manifest path & `src/Keys.cpp` once stable monitoring (Phase 7) completes and rollback risk is minimal.

## Accessibility & Inclusive Design
Layout modules should preserve consistent naming and avoid ambiguous labels. Review added audio file names for clarity (avoid cryptic codes).  
Generated code was produced with accessibility in mind; please manually test with assistive tools.

---
Questions? See `specs/006-language-resource-optimization/spec.md` for original spec and `quickstart.md` for historical migration steps.
