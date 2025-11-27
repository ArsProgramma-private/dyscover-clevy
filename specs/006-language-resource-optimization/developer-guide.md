# Developer Guide: Language-Specific Resource Optimization

**Feature**: 006-language-resource-optimization  
**Status**: Implemented  
**Last Updated**: November 27, 2025

## Overview

This feature optimizes build packages by including only language-specific audio and TTS resources, reducing package sizes by approximately 40%.

## How It Works

### 1. Resource Extraction

When you configure CMake with a specific `LANGUAGE` (e.g., `nl` or `nl_be`), the build system:

1. Parses `src/Keys.cpp` to find all keyboard layout definitions for that language
2. Extracts audio filenames from `KeyTranslationEntry::sound` fields
3. Generates a manifest file (`resource-manifest-{LANGUAGE}.txt`)
4. Validates that all referenced files exist in `res/data/`

### 2. Build Process

- **Configuration Time**: Resource extraction and validation happen during `cmake` configuration
- **Build Time**: Only manifest files are copied to the `audio/` directory
- **Install Time**: Only manifest files are included in the package

### 3. File Structure

```
dyscover-clevy/
├── cmake/
│   └── ResourceManifest.cmake           # CMake module for manifest generation
├── scripts/
│   ├── build-tools/
│   │   ├── extract-audio-resources.cmake   # Parses Keys.cpp, extracts requirements
│   │   └── validate-resources.cmake         # Validates file existence
│   ├── build-all-languages.sh           # Builds all languages
│   └── validate-resource-optimization.sh # Tests success criteria
├── src/
│   └── Keys.cpp                         # Keyboard layouts (parsed at build time)
└── res/data/
    ├── *.wav                            # Audio files
    └── tts/data/                        # TTS files
```

## Adding a New Language

To add support for a new language (e.g., `en` for English):

### Step 1: Add Language to Keys.cpp

```cpp
// In src/Keys.cpp
#elif defined __LANGUAGE_EN__
static const std::vector<KeyTranslationEntry> g_englishDefault = {
    { Key::A, false, false, false, { { Key::A } }, "a.wav" },
    { Key::B, false, false, false, { { Key::B } }, "b.wav" },
    // ... more entries
};
```

### Step 2: Update CMakeLists.txt

```cmake
# Around line 350 in CMakeLists.txt
elseif(LANGUAGE STREQUAL "en")
  set(LANG "EN")
  set(LANG_NAME "English")
  set(LANG_ID "0409")  # 0x0409 = English (US)
  set(CHARSET "04B0")
  set(TTS_LANG "en_us")
  set(TTS_VOICE "David")
  target_compile_definitions(Dyscover PRIVATE __LANGUAGE_EN__)
```

### Step 3: Update Extraction Script

```cmake
# In scripts/build-tools/extract-audio-resources.cmake
# Add the new language to the supported list (around line 40)
if(LANGUAGE STREQUAL "nl")
  set(LANGUAGE_DEFINE "__LANGUAGE_NL__")
elseif(LANGUAGE STREQUAL "nl_be")
  set(LANGUAGE_DEFINE "__LANGUAGE_NL_BE__")
elseif(LANGUAGE STREQUAL "en")
  set(LANGUAGE_DEFINE "__LANGUAGE_EN__")
else()
  message(FATAL_ERROR "Unsupported language: ${LANGUAGE}")
endif()
```

### Step 4: Update Multi-Language Build Script

```bash
# In scripts/build-all-languages.sh (line 8)
SUPPORTED_LANGUAGES=("nl" "nl_be" "en")
```

### Step 5: Add Audio Files

Ensure all `.wav` files referenced in the English layouts exist in `res/data/`.

### Step 6: Add TTS Files

Place TTS files in `res/data/tts/data/`:
- `en_us.db`, `en_us.fsa`, `en_us.fst`
- `David.db`, `David.fon`, `David.udb`

### Step 7: Test

```bash
# Build English version
cmake -DLANGUAGE=en -B build-en .
cmake --build build-en

# Verify resource extraction
cat build-en/resource-manifest-en.txt

# Verify audio files
ls build-en/audio/
```

## Building for Multiple Languages

### Sequential Build

```bash
# Build all languages one at a time
./scripts/build-all-languages.sh --sequential
```

### Parallel Build (Faster)

```bash
# Build all languages simultaneously
./scripts/build-all-languages.sh
```

### Custom Build Options

```bash
# Build with full licensing
./scripts/build-all-languages.sh --licensing full

# Build with tests
./scripts/build-all-languages.sh --tests ON
```

## Build System Integration

### For Linux

```bash
# Configure for Dutch
cmake -DLANGUAGE=nl -B build-nl .

# Configure for Flemish
cmake -DLANGUAGE=nl_be -B build-nl_be .

# Build
cmake --build build-nl --parallel
cmake --build build-nl_be --parallel
```

### For Windows

```powershell
# Dutch build
.\build-windows.ps1 -Language nl

# Flemish build
.\build-windows.ps1 -Language nl_be
```

## Validation

### Manual Validation

```bash
# Verify builds are language-specific
find build-nl/audio -name "*.wav" | wc -l
find build-nl_be/audio -name "*.wav" | wc -l
find res/data -name "*.wav" | wc -l

# Compare lists
comm -12 <(ls build-nl/audio | sort) <(ls build-nl_be/audio | sort)
```

### Automated Validation

```bash
# Run comprehensive validation suite
./scripts/validate-resource-optimization.sh
```

This script tests:
- **SC-001**: Dutch package size reduction ≥40%
- **SC-002**: Flemish package size reduction ≥40%
- **SC-003**: Builds complete successfully
- **SC-004**: Validation completes within 10 seconds
- **SC-006**: No cross-language contamination
- **SC-007**: 100% error detection

## CI/CD Integration

GitHub Actions workflow is provided at `.github/workflows/multi-language-build.yml`:

```yaml
# Builds all languages in parallel
# Verifies package independence
# Uploads artifacts for each language
```

## Troubleshooting

### Problem: "Language section not found"

**Error**: `Language section '#if defined __LANGUAGE_XX__' not found in Keys.cpp`

**Solution**: Ensure Keys.cpp has the correct preprocessor block:
```cpp
#if defined __LANGUAGE_NL__
// or
#elif defined __LANGUAGE_NL_BE__
```

### Problem: "Missing audio files"

**Error**: `Missing audio files (N): file.wav`

**Solutions**:
1. Add the missing `.wav` file to `res/data/`
2. Or remove the reference from Keys.cpp if it's not needed
3. Check filename spelling in Keys.cpp (case-sensitive)

### Problem: "Resource validation failed"

**Solution**: Run CMake with verbose output:
```bash
cmake -DLANGUAGE=nl -B build-test . 2>&1 | grep -A 20 "Resource Validation"
```

### Problem: Build includes all files (no optimization)

**Check**:
1. Verify manifest file exists: `cat build-nl/resource-manifest-nl.txt`
2. Check manifest file count: should be ~62 for Dutch, not 68
3. Ensure you're building with the correct LANGUAGE setting

## Performance

- **Extraction Time**: <1 second (parsing Keys.cpp)
- **Validation Time**: <1 second (checking file existence)
- **Total Overhead**: <2 seconds added to CMake configuration

## Key Files Reference

| File | Purpose |
|------|---------|
| `cmake/ResourceManifest.cmake` | CMake module providing `generate_resource_manifest()` |
| `scripts/build-tools/extract-audio-resources.cmake` | Parses Keys.cpp, generates manifest |
| `scripts/build-tools/validate-resources.cmake` | Validates resources exist |
| `scripts/build-all-languages.sh` | Multi-language build automation |
| `scripts/validate-resource-optimization.sh` | Tests success criteria |
| `.github/workflows/multi-language-build.yml` | CI/CD workflow |

## Technical Details

### Manifest File Format

Simple text file, one filename per line:
```
a.wav
b.wav
c.wav
...
```

### Extraction Algorithm

1. Read Keys.cpp file content
2. Find language section start: `#if defined __LANGUAGE_XX__` or `#elif defined __LANGUAGE_XX__`
3. Find section end: next `#elif` or `#endif`
4. Extract substring for that language
5. Regex match all `"*.wav"` patterns
6. Remove duplicates and sort
7. Write to manifest file

### Validation Algorithm

1. Read manifest file (audio files)
2. Check each file exists in `res/data/`
3. Check TTS files exist in `res/data/tts/data/`
4. Accumulate all errors (don't fail-fast)
5. Report comprehensive error message with all missing files

## Future Enhancements

Potential improvements (currently out of scope):

1. **Source Restructuring**: Split Keys.cpp into per-language files
2. **Resource Organization**: Subdirectories for language-specific resources
3. **Dynamic Loading**: Runtime language switching without rebuild
4. **Audio Compression**: Use OGG instead of WAV for smaller files
5. **Automatic Detection**: Infer required files from layout definitions

---

**Questions?** See `specs/006-language-resource-optimization/spec.md` for full specification.
